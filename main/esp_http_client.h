#pragma once

#include "http_client.h"
#include <string>
#include <map>

// Forward declarations for ESP-IDF types
struct esp_http_client_event_t;
typedef int esp_err_t;

class EspHttpClient : public HttpClient {
public:
    EspHttpClient();
    ~EspHttpClient() override;

    bool post(const std::string& url, const std::string& data, 
              const std::map<std::string, std::string>& headers, std::string& response) override;
    bool get(const std::string& url, const std::map<std::string, std::string>& headers, 
             std::string& response) override;

private:
    static esp_err_t http_event_handler(esp_http_client_event_t *evt);
    static std::string response_data_;
    static bool request_completed_;
    static int status_code_;
}; 