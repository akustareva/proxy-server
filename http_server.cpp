#include "http_server.h"

http_server::http_server(epoll &ep, ipv4_endpoint &endpoint, std::function<void()> on_accept)
        : ep(ep),
          socket(endpoint.get_port(), endpoint.get_address()),
          data(ep, socket.get_file_descriptor(), EPOLLIN, [this](uint32_t event) {
              if (event == EPOLLIN) {
                this->on_accept();
              }
          }),
          on_accept(on_accept)
{
    socket.bind_and_listen();
}

sockets &http_server::get_socket() {
    return socket;
}





