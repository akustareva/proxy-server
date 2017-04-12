#ifndef PROXY_HTTP_SERVER_H
#define PROXY_HTTP_SERVER_H


#include "epoll_wrapper.h"
#include "ipv4_endpoint.h"

class http_server {
public:
    http_server(epoll &ep, ipv4_endpoint &endpoint, std::function<void()> on_accept);
    sockets & get_socket();
private:
    epoll &ep;
    sockets socket;
    data_info data;
    std::function<void()> on_accept;
};


#endif //PROXY_HTTP_SERVER_H
