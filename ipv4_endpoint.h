#ifndef PROXY_IPV4_ENDPOINT_H
#define PROXY_IPV4_ENDPOINT_H

#include <stdint.h>
#include <netinet/in.h>
#include <string>
#include <netdb.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <ostream>

struct ipv4_endpoint {
private:
    uint32_t addr_net;
    uint16_t port_net;
public:
    ipv4_endpoint();
    ipv4_endpoint(uint16_t port, uint32_t addr);
    uint16_t get_port() const;
    uint32_t get_address() const;
    static uint32_t any();

    friend std::ostream& operator<<(std::ostream& os, ipv4_endpoint const& endpoint);
};

#endif //PROXY_IPV4_ENDPOINT_H
