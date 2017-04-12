#include "DNS_resolver.h"

DNS_resolver::DNS_resolver(event_handler *handler, size_t threads_count)
        : finished(false),
          handler(handler)
{
    for (int i = 0; i < threads_count; i++) {
        resolvers.push_back(std::thread(&DNS_resolver::resolver, this));
    }
}

void DNS_resolver::resolver() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    while (!finished) {
        std::unique_lock<std::mutex> request_locker(request_mutex);
        condition.wait(request_locker, [&]{return (!resolve_queue.empty() || finished);});
        if (finished) {
            break;
        }
        auto request = std::move(resolve_queue.front());
        resolve_queue.pop();
        request_locker.unlock();

        std::string host = request->hostname;
        std::string port = "80";
        auto pos = host.find(":");
        if (pos != host.npos) {
            host = host.substr(0, pos);
            port = host.substr(pos + 1);
        }

        sockaddr resolved;
        socklen_t resolved_len;

        struct addrinfo hints, *res;
        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        int error = getaddrinfo(host.data(), port.data(), &hints, &res);
        if (error == 0) {
            resolved = *(res->ai_addr);
            resolved_len = res->ai_addrlen;
            freeaddrinfo(res);
        }
        std::unique_lock<std::mutex> results_locker(results_mutex);
        result_queue.push(new response(request->id, resolved, resolved_len, error != 0, request->callback));
        uint64_t i = 1;
        handler->get_file_descriptor().write(&i, sizeof(i));
        results_locker.unlock();
    }
}

DNS_resolver::~DNS_resolver() {
    std::unique_lock<std::mutex> locker_t(request_mutex);
    finished = true;
    locker_t.unlock();
    condition.notify_all();
    for (auto& thread : resolvers) {
        thread.join();
    }
}

DNS_resolver::request::request(uint64_t id, std::string const &hostname, callback_t callback): id(id),
                                                                                               hostname(hostname),
                                                                                               callback(callback)
{}

DNS_resolver::response::response(uint64_t id, sockaddr resolved, socklen_t resolved_len, bool failed,
                                 callback_t callback): id(id),
                                                       resolved(resolved),
                                                       resolved_len(resolved_len),
                                                       failed(failed),
                                                       callback(std::move(callback))
{}

uint64_t DNS_resolver::resolve(std::string const &host, callback_t callback) {
    uint64_t id = ids.get_next_id();
    std::unique_lock<std::mutex> locker(request_mutex);
    resolve_queue.push(new request(id, host, std::move(callback)));
    condition.notify_one();
    locker.unlock();
    return id;
}

DNS_resolver::response DNS_resolver::get_response() {
    std::unique_lock<std::mutex> results_locker(results_mutex);
    auto response = result_queue.front();
    result_queue.pop();
    results_locker.unlock();

    return *response;
}

void DNS_resolver::add_id(uint64_t id) {
    ids.add_id(id);
}

DNS_resolver::id_queue::id_queue(): counter(0)
{
    queue.push(counter);
}

uint64_t DNS_resolver::id_queue::get_next_id() {
    uint64_t id;
    std::unique_lock<std::mutex> locker(mutex);
    if (queue.empty()) {
        queue.push(++counter);
    }
    id = queue.front();
    queue.pop();
    locker.unlock();
    return id;
}

void DNS_resolver::id_queue::add_id(uint64_t id) {
    std::unique_lock<std::mutex> locker(mutex);
    queue.push(id);
    locker.unlock();
}
