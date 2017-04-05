#include "proxy_server.h"

proxy_server::proxy_server(epoll& ep, ipv4_endpoint const& local_endpoint):
        ep(ep),
        s_socket{local_endpoint.get_port(), local_endpoint.get_address()},
        data{ep, s_socket.get_file_descriptor(), EPOLLIN, std::bind(&proxy_server::create_new_inbound_connection, this)},
        resolver(&handler, 10),
        handler(ep, [this] () {
            auto response = resolver.get_response();
            auto found_connection = waiting_for_connection.find(response.id);
            if (found_connection != waiting_for_connection.end()) {
                if (inbound_connections.find(found_connection->second) != inbound_connections.end()) {
                    if (response.failed) {
                        // TODO: bad request
                    } else {
                        response.callback(response.resolved, response.resolved_len);
                    }
                }
                waiting_for_connection.erase(response.id);
                resolver.add_id(response.id);
            }
        })
{
    s_socket.bind_and_listen();
}

server_socket& proxy_server::get_server_socket() {
    return s_socket;
}

epoll& proxy_server::get_epoll() {
    return ep;
}

void proxy_server::run() {
    ep.run();
}

void proxy_server::create_new_inbound_connection() {
    std::unique_ptr<inbound_connection> u_ptr(new inbound_connection(this,
                        [this](inbound_connection* connection) {inbound_connections.erase(connection);}));
    inbound_connection* ptr = u_ptr.get();
    inbound_connections.emplace(ptr, std::move(u_ptr));
}

void proxy_server::create_new_outbound_connection(inbound_connection *inbound) {
    waiting_for_connection.emplace(resolver.resolve(inbound->request->get_host(), [this, inbound] (sockaddr addr, socklen_t slen) {
        std::unique_ptr<outbound_connection> out_uptr(new outbound_connection(this, inbound, addr, slen, [this] (outbound_connection* outbound) {
            outbound_connections.erase(outbound);
        }));
        outbound_connection* out_ptr = out_uptr.get();
        this->outbound_connections.emplace(out_ptr, std::move(out_uptr));
    }), inbound);
}

inbound_connection::inbound_connection(proxy_server* proxy, std::function<void(inbound_connection*)> on_disconnect):
        proxy(proxy),
        c_socket{proxy->get_server_socket().accept()},
        outbound(nullptr),
        on_disconnect(on_disconnect),
        data(proxy->get_epoll(), c_socket.get_file_descriptor(), EPOLLIN, [this] (uint32_t events) {
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
        })
{}

inbound_connection::~inbound_connection() {}

void inbound_connection::read_request() {
    std::string buffer = "";
    c_socket.read_into_buffer(buffer);
    if (request.get() == nullptr) {
        request.reset(new http_request(buffer));
    } else {
        request->add_part(buffer);
    }
    if (request->get_state() == BAD) {
        response_messages.push("HTTP/1.1 400 Bad Request\r\n\r\n");
        data.add_flag(EPOLLOUT);
    } else if (request->get_state() == FULL_BODY) {
        proxy->create_new_outbound_connection(this);
    }
}

void inbound_connection::send_response() {
    while (!response_messages.empty()) {
        std::string message = response_messages.front();
        ssize_t sent = c_socket.write(message.c_str(), message.size());
        if (sent != message.size()) {
            response_messages.front() = message.substr(sent);
            break;
        }
        response_messages.pop();
    }
    if (response_messages.empty()) {
        if (outbound != nullptr) {
            outbound->data.add_flag(EPOLLIN);
        }
        data.remove_flag(EPOLLOUT);
    }
}

outbound_connection::outbound_connection(proxy_server *proxy, inbound_connection *inbound, sockaddr addr, socklen_t slen,
                                         std::function<void(outbound_connection *)> on_disconnect) :
        s_socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK),
        proxy(proxy),
        on_disconnect(on_disconnect),
        inbound(inbound),
        request(std::move(inbound->request)),
        data(proxy->get_epoll(), s_socket.get_file_descriptor(), EPOLLOUT, [this] (uint32_t events) {
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
        })
{
    s_socket.connect(&addr, slen);
    inbound->outbound = this;
}

outbound_connection::~outbound_connection() {
    if (inbound != nullptr && inbound->outbound == this) {
        inbound->outbound = nullptr;
    }
}

void outbound_connection::read_response() {
    if (inbound != nullptr) {
        std::string buffer;
        s_socket.read_into_buffer(buffer);
        if (response.get() == nullptr) {
            response.reset(new http_response(buffer));
        } else {
            response->add_part(buffer);
        }
        inbound->response_messages.push(buffer);
        inbound->data.add_flag(EPOLLOUT);
        data.remove_flag(EPOLLIN);
    } else {
        throw_error("Error in read_response(): no inbound connection associated with it.");
    }
}

void outbound_connection::send_request() {
    if (inbound != nullptr) {
        std::string buffer;
        // TODO: send request
    } else {
        throw_error("Error in send_request(): no inbound connection associated with it.");
    }
}
