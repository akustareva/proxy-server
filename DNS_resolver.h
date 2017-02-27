#ifndef PROXY_SERVER_DNS_RESOLVER_H
#define PROXY_SERVER_DNS_RESOLVER_H

#include "event_handler.h"
#include "sockets.h"

#include <vector>
#include <thread>
#include <string.h>
#include <condition_variable>
#include <mutex>
#include <queue>

typedef typename std::function<void(sockaddr, socklen_t)> callback_t;

class DNS_resolver {
private:
    class request {
    public:
        request(uint64_t id, std::string const& hostname, callback_t callback);
        uint64_t id;
        std::string hostname;
        callback_t callback;
    };
    class response {
    public:
        response(uint64_t id, sockaddr resolved, socklen_t resolved_len, bool failed, callback_t callback);
        uint64_t id;
        sockaddr resolved;
        socklen_t resolved_len;
        bool failed;
        callback_t callback;
    };
    bool finished;
    event_handler* handler;
    std::vector<std::thread> resolvers;

    std::mutex request_mutex;
    std::mutex response_mutex;
    std::condition_variable condition;
    std::queue<request*> resolve_queue;
    std::queue<response*> result_queue;

    void resolver();
public:
    DNS_resolver(event_handler* handler, size_t thread_count);
    ~DNS_resolver();

    response get_response();
};

#endif //PROXY_SERVER_DNS_RESOLVER_H
