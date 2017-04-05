#include "sockets.h"

socket_t::socket_t() : fd(-1) {}

int socket_t::get_available() {
    return fd.get_available();
}

ssize_t socket_t::read(void *buffer, size_t size) {
    return fd.read(buffer, size);
}

ssize_t socket_t::write(void const *buffer, size_t size) {
    return fd.write(buffer, size);
}

void socket_t::read_into_buffer(std::string& buffer) {
    int n = get_available();
    if (n == 0) {
        std:: cerr << "There are no available bytes.\n";
        return;
    }
    char char_buffer[n + 1];
    if (read(char_buffer, n) == 0) {
        std:: cerr << "There are no bytes were read: EOF.\n";
        return;
    }
    char_buffer[n] = '\0';
    buffer = std::string(char_buffer, n);
}

raii_file_descriptor &socket_t::get_file_descriptor() {
    return fd;
}

server_socket::server_socket() {}

server_socket::server_socket(uint16_t port, uint32_t addr): port(port),
                                                            address(addr)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socket_fd == -1) {
        throw_error("Can't create setver socket");
    }
    fd = socket_fd;
}

void server_socket::bind_and_listen() {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = address;

    int bnd = bind(fd.get_file_descriptor(), reinterpret_cast<sockaddr*>(&server), sizeof(server));
    if (bnd == -1) {
        throw_error("Error during bind() server socket");
    }

    if (listen(fd.get_file_descriptor(), SOMAXCONN) != 0) {
        throw_error("Error during listen() server socket");
    }
}

void server_socket::connect(sockaddr* addr, socklen_t slen) {
    if ((::connect(fd.get_file_descriptor(), addr, slen)) == -1) {
        if (errno != EINPROGRESS) {
            throw_error("Error during connect() connect server socket");
        }
    }
}

client_socket server_socket::accept() {
    int client_socket_fd = ::accept(fd.get_file_descriptor(), NULL, NULL);
    if (client_socket_fd == -1) {
        throw_error("Error during accept() server socket");
    }
    return client_socket(client_socket_fd);
}

client_socket::client_socket() {}

client_socket::client_socket(int fd): fd(fd) {}

void client_socket::connect(ipv4_endpoint &remote) {
    sockaddr_in saddr{};
    saddr.sin_family = AF_INET;
    saddr.sin_port = remote.get_port();
    saddr.sin_addr.s_addr = remote.get_address();

    int res = ::connect(fd.get_file_descriptor(), reinterpret_cast<sockaddr const*>(&saddr), sizeof(saddr));
    if (res == -1 && errno != EINPROGRESS) {
        throw_error("Error during connect() client socket " + fd.get_file_descriptor());
    }
}
