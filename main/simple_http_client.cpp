#include "simple_http_client.h"
#include "esp_log.h"
#include <string>
#include <map>

static const char* TAG = "SIMPLE_HTTP_CLIENT";

SimpleHttpClient::SimpleHttpClient() {
    ESP_LOGI(TAG, "Simple HTTP Client initialized");
}

SimpleHttpClient::~SimpleHttpClient() {
    ESP_LOGI(TAG, "Simple HTTP Client destroyed");
}

bool SimpleHttpClient::post(const std::string& url, const std::string& data, 
                           const std::map<std::string, std::string>& headers, std::string& response) {
    ESP_LOGI(TAG, "Simple HTTP POST to: %s", url.c_str());
    ESP_LOGI(TAG, "Data: %s", data.c_str());
    
    // Log headers
    for (const auto& header : headers) {
        ESP_LOGI(TAG, "Header: %s = %s", header.first.c_str(), header.second.c_str());
    }
    
    // Return a mock response
    response = "{\"mock\": \"response\", \"status\": \"success\"}";
    return true;
}

bool SimpleHttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers, 
                          std::string& response) {
    ESP_LOGI(TAG, "Simple HTTP GET from: %s", url.c_str());
    
    // Log headers
    for (const auto& header : headers) {
        ESP_LOGI(TAG, "Header: %s = %s", header.first.c_str(), header.second.c_str());
    }
    
    // Return a mock response
    response = "{\"mock\": \"response\", \"status\": \"success\"}";
    return true;
} 