#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "owlet_api.h"
#include "simple_http_client.h"

static const char* TAG = "MAIN";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting Owlet C++ ESP32 Application");
    
    // Create HTTP client first
    auto http_client = std::make_shared<SimpleHttpClient>();
    
    // Create Owlet API instance
    // You can pass username/password for authentication
    // or existing tokens if you have them
    OwletAPI api("world", "slwisidagama@gmail.com", "Yasanga.92", 
                 std::nullopt, std::nullopt, std::nullopt, http_client);
    
    ESP_LOGI(TAG, "Owlet API instance created successfully");
    
    // Authenticate with the API
    ESP_LOGI(TAG, "Attempting to authenticate...");
    bool auth_success = api.authenticate();
    
    if (auth_success) {
        ESP_LOGI(TAG, "Authentication successful!");
        
        // Get current tokens
        TokenDict tokens = api.get_tokens();
        if (tokens.api_token) {
            ESP_LOGI(TAG, "API Token: %s", tokens.api_token->c_str());
        }
        if (tokens.expiry) {
            ESP_LOGI(TAG, "Token expires at: %f", *tokens.expiry);
        }
        
        // You can now use other API methods like:
        // DevicesResponse devices = api.get_devices();
        // PropertiesResponse props = api.get_properties("device_serial");
        // bool activated = api.activate("device_serial");
        
    } else {
        ESP_LOGE(TAG, "Authentication failed!");
    }
    
    // Keep the task alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 second delay
        ESP_LOGI(TAG, "Main task still running...");
    }
} 