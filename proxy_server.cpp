#include "proxy_server.h"

proxy_server::proxy_server(epoll& ep, ipv4_endpoint const& local_endpoint):
        ep(ep),
        s_socket{local_endpoint.get_port(), local_endpoint.get_address()},
        data{ep, s_socket.get_file_descriptor(), EPOLLIN, std::bind(&proxy_server::create_new_inbound_connection, this)}
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

}

inbound_connection::inbound_connection(proxy_server* proxy, std::function<void(inbound_connection*)> on_disconnect):
        proxy(proxy),
        c_socket{proxy->get_server_socket().accept()},
        outbound(nullptr),
        on_disconnect(on_disconnect),
        data(proxy->get_epoll(), c_socket.get_file_descriptor(), EPOLLIN, [this] (uint32_t events) {
            try {
                if (events & EPOLLIN) {
                    // read request
                }
                if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    this->on_disconnect(this);
                    return;
                }
                if (events & EPOLLOUT) {
                    // write response
                }
            } catch(std::runtime_error &e) {
                this->on_disconnect(this);
            }
        })
{}
