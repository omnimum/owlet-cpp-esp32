idf_component_register(SRCS "main.cpp" "owlet_api.cpp" "simple_http_client.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi nvs_flash esp_netif esp_timer esp_event)

# Enable C++ exceptions
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions)