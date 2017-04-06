#include <iostream>
#include "proxy_server.h"

using namespace std;

int main() {
    epoll ep;
    proxy_server proxy(ep, ipv4_endpoint(2539, ipv4_endpoint::any()));
    proxy.run();
    return 0;
}