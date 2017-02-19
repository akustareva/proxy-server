#include "proxy_server.h"

proxy_server::proxy_server(epoll& ep, ipv4_endpoint const& local_endpoint): ep(ep),
                                                                            s_socket{local_endpoint.get_port(), local_endpoint.get_address()}
{
    s_socket.bind_and_listen();
}

server_socket proxy_server::get_server_socket() {
    return s_socket;
}

epoll proxy_server::get_epoll() {
    return ep;
}

void proxy_server::run() {
    ep.run();
}
