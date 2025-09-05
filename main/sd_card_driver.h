/**
 * @file sd_card_driver.h
 * @brief SD card driver interface for SDMMC
 */

#pragma once

#include "esp_err.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and mount the SD card
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t sd_card_init(void);

/**
 * @brief Cleanup and unmount SD card
 */
void sd_card_cleanup(void);

/**
 * @brief Get the mounted SD card handle
 * @return Pointer to SD card handle, NULL if not initialized
 */
sdmmc_card_t* sd_card_get_handle(void);

/**
 * @brief Format the SD card
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t sd_card_format(void);

#ifdef __cplusplus
}
#endif
