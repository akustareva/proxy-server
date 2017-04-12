#ifndef PROXY_ADDRESS_H
#define PROXY_ADDRESS_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdint>
#include <string>
#include <sstream>
#include <stdexcept>
#include <ostream>

struct ipv4_endpoint {
private:
    uint16_t port_net;
    uint32_t addr_net;
public:
    ipv4_endpoint();
    ipv4_endpoint(sockaddr_in saddr);
    ipv4_endpoint(uint16_t port, uint32_t addr);

    uint16_t get_port() const;
    uint32_t get_address() const;
    std::string to_string() const;
    static uint32_t any();

    friend std::ostream& operator<<(std::ostream& os, ipv4_endpoint const& endpoint);
};

std::ostream& operator<<(std::ostream& os, ipv4_endpoint const& endpoint);

#endif //PROXY_ADDRESS_H
