#include "esp_http_client.h"
#include "owlet_api.h"
#include "esp_log.h"
#include "esp_err.h"
#include <cstring>

static const char* TAG = "ESP_HTTP_CLIENT";

// Static member variables
std::string EspHttpClient::response_data_;
bool EspHttpClient::request_completed_ = false;
int EspHttpClient::status_code_ = 0;

EspHttpClient::EspHttpClient() {
    ESP_LOGI(TAG, "ESP HTTP Client initialized");
}

EspHttpClient::~EspHttpClient() {
    ESP_LOGI(TAG, "ESP HTTP Client destroyed");
}

esp_err_t EspHttpClient::http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP Client Error");
            request_completed_ = true;
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP Client Connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP Client Header Sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP Client Header Received");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP Client Data Received, len=%d", evt->data_len);
            if (evt->data) {
                response_data_.append((char*)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP Client Finished");
            request_completed_ = true;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP Client Disconnected");
            request_completed_ = true;
            break;
        default:
            break;
    }
    return ESP_OK;
}

bool EspHttpClient::post(const std::string& url, const std::string& data, 
                        const std::map<std::string, std::string>& headers, std::string& response) {
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = http_event_handler;
    config.method = HTTP_METHOD_POST;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return false;
    }

    // Set headers
    for (const auto& header : headers) {
        esp_http_client_set_header(client, header.first.c_str(), header.second.c_str());
    }

    // Set post data
    if (!data.empty()) {
        esp_http_client_set_post_field(client, data.c_str(), data.length());
    }

    // Reset static variables
    response_data_.clear();
    request_completed_ = false;
    status_code_ = 0;

    // Perform request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        status_code_ = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP POST Status = %d", status_code_);
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    // Wait for completion
    while (!request_completed_) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    response = response_data_;
    esp_http_client_cleanup(client);

    return (err == ESP_OK && status_code_ >= 200 && status_code_ < 300);
}

bool EspHttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers, 
                       std::string& response) {
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = http_event_handler;
    config.method = HTTP_METHOD_GET;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return false;
    }

    // Set headers
    for (const auto& header : headers) {
        esp_http_client_set_header(client, header.first.c_str(), header.second.c_str());
    }

    // Reset static variables
    response_data_.clear();
    request_completed_ = false;
    status_code_ = 0;

    // Perform request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        status_code_ = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP GET Status = %d", status_code_);
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // Wait for completion
    while (!request_completed_) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    response = response_data_;
    esp_http_client_cleanup(client);

    return (err == ESP_OK && status_code_ >= 200 && status_code_ < 300);
} 