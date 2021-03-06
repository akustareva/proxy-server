#include "http_wrapper.h"

const std::string http_wrapper::BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";

int http_wrapper::get_state() {
    return state;
}

void http_wrapper::add_part(std::string string) {
    message.append(string);
    update_state();
}

void http_wrapper::update_state() {
    if (state == START && message.find("\r\n") != std::string::npos) {
        state = FIRST_LINE;
        parse_first_line();
    }
    if (state == FIRST_LINE && (body_start == 0 || body_start == std::string::npos)) {
        body_start = message.find("\r\n\r\n");
    }
    if (state == FIRST_LINE && body_start != std::string::npos && body_start != 0) {
        state = HEADERS;
        body_start += 4;
        parse_headers();
    }
    if (state >= HEADERS) {
        check_body();
    }
}

void http_wrapper::parse_headers() {
    auto headers_start = std::find_if(message.begin(), message.end(), [](char a) { return a == '\n'; })++;
    auto headers_end = headers_start + 1;
    while (headers_end != message.end() && *headers_end != '\r') {
        auto space = std::find_if(headers_end, message.end(), [](char a) { return a == ':'; });
        auto crlf = std::find_if(space + 1, message.end(), [](char a) { return a == '\r'; });
        headers.insert({{headers_end, space}, {space + 2, crlf}});
        headers_end = crlf + 2;
    };
}

void http_wrapper::check_body() {
    body = message.substr(body_start);
    if (get_header_value("Content-Length") != "") {
        if (body.size() == static_cast<size_t>(std::stoi(get_header_value("Content-Length")))) {
            state = FULL_BODY;
        } else {
            state = PARTIAL_BODY;
        }
    } else if (get_header_value("Transfer-Encoding") == "chunked") {
        if (std::string(body.end() - 7, body.end()) == "\r\n0\r\n\r\n") {
            state = FULL_BODY;
        } else {
            state = PARTIAL_BODY;
        }
    } else if (body.size() == 0) {
        state = FULL_BODY;
    } else {
        state = BAD;
    }
}

std::string http_wrapper::get_header_value(std::string name) const {
    if (headers.find(name) != headers.end()) {
        auto value = headers.at(name);
        return value;
    }
    return "";
}


http_request::http_request(std::string text) : http_wrapper(text) {
    update_state();
}

std::string http_request::get_URI() {
    if (URI.find("http://") == 0) {
        URI = URI.substr(7);
    }
    if (URI.find(host) == 0) {
        URI = URI.substr(URI.find(host) + host.size());
    }
    return URI;
}

std::string http_request::get_host() {
    if (host == "") {
        host = get_header_value("Host");
        if (host != "") {
            return host;
        }
    }
    return get_header_value("host");
}

void http_request::parse_first_line() {
    auto first_space = std::find_if(message.begin(), message.end(), [](char a) { return a == ' '; });
    auto second_space = std::find_if(first_space + 1, message.end(), [](char a) { return a == ' '; });
    auto crlf = std::find_if(second_space + 1, message.end(), [](char a) { return a == '\r'; });
    if (first_space == message.end() || second_space == message.end() || crlf == message.end()) {
        state = BAD;
        return;
    }
    method = {message.begin(), first_space};
    URI = {first_space + 1, second_space};
    http_version = {second_space + 1, crlf};
    if (method != "POST" && method != "GET" || URI == "" ||
            http_version != "HTTP/1.1" && http_version != "HTTP/1.0") {
        state = BAD;
    }
}

std::string http_request::get_request_message() {
    get_host();
    std::string first_line = method + " " + get_URI() + " " + http_version + "\r\n";
    std::string headers;
    for (auto it : this->headers) {
        if (it.first != "Proxy-Connection") {
            headers.append(it.first + ": " + it.second + "\r\n");
        }
    }
    headers += "\r\n";
    return first_line + headers + body;
}

http_response::http_response(const http_response &r) : http_wrapper(r.message) {
    code = r.code;
    http_version = r.http_version;
}

http_response::http_response(std::string text) : http_wrapper(text) {
    update_state();
}

void http_response::parse_first_line() {
    auto first_space = std::find_if(message.begin(), message.end(), [](char a) { return a == ' '; });
    auto second_space = std::find_if(first_space + 1, message.end(), [](char a) { return a == ' '; });
    auto crlf = std::find_if(second_space + 1, message.end(), [](char a) { return a == '\r'; });
    if (first_space == message.end() || second_space == message.end() || crlf == message.end()) {
        state = BAD;
        return;
    }
    http_version = {message.begin(), first_space};
    code = {first_space + 1, second_space};
    if (http_version != "HTTP/1.1" && http_version != "HTTP/1.0") {
        state = BAD;
        return;
    }
}




