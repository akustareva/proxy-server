#ifndef PROXY_HTTP_WRAPPER_H
#define PROXY_HTTP_WRAPPER_H


#include <unordered_map>
#include <string>
#include <sstream>
#include <regex>
#include <iostream>

enum state_t {
    BAD, START, FIRST_LINE, HEADERS, PARTIAL_BODY, FULL_BODY
};

class http_wrapper {
private:
    size_t body_start = 0;
protected:
    std::string message;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    state_t state = START;

    void update_state();
    void check_body();
    void parse_headers();
    std::string get_header_value(std::string) const;
    virtual void parse_first_line() = 0;
public:
    static const std::string BAD_REQUEST;

    http_wrapper(std::string input) : message(input) {};
    virtual ~http_wrapper() {};

    int get_state();
    void add_part(std::string);
};

class http_request: public http_wrapper {
private:
    std::string method;
    std::string URI;
    std::string http_version;
    std::string host = "";

    void parse_first_line() override;
public:
    http_request(std::string text);

    std::string get_URI();
    std::string get_host();
    std::string get_request_message();
};

class http_response: public http_wrapper {
private:
    std::string code;
    std::string http_version;

    void parse_first_line() override;
public:
    http_response(std::string text);
    http_response(const http_response&);
};


#endif //PROXY_HTTP_WRAPPER_H
