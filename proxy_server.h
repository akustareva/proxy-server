#ifndef PROXY_SERVER_PROXY_SERVER_H
#define PROXY_SERVER_PROXY_SERVER_H


#include <vector>
#include <memory>
#include <map>
#include <set>
#include <queue>
#include <iostream>
#include <bits/unique_ptr.h>

#include "http_server.h"
#include "http_wrapper.h"
#include "DNS_resolver.h"
#include "epoll_wrapper.h"
#include "event_handler.h"


class inbound_connection;
class outbound_connection;

class proxy_server {
public:
    proxy_server(ipv4_endpoint endpoint);
    sockets & get_server();
    epoll &get_epoll();

    void create_new_inbound_connection();
    void run();

    void create_new_outbound_connection(inbound_connection *);
private:
    epoll ep;
    http_server server;
    ipv4_endpoint endpoint;
    std::map<inbound_connection *, std::unique_ptr <inbound_connection> > inbound_connections;
    std::map<outbound_connection *, std::unique_ptr <outbound_connection> > outbound_connections;
    std::map<uint64_t, inbound_connection *> not_connected;

    event_handler handler;
    DNS_resolver resolver;
};

class inbound_connection {
    friend class outbound_connection;
    friend class proxy_server;
public:
    inbound_connection(proxy_server *proxy, std::function<void(inbound_connection *)> on_disconnect);
    ~inbound_connection();

    void read_request();
    void send_response();
private:
    proxy_server* proxy;
    sockets socket;
    data_info data;
    outbound_connection * partner;
    std::unique_ptr<http_request> request;
    std::queue<std::string> messages;
    std::function<void(inbound_connection *)> on_disconnect;

    void set_relations(outbound_connection * p);
    void send_smt_bad(std::string msg);

    std::set<outbound_connection *> connected;
};

class outbound_connection {
    friend class inbound_connection;
public:
    outbound_connection(proxy_server *proxy, inbound_connection * partner, sockaddr x, socklen_t y, std::function<void(
            outbound_connection *)> on_disconnect);
    ~outbound_connection();

    void send_request();
    void read_response();
private:
    proxy_server* proxy;
    sockets socket;
    data_info data;
    inbound_connection * partner;
    std::unique_ptr<http_response> response;
    std::function<void(outbound_connection *)> on_disconnect;

    std::unique_ptr<http_request> request;
    std::string rest;
};

#endif //PROXY_SERVER_PROXY_SERVER_H
