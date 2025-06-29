#pragma once

#include <string>
#include <optional>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http_client.h"

// Forward declarations
struct TokenDict;
struct SockData;
struct DevicesResponse;
struct PropertiesResponse;
class EspHttpClient;

// Error handling without exceptions (ESP32 builds with -fno-exceptions)
class OwletError {
public:
    explicit OwletError(const std::string& message) : message_(message) {}
    const char* what() const noexcept { return message_.c_str(); }
private:
    std::string message_;
};

class OwletCredentialsError : public OwletError {
public:
    explicit OwletCredentialsError(const std::string& message) : OwletError(message) {}
};

class OwletAuthenticationError : public OwletError {
public:
    explicit OwletAuthenticationError(const std::string& message) : OwletError(message) {}
};

class OwletConnectionError : public OwletError {
public:
    explicit OwletConnectionError(const std::string& message) : OwletError(message) {}
};

class OwletDevicesError : public OwletError {
public:
    explicit OwletDevicesError(const std::string& message) : OwletError(message) {}
};

// Data structures
struct TokenDict {
    std::optional<std::string> api_token;
    std::optional<double> expiry;
    std::optional<std::string> refresh;
};

struct SockData {
    std::string product_name;
    std::string model;
    std::string dsn;
    std::string oem_model;
    std::string sw_version;
    std::string mac;
    std::string lan_ip;
    std::string connection_status;
    std::string device_type;
    std::string manuf_model;
};

struct DevicesResponse {
    std::vector<std::map<std::string, SockData>> response;
    std::optional<TokenDict> tokens;
};

struct PropertiesResponse {
    std::map<std::string, std::map<std::string, std::string>> response;
    std::optional<TokenDict> tokens;
};

// Main API class
class OwletAPI {
public:
    OwletAPI(
        const std::string& region,
        const std::optional<std::string>& user = std::nullopt,
        const std::optional<std::string>& password = std::nullopt,
        const std::optional<std::string>& token = std::nullopt,
        const std::optional<double>& expiry = std::nullopt,
        const std::optional<std::string>& refresh = std::nullopt,
        std::shared_ptr<HttpClient> http_client = nullptr
    );

    ~OwletAPI();

    // Authentication methods
    bool authenticate();
    bool validate_authentication();
    bool refresh_authentication();

    // Device management
    DevicesResponse get_devices(const std::vector<int>& versions = {3, 2});
    bool activate(const std::string& device_serial);
    PropertiesResponse get_properties(const std::string& device);

    // Token management
    TokenDict get_tokens() const;
    void close();

private:
    // Private methods
    bool password_verification();
    std::string get_mini_token(const std::string& id_token);
    std::optional<TokenDict> token_sign_in(const std::string& mini_token);
    void update_tokens(const std::optional<std::string>& new_token, 
                      const std::optional<double>& new_expiry, 
                      const std::optional<std::string>& new_refresh);
    bool is_valid_version(const std::string& dsn, const std::vector<int>& versions);
    std::optional<std::string> request(const std::string& method, const std::string& url, 
                                      const std::optional<std::map<std::string, std::string>>& data = std::nullopt);

    // Member variables
    std::string region_;
    std::optional<std::string> user_;
    std::optional<std::string> password_;
    std::optional<std::string> auth_token_;
    std::optional<double> expiry_;
    std::optional<std::string> refresh_;
    bool tokens_changed_;
    std::shared_ptr<HttpClient> http_client_;
    std::map<std::string, std::string> headers_;
    std::string api_url_;

    // Region information
    struct RegionInfo {
        std::string url_base;
        std::string url_mini;
        std::string url_signin;
        std::string api_key;
    };

    static std::map<std::string, RegionInfo> region_info_;
    static void initialize_region_info();
}; 