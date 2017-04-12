#include "proxy_server.h"

proxy_server::proxy_server(ipv4_endpoint endpoint)
        : endpoint(endpoint),
          server(ep, endpoint, std::bind(&proxy_server::create_new_inbound_connection, this)),
          resolver(&handler, 10),
          handler(ep, [this] () throw(std::runtime_error) {
              auto res = resolver.get_response();
              auto p = not_connected.find(res.id);
              if (p != not_connected.end()) {
                  if (res.failed) {
                      if (inbound_connections.find(p->second) != inbound_connections.end()) {
                          p->second->send_smt_bad(http_wrapper::BAD_REQUEST);
                      }
                  } else {
                      if (inbound_connections.find(p->second) != inbound_connections.end()) {
                          res.callback(res.resolved, res.resolved_len);
                      }
                  }
                  not_connected.erase(res.id);
                  resolver.add_id(res.id);
              }
          })
{
    std::cerr << "Server run on " << endpoint.to_string() << "\n";
}

sockets &proxy_server::get_server() {
    return server.get_socket();
}

epoll &proxy_server::get_epoll() {
    return ep;
}

void proxy_server::create_new_inbound_connection() {
    std::unique_ptr<inbound_connection> u_ptr(new inbound_connection(this, [this](
            inbound_connection * item) throw(std::runtime_error) {inbound_connections.erase(item);}));
    inbound_connection *ptr = u_ptr.get();
    inbound_connections.emplace(ptr, std::move(u_ptr));
}

void proxy_server::create_new_outbound_connection(inbound_connection *caller) {
    not_connected.emplace(resolver.resolve(caller->request->get_host(), [this, caller] (sockaddr x, socklen_t y) throw(std::runtime_error) {
        std::unique_ptr<outbound_connection> u_ptr(new outbound_connection(this, caller, x, y, [this](
                outbound_connection * item) {outbound_connections.erase(item);}));
        outbound_connection *ptr = u_ptr.get();
        this->outbound_connections.emplace(ptr, std::move(u_ptr));
    }), caller);
}

void proxy_server::run() {
    ep.run();
}

inbound_connection::inbound_connection(proxy_server *proxy, std::function<void(inbound_connection *)> on_disconnect)
        : proxy(proxy),
          socket(proxy->get_server().accept()),
          partner(nullptr),
          data(proxy->get_epoll(), socket.get_file_descriptor(), EPOLLIN, [this] (uint32_t events) mutable throw(std::runtime_error)
          {
              try {
                  if (events & EPOLLIN) {
                      read_request();
                  }
                  if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                      this->on_disconnect(this);
                      return;
                  }
                  if (events & EPOLLOUT) {
                      send_response();
                  }
              } catch(std::runtime_error &e) {
                  this->on_disconnect(this);
              }
          }),
          on_disconnect(on_disconnect)
{}

inbound_connection::~inbound_connection() {
    while (connected.size()) {
        (*connected.begin())->on_disconnect(*connected.begin());
    }
    connected.clear();
}

void inbound_connection::read_request() {
    std::string buffer;

    socket.read_into_buffer(buffer);

    if (request.get() == nullptr) {
        request.reset(new http_request(buffer));
    } else {
        request->add_part(buffer);
    }

    if (request->get_state() == BAD) {
        messages.push(http_wrapper::BAD_REQUEST);
        data.add_flag(EPOLLOUT);
    } else if (request->get_state() == FULL_BODY) {
        proxy->create_new_outbound_connection(this);
    }
}

void inbound_connection::send_response() {
    while (!messages.empty()) {
        ssize_t ind = socket.write(messages.front().c_str(), messages.front().size());
        if (ind != messages.front().size()) {
            messages.front() = messages.front().substr(ind);
            break;
        }
        messages.pop();
    }

    if (messages.empty()) {
        if (partner) {
            partner->data.add_flag(EPOLLIN);
        }
        data.remove_flag(EPOLLOUT);
    }
}

void inbound_connection::set_relations(outbound_connection *p) {
    partner = p;
    connected.insert(p);
}

void inbound_connection::send_smt_bad(std::string msg) {
    messages.push(msg);
    data.add_flag(EPOLLOUT);
}


outbound_connection::outbound_connection(proxy_server *proxy, inbound_connection *partner, sockaddr x, socklen_t y, std::function<void(
        outbound_connection *)> on_disconnect)
        : socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK),
          partner(partner),
          data(proxy->get_epoll(), socket.get_file_descriptor(), EPOLLOUT, [this] (uint32_t events) mutable throw(std::runtime_error)
          {
              try {
                  if (events & EPOLLIN) {
                      read_response();
                  }
                  if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                      this->on_disconnect(this);
                      return;
                  }
                  if (events & EPOLLOUT) {
                      send_request();
                  }
              } catch(std::runtime_error &e) {
                  this->on_disconnect(this);
              }
          }),
          on_disconnect(on_disconnect),
          proxy(proxy),
          request(std::move(partner->request))
{
    this->partner->set_relations(this);
    socket.connect(&x, y);
}

outbound_connection::~outbound_connection() {
    if (partner){
        partner->connected.erase(this);
        if (partner->partner == this) {
            partner->partner = nullptr;
        }
    }
}

void outbound_connection::send_request() {
    if (partner) {
        std::string buf;
        if (rest.empty()) {
            buf = request->get_request_message();
        } else {
            buf = rest;
        }

        ssize_t ind = socket.write(buf.c_str(), buf.size());
        if (ind != buf.size()) {
            rest = buf.substr(ind);
        } else {
            data.add_flag(EPOLLIN);
            data.remove_flag(EPOLLOUT);
        }
    } else {
        throw_error("No partner");
    }
}

void outbound_connection::read_response() {
    if (partner) {
        std::string buffer;

        socket.read_into_buffer(buffer);

        std::string sub(buffer);
        if (response.get() == nullptr) {
            response.reset(new http_response(sub));
        } else {
            response->add_part(sub);
        }

        if (response->get_state() >= FIRST_LINE) {
            partner->messages.push(sub);
            partner->data.add_flag(EPOLLOUT);
            data.remove_flag(EPOLLIN);
        } else if (response->get_state() == BAD) {
            partner->messages.push(sub);
            partner->data.add_flag(EPOLLOUT);
            data.remove_flag(EPOLLIN);
        }
    } else {
        throw_error("No partner");
    }
}
