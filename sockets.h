#ifndef PROXY_POSIX_SOCKET_H
#define PROXY_POSIX_SOCKET_H

#include <sys/types.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <netdb.h>

#include "throw_error.h"
#include "raii_file_descriptor.h"

class sockets {
private:
    raii_file_descriptor fd;
    uint16_t port;
    uint32_t address;

    int create_socket_fd();
public:
    sockets();
    sockets(int fd);
    sockets(uint16_t port, in_addr_t s_addr);
    ~sockets();

    void bind_and_listen();
    void connect(sockaddr *addr, socklen_t slen);
    int accept();

    int get_flags();
    void set_flags(uint32_t nex_flags);
    int get_available();
    ssize_t read(void *buffer, size_t size);
    ssize_t write(void const *buffer, size_t _size);
    void read_into_buffer(std::string &buffer);

    raii_file_descriptor &get_file_descriptor();
};


#endif //PROXY_POSIX_SOCKET_H
