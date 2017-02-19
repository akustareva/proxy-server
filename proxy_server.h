#ifndef PROXY_SERVER_PROXY_SERVER_H
#define PROXY_SERVER_PROXY_SERVER_H


#include "epoll_wrapper.h"
#include "sockets.h"

class proxy_server {
private:
    epoll& ep;
    server_socket s_socket;
public:
    proxy_server(epoll& ep, ipv4_endpoint const& local_endpoint);

    server_socket get_server_socket();
    epoll get_epoll();
    void run();
};


#endif //PROXY_SERVER_PROXY_SERVER_H
