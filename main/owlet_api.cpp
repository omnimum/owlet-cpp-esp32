#include "owlet_api.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <sstream>
#include <cstring>

static const char* TAG = "OWLET_API";

// Static member initialization
std::map<std::string, OwletAPI::RegionInfo> OwletAPI::region_info_;

void OwletAPI::initialize_region_info() {
    if (region_info_.empty()) {
        // World region
        region_info_["world"] = {
            "https://ads-field.aylanetworks.com",
            "https://ads-field.aylanetworks.com/api/v1/token_sign_in_for_cert",
            "https://ads-field.aylanetworks.com/api/v1/token_sign_in_for_cert",
            "AIzaSyD-JtmyJ8N5zK9ajD8WR87AiSx0RHgjADQ"
        };
        
        // Europe region
        region_info_["europe"] = {
            "https://ads-eu.aylanetworks.com",
            "https://ads-eu.aylanetworks.com/api/v1/token_sign_in_for_cert",
            "https://ads-eu.aylanetworks.com/api/v1/token_sign_in_for_cert",
            "AIzaSyD-JtmyJ8N5zK9ajD8WR87AiSx0RHgjADQ"
        };
    }
}

OwletAPI::OwletAPI(
    const std::string& region,
    const std::optional<std::string>& user,
    const std::optional<std::string>& password,
    const std::optional<std::string>& token,
    const std::optional<double>& expiry,
    const std::optional<std::string>& refresh,
    std::shared_ptr<HttpClient> http_client
) : region_(region),
    user_(user),
    password_(password),
    auth_token_(token),
    expiry_(expiry),
    refresh_(refresh),
    tokens_changed_(false),
    http_client_(http_client) {
    
    ESP_LOGI(TAG, "Initializing Owlet API for region: %s", region.c_str());
    
    // Initialize region info if not already done
    initialize_region_info();
    
    // Validate region
    if (region_info_.find(region) == region_info_.end()) {
        ESP_LOGE(TAG, "Supplied region not valid: %s", region.c_str());
        return;
    }
    
    // Set API URL
    api_url_ = region_info_[region].url_base;
    
    // Set auth token in headers if provided
    if (auth_token_) {
        headers_["Authorization"] = "auth_token " + *auth_token_;
    }
    
    ESP_LOGI(TAG, "Owlet API initialized successfully");
}

OwletAPI::~OwletAPI() {
    ESP_LOGI(TAG, "Owlet API destroyed");
}

bool OwletAPI::authenticate() {
    ESP_LOGI(TAG, "Starting authentication process");
    
    // Check if we have valid tokens
    if (validate_authentication()) {
        ESP_LOGI(TAG, "Authentication already valid");
        return true;
    }
    
    // Try to refresh if we have a refresh token
    if (refresh_ && refresh_authentication()) {
        ESP_LOGI(TAG, "Authentication refreshed successfully");
        return true;
    }
    
    // Full authentication process
    if (!user_ || !password_) {
        ESP_LOGE(TAG, "Username and password required for authentication");
        return false;
    }
    
    ESP_LOGI(TAG, "Performing password verification");
    if (!password_verification()) {
        ESP_LOGE(TAG, "Password verification failed");
        return false;
    }
    
    ESP_LOGI(TAG, "Authentication completed successfully");
    return true;
}

bool OwletAPI::validate_authentication() {
    if (!auth_token_ || !expiry_) {
        ESP_LOGI(TAG, "No auth token or expiry available");
        return false;
    }
    
    // Check if token is expired (with 5 minute buffer)
    int64_t now_seconds = esp_timer_get_time() / 1000000; // Convert microseconds to seconds
    
    if (now_seconds >= (*expiry_ - 300)) { // 5 minute buffer
        ESP_LOGI(TAG, "Auth token expired or will expire soon");
        return false;
    }
    
    ESP_LOGI(TAG, "Auth token is valid");
    return true;
}

bool OwletAPI::refresh_authentication() {
    if (!refresh_) {
        ESP_LOGI(TAG, "No refresh token available");
        return false;
    }
    
    ESP_LOGI(TAG, "Attempting to refresh authentication");
    
    // This would implement the refresh token logic
    // For now, return false to indicate refresh is not implemented
    ESP_LOGW(TAG, "Token refresh not yet implemented");
    return false;
}

bool OwletAPI::password_verification() {
    ESP_LOGI(TAG, "Starting password verification");
    
    if (!user_ || !password_) {
        ESP_LOGE(TAG, "Username or password not provided");
        return false;
    }
    
    std::string api_key = region_info_[region_].api_key;
    std::string url = "https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyPassword?key=" + api_key;
    
    // Prepare post data
    std::string post_data = "email=" + *user_ + "&password=" + *password_ + "&returnSecureToken=true";
    
    // Prepare headers
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/x-www-form-urlencoded"},
        {"X-Android-Package", "com.owletcare.owletcare"},
        {"X-Android-Cert", "2A3BC26DB0B8B0792DBE28E6FFDC2598F9B12B74"}
    };
    
    std::string response;
    if (!http_client_->post(url, post_data, headers, response)) {
        ESP_LOGE(TAG, "Password verification HTTP request failed");
        return false;
    }
    
    ESP_LOGI(TAG, "Password verification response: %s", response.c_str());
    
    // Parse response and extract tokens
    // For now, we'll just log the response and return true
    // In a full implementation, you'd parse the JSON response
    
    return true;
}

