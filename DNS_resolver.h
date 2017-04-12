#ifndef PROXY_SERVER_DNS_RESOLVER_H
#define PROXY_SERVER_DNS_RESOLVER_H


#include <vector>
#include <thread>
#include <string.h>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <netdb.h>

#include "event_handler.h"

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
    class id_queue {
    private:
        uint64_t counter;
        std::queue<uint64_t> queue;
        std::mutex mutex;
    public:
        id_queue();

        uint64_t get_next_id();
        void add_id(uint64_t id);
    };
    bool finished;
    event_handler *handler;
    std::vector<std::thread> resolvers;
    id_queue ids;

    std::mutex request_mutex;
    std::mutex results_mutex;
    std::condition_variable condition;
    std::queue<request*> resolve_queue;
    std::queue<response*> result_queue;

    void resolver();
public:
    DNS_resolver(event_handler *handler, size_t threads_count);
    ~DNS_resolver();

    uint64_t resolve(std::string const& host, callback_t callback);
    response get_response();
    void add_id(uint64_t id);
};



#endif //PROXY_SERVER_DNS_RESOLVER_H