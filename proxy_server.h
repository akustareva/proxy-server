#ifndef PROXY_SERVER_PROXY_SERVER_H
#define PROXY_SERVER_PROXY_SERVER_H


#include "epoll_wrapper.h"
#include "sockets.h"

#include <map>
#include <bits/unique_ptr.h>

class inbound_connection;
class outbound_connection;

class proxy_server {
private:
    epoll& ep;
    server_socket s_socket;
    data_info data;
    std::map<inbound_connection*, std::unique_ptr <inbound_connection> > inbound_connections;
    std::map<outbound_connection*, std::unique_ptr <outbound_connection> > outbound_connections;
public:
    proxy_server(epoll& ep, ipv4_endpoint const& local_endpoint);

    server_socket& get_server_socket();
    epoll& get_epoll();
    void run();
    void create_new_inbound_connection();
    void create_new_outbound_connection(inbound_connection* inbound);
};

class inbound_connection {
    friend class proxy_server;
    friend class outbound_connection;
private:
    proxy_server* proxy;
    client_socket c_socket;
    outbound_connection* outbound;
    data_info data;
public:
    inbound_connection(proxy_server *proxy, std::function<void(inbound_connection*)> on_disconnect);
};

class outbound_connection {
    friend class proxy_server;
    friend class inbound_connection;
};


#endif //PROXY_SERVER_PROXY_SERVER_H
