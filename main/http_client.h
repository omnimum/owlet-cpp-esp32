#pragma once

#include <string>
#include <map>

// HTTP client interface (to be implemented with ESP-IDF HTTP client)
class HttpClient {
public:
    virtual ~HttpClient() = default;
    virtual bool post(const std::string& url, const std::string& data, const std::map<std::string, std::string>& headers, std::string& response) = 0;
    virtual bool get(const std::string& url, const std::map<std::string, std::string>& headers, std::string& response) = 0;
}; 