#include "sockets.h"

sockets::sockets() : fd(-1) {}

sockets::sockets(uint16_t port, in_addr_t s_addr) : fd(create_socket_fd()),
                                                    port(port),
                                                    address(s_addr) {}

int sockets::create_socket_fd() {
    int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socket_fd == -1) {
        throw_error("Can't create server socket.");
    }
    return socket_fd;
}

sockets::sockets(int fd) : fd(fd) {
    set_flags(get_flags() | SOCK_STREAM | SOCK_NONBLOCK);
}

sockets::~sockets() {}

void sockets::bind_and_listen() {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = address;

    int bnd = ::bind(fd.get_file_descriptor(), reinterpret_cast<sockaddr*>(&server), sizeof(server));
    if (bnd == -1) {
        throw_error("Error during bind() server socket");
    }

    if (::listen(fd.get_file_descriptor(), SOMAXCONN) != 0) {
        throw_error("Error during listen() server socket");
    }
}

void sockets::connect(sockaddr *addr, socklen_t slen) {
    if ((::connect(fd.get_file_descriptor(), addr, slen)) == -1) {
        if (errno != EINPROGRESS) {
            throw_error("Error during connect() connect server socket");
        }
    }
}

int sockets::accept() {
    int new_fd = ::accept(fd.get_file_descriptor(), NULL, NULL);
    if (new_fd == -1) {
        throw_error("Error during accept() server socket");
    }

    return new_fd;
}

int sockets::get_flags() {
    return fd.get_flags();
}

void sockets::set_flags(uint32_t nex_flags) {
    fd.set_flags(nex_flags);
}

int sockets::get_available() {
    return fd.get_available();
}

ssize_t sockets::read(void *buffer, size_t size) {
    return fd.read(buffer, size);
}

ssize_t sockets::write(void const *buffer, size_t size) {
    return fd.write(buffer, size);
}

void sockets::read_into_buffer(std::string& buffer) {
    int n = get_available();
    if (n == 0) {
        return;
    }
    char char_buffer[n + 1];
    if (read(char_buffer, n) == 0) {
        return;
    }
    char_buffer[n] = '\0';
    buffer = std::string(char_buffer, n);
}

raii_file_descriptor &sockets::get_file_descriptor() {
    return fd;
}