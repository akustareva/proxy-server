#ifndef PROXY_SERVER_SOCKETS_H
#define PROXY_SERVER_SOCKETS_H

#include "raii_file_descriptor.h"
#include "throw_error.h"
#include "ipv4_endpoint.h"

#include <sys/socket.h>
#include <netinet/in.h>

class socket_t {
protected:
    raii_file_descriptor fd;
public:
    socket_t();

    int get_available();
    ssize_t read(void *buffer, size_t size);
    ssize_t write(void const *buffer, size_t size);
    void read_into_buffer(std::string& buffer);
    raii_file_descriptor& get_file_descriptor();
};

class client_socket : public socket_t {
public:
    client_socket();
    client_socket(int fd);

    void connect(ipv4_endpoint &remote);
};

class server_socket : public socket_t {
private:
    uint16_t port;
    uint32_t address;
public:
    server_socket();
    server_socket(uint16_t port, uint32_t addr);

    void bind_and_listen();
    client_socket accept();
    void connect(sockaddr* addr, socklen_t slen);
};

#endif //PROXY_SERVER_SOCKETS_H
