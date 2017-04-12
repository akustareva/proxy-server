#include <iostream>

#include "proxy_server.h"

int main() {
    proxy_server proxy(ipv4_endpoint(3339, ipv4_endpoint::any()));
    proxy.run();
    return 0;
}