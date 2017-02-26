#ifndef PROXY_SERVER_DNS_RESOLVER_H
#define PROXY_SERVER_DNS_RESOLVER_H

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "event_handler.h"

class DNS_resolver {
private:
    bool finished;
    event_handler* handler;
    std::vector<std::thread> resolvers;

    void resolver();
public:
    DNS_resolver(event_handler* handler, size_t thread_count);
    ~DNS_resolver();
};

#endif //PROXY_SERVER_DNS_RESOLVER_H
