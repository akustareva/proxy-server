#include <iostream>
#include "proxy_server.h"

using namespace std;

int main() {
    epoll ep;
    proxy_server proxy(ep, ipv4_endpoint(8080, ipv4_endpoint::any()));
    proxy.run();
    return 0;
}