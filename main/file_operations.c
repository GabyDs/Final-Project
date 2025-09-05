/**
 * @file file_operations.c
 * @brief File operation utilities implementation
 */

#include "file_operations.h"
#include "app_config.h"
#include <esp_log.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "file_operations";

esp_err_t file_write_binary(const char *path, const uint8_t *data, size_t size)
{
    ESP_LOGI(TAG, "Writing binary file: %s (%zu bytes)", path, size);
    
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return ESP_FAIL;
    }
    
    size_t written = fwrite(data, 1, size, file);
    fclose(file);
    
    if (written != size) {
        ESP_LOGE(TAG, "Failed to write complete data to file (wrote %zu of %zu bytes)", written, size);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Binary file written successfully: %zu bytes", written);
    return ESP_OK;
}

esp_err_t file_read_text(const char *path)
{
    ESP_LOGI(TAG, "Reading text file: %s", path);
    
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading: %s", path);
        return ESP_FAIL;
    }
    
    char line[EXAMPLE_MAX_CHAR_SIZE];
    if (fgets(line, sizeof(line), file) != NULL) {
        // Strip newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            *newline_pos = '\0';
        }
        ESP_LOGI(TAG, "File content: '%s'", line);
    }
    
    fclose(file);
    return ESP_OK;
}

esp_err_t file_write_text(const char *path, const char *text)
{
    ESP_LOGI(TAG, "Writing text file: %s", path);
    
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return ESP_FAIL;
    }
    
    int result = fputs(text, file);
    fclose(file);
    
    if (result == EOF) {
        ESP_LOGE(TAG, "Failed to write text to file");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Text file written successfully");
    return ESP_OK;
}