std::string OwletAPI::get_mini_token(const std::string& id_token) {
    ESP_LOGI(TAG, "Getting mini token");
    
    std::string url = region_info_[region_].url_mini;
    std::map<std::string, std::string> headers = {
        {"Authorization", id_token}
    };
    
    std::string response;
    if (!http_client_->get(url, headers, response)) {
        ESP_LOGE(TAG, "Failed to get mini token");
        return "";
    }
    
    ESP_LOGI(TAG, "Mini token response: %s", response.c_str());
    
    // Parse response and extract mini token
    // For now, return a placeholder
    return "mini_token_placeholder";
}

std::optional<TokenDict> OwletAPI::token_sign_in(const std::string& mini_token) {
    ESP_LOGI(TAG, "Performing token sign in");
    
    std::string url = region_info_[region_].url_signin;
    std::string post_data = "mini_token=" + mini_token;
    
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/x-www-form-urlencoded"}
    };
    
    std::string response;
    if (!http_client_->post(url, post_data, headers, response)) {
        ESP_LOGE(TAG, "Token sign in failed");
        return std::nullopt;
    }
    
    ESP_LOGI(TAG, "Token sign in response: %s", response.c_str());
    
    // Parse response and return token dict
    // For now, return a placeholder
    TokenDict tokens;
    tokens.api_token = "api_token_placeholder";
    tokens.expiry = (esp_timer_get_time() / 1000000) + 3600; // 1 hour from now
    tokens.refresh = "refresh_token_placeholder";
    
    return tokens;
}

void OwletAPI::update_tokens(const std::optional<std::string>& new_token, 
                            const std::optional<double>& new_expiry, 
                            const std::optional<std::string>& new_refresh) {
    bool changed = false;
    
    if (new_token && new_token != auth_token_) {
        auth_token_ = new_token;
        headers_["Authorization"] = "auth_token " + *auth_token_;
        changed = true;
    }
    
    if (new_expiry && new_expiry != expiry_) {
        expiry_ = new_expiry;
        changed = true;
    }
    
    if (new_refresh && new_refresh != refresh_) {
        refresh_ = new_refresh;
        changed = true;
    }
    
    if (changed) {
        tokens_changed_ = true;
        ESP_LOGI(TAG, "Tokens updated");
    }
}

bool OwletAPI::is_valid_version(const std::string& dsn, const std::vector<int>& versions) {
    // Implementation would check device version
    // For now, return true
    return true;
}

std::optional<std::string> OwletAPI::request(const std::string& method, const std::string& url, 
                                            const std::optional<std::map<std::string, std::string>>& data) {
    ESP_LOGI(TAG, "Making %s request to %s", method.c_str(), url.c_str());
    
    std::string response;
    bool success = false;
    
    if (method == "GET") {
        success = http_client_->get(url, headers_, response);
    } else if (method == "POST") {
        std::string post_data;
        if (data) {
            // Convert data map to post string
            for (const auto& pair : *data) {
                if (!post_data.empty()) post_data += "&";
                post_data += pair.first + "=" + pair.second;
            }
        }
        success = http_client_->post(url, post_data, headers_, response);
    }
    
    if (success) {
        ESP_LOGI(TAG, "Request successful, response: %s", response.c_str());
        return response;
    } else {
        ESP_LOGE(TAG, "Request failed");
        return std::nullopt;
    }
}

DevicesResponse OwletAPI::get_devices(const std::vector<int>& versions) {
    ESP_LOGI(TAG, "Getting devices");
    
    // This would implement the actual device retrieval
    // For now, return empty response
    DevicesResponse response;
    return response;
}

bool OwletAPI::activate(const std::string& device_serial) {
    ESP_LOGI(TAG, "Activating device: %s", device_serial.c_str());
    
    // This would implement device activation
    // For now, return true
    return true;
}

PropertiesResponse OwletAPI::get_properties(const std::string& device) {
    ESP_LOGI(TAG, "Getting properties for device: %s", device.c_str());
    
    // This would implement property retrieval
    // For now, return empty response
    PropertiesResponse response;
    return response;
}

TokenDict OwletAPI::get_tokens() const {
    TokenDict tokens;
    tokens.api_token = auth_token_;
    tokens.expiry = expiry_;
    tokens.refresh = refresh_;
    return tokens;
}

void OwletAPI::close() {
    ESP_LOGI(TAG, "Closing Owlet API");
    // Cleanup if needed
} 