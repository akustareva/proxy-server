#include "DNS_resolver.h"

void DNS_resolver::resolver() {

}

DNS_resolver::DNS_resolver(event_handler* handler, size_t thread_count): finished(false),
                                                                         handler(handler)
{
    for (size_t i = 0; i < thread_count; i++) {
        resolvers.push_back(std::thread(&DNS_resolver::resolver, this));
    }
}

DNS_resolver::~DNS_resolver() {
    finished = true;
    for (auto& thread : resolvers) {
        thread.join();
    }
}
