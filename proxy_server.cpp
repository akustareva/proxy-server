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
                    // TODO: read request
                }
                if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    this->on_disconnect(this);
                    return;
                }
                if (events & EPOLLOUT) {
                    // TODO: write response
                }
            } catch(std::runtime_error &e) {
                this->on_disconnect(this);
            }
        })
{}

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
                    // TODO: read response
                }
                if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    this->on_disconnect(this);
                    return;
                }
                if (events & EPOLLOUT) {
                    // TODO: write request
                }
            } catch(std::runtime_error &e) {
                this->on_disconnect(this);
            }
        })
{
    s_socket.connect(&addr, slen);
    inbound->outbound = this;
}
