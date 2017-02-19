#ifndef PROXY_SERVER_SOCKETS_H
#define PROXY_SERVER_SOCKETS_H

#include "raii_file_descriptor.h"
#include "throw_error.h"
#include "ipv4_endpoint.h"

#include <sys/socket.h>
#include <netinet/in.h>

class client_socket;

class server_socket {
private:
    raii_file_descriptor fd;
    uint16_t port;
    uint32_t address;
public:
    server_socket();
    server_socket(uint16_t port, uint32_t addr);

    void bind_and_listen();
    client_socket accept();

    raii_file_descriptor& get_file_descriptor();
};

class client_socket {
private:
    raii_file_descriptor fd;
public:
    client_socket();
    client_socket(int fd);

    void connect(ipv4_endpoint &remote);
};

#endif //PROXY_SERVER_SOCKETS_H
