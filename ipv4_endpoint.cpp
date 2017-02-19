#include "ipv4_endpoint.h"

ipv4_endpoint::ipv4_endpoint(): port_net(), addr_net() {}

ipv4_endpoint::ipv4_endpoint(uint16_t port, uint32_t addr) : port_net(htons(port)),
                                                             addr_net(addr) {}

uint16_t ipv4_endpoint::get_port() const {
    return port_net;
}

uint32_t ipv4_endpoint::get_address() const {
    return addr_net;
}

uint32_t ipv4_endpoint::any() {
    return INADDR_ANY;
}

std::ostream &operator<<(std::ostream &os, ipv4_endpoint const &endpoint) {
    in_addr tmp;
    tmp.s_addr = endpoint.addr_net;
    os << inet_ntoa(tmp) << ':' << ntohs(endpoint.get_port());
    return os;
}
