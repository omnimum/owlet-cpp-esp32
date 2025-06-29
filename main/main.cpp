#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

extern "C" void app_main(void)
{
    // Print to console using C++ iostream
    std::cout << "Hello Sahas!" << std::endl;
    
    // Also print using ESP-IDF logging for consistency
    ESP_LOGI("MAIN", "Hello Sahas!");
    
    // Keep the task alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        std::cout << "Hello Sahas!" << std::endl;
    }
} 