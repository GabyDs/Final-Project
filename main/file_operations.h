/**
 * @file file_operations.h
 * @brief File operation utilities for SD card
 */

#pragma once

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write data to a file on the SD card
 * @param path File path to write to
 * @param data Data buffer to write
 * @param size Size of data to write
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t file_write_binary(const char *path, const uint8_t *data, size_t size);

/**
 * @brief Read and display content from a text file
 * @param path File path to read from
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t file_read_text(const char *path);

/**
 * @brief Write text data to a file
 * @param path File path to write to
 * @param text Text string to write
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t file_write_text(const char *path, const char *text);

#ifdef __cplusplus
}
#endif
