#pragma once

#include "http_client.h"
#include <string>
#include <map>

// Simple HTTP client for testing (doesn't actually make HTTP requests)
class SimpleHttpClient : public HttpClient {
public:
    SimpleHttpClient();
    ~SimpleHttpClient() override;

    bool post(const std::string& url, const std::string& data, 
              const std::map<std::string, std::string>& headers, std::string& response) override;
    bool get(const std::string& url, const std::map<std::string, std::string>& headers, 
             std::string& response) override;
}; 