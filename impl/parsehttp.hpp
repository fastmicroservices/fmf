#pragma once
#include <string>
#include <strstream>
#include <cstdlib>
#include <utility>
#include <list>
#include <algorithm>
#include <cctype>
#include <zstr.hpp>

#include <iostream>

struct HttpResult {
    std::string _protocol, _response_description, _content_encoding;
    int _response_code;
    std::list<std::pair<std::string, std::string>> _headers;
    bool _valid;
    long _content_length;
    std::string _contents;

    HttpResult(std::string const &src) {
        std::istrstream str(src.c_str(), src.size());
        // the first line contains the protocol signature, version, response code, response description
        std::getline(str, _protocol, ' ');
        std::string tmp;
        std::getline(str, tmp, ' ');
        _response_code = std::atol(tmp.c_str());
        _valid = _response_code > 99 && _response_code < 600;
        if (_valid) {
            std::getline(str, _response_description);

            for (std::getline(str, tmp); _valid && tmp.size() > 1; std::getline(str, tmp)) {
                // must be a header
                std::pair<std::string,std::string> keyval;
                auto content_started = false;
                for (char const *pc = tmp.c_str(); *pc != '\r' && *pc; pc++) {
                    if (content_started) {
                        if (!keyval.second.empty() || !std::isspace(*pc)) {
                            keyval.second += *pc;
                        }
                    }
                    else if (*pc == ':') {
                        content_started = true;
                    }
                    else {
                        keyval.first += *pc;
                    }
                }
                _valid = !keyval.first.empty() && !keyval.second.empty();
                _headers.push_back(keyval);
                if (keyval.first == "Content-Length") {
                    _content_length = std::atol(keyval.second.c_str());
                }
                else if (keyval.first == "Content-Encoding") {
                    _content_encoding = keyval.second;
                }
            }
            if (_valid) {
                if (_content_encoding == "gzip") {
                    zstr::istream gzif(str);
                    _contents = std::string(std::istreambuf_iterator<char>(gzif), {});
                }
            }
        }
    }
};
