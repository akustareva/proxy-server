#ifndef PROXY_SERVER_PROXY_SERVER_H
#define PROXY_SERVER_PROXY_SERVER_H

#include "epoll_wrapper.h"
#include "sockets.h"
#include "event_handler.h"
#include "DNS_resolver.h"
#include "http_wrapper.h"

#include <map>
#include <bits/unique_ptr.h>

class inbound_connection;
class outbound_connection;

class proxy_server {
private:
    epoll& ep;
    server_socket s_socket;
    data_info data;
    event_handler handler;
    DNS_resolver resolver;
    std::map<inbound_connection*, std::unique_ptr <inbound_connection> > inbound_connections;
    std::map<outbound_connection*, std::unique_ptr <outbound_connection> > outbound_connections;
    std::map<uint64_t, inbound_connection*> waiting_for_connection;
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
    std::unique_ptr<http_request> request;
    std::function<void(inbound_connection*)> on_disconnect;
    data_info data;
    std::queue<std::string> response_messages;
public:
    inbound_connection(proxy_server *proxy, std::function<void(inbound_connection*)> on_disconnect);
    ~inbound_connection();

    void read_request();
    void send_response();
};

class outbound_connection {
    friend class proxy_server;
    friend class inbound_connection;
private:
    proxy_server* proxy;
    server_socket s_socket;
    data_info data;
    inbound_connection* inbound;
    std::unique_ptr<http_request> request;
    std::unique_ptr<http_response> response;
    std::function<void(outbound_connection*)> on_disconnect;
    std::string request_message_rest;
public:
    outbound_connection(proxy_server *proxy, inbound_connection* inbound, sockaddr addr, socklen_t slen, std::function<void(outbound_connection*)> on_disconnect);
    ~outbound_connection();

    void read_response();
    void send_request();
};

#endif //PROXY_SERVER_PROXY_SERVER_H
