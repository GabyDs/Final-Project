/**
 * ESP-IDF Camera and SD Card Example - Main Application
 *
 * This example demonstrates how to:
 * - Initialize and configure an ESP32-CAM module
 * - Mount an SD card using SDMMC interface
 * - Capture a single photo and save it to the SD card
 *
 * Hardware Requirements:
 * - ESP32-CAM module (AI-Thinker or compatible)
 * - SD card inserted into the module
 */

/* Standard library includes */
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

/* ESP-IDF includes */
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>

/* FreeRTOS includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/queue.h"

/* Application modules */
#include "app_config.h"
#include "camera_driver.h"
#include "sd_card_driver.h"
#include "file_operations.h"
#include "driver/gpio.h"

#define PIR_SENSOR_PIN 12 // GPIO 12

static const char *TAG = "camera_sd_example";
uint8_t intr_flag = 0;

// ISR handler for GPIO interrupt
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    // ESP_LOGI(TAG, "Rising edge detected on GPIO 12!");
    intr_flag = 1;
}

/**
 * @brief Capture and save a photo to SD card
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t capture_and_save_photo(void)
{
    if (!camera_is_supported())
    {
        ESP_LOGW(TAG, "Camera not supported on this platform");
        return ESP_ERR_NOT_SUPPORTED;
    }

    camera_fb_t *frame_buffer = camera_capture_photo();
    if (frame_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to capture photo");
        return ESP_FAIL;
    }

    /* Save photo to SD card */
    const char *photo_path = MOUNT_POINT "/picture.jpg";
    esp_err_t ret = file_write_binary(photo_path, frame_buffer->buf, frame_buffer->len);

    /* Return the frame buffer */
    camera_return_frame_buffer(frame_buffer);

    return ret;
}

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    // Install GPIO ISR service and add handler
    gpio_install_isr_service(0); // already installed
    gpio_isr_handler_add(PIR_SENSOR_PIN, gpio_isr_handler, NULL);

    // Configure GPIO 12 as input with pull-down and interrupt on rising edge
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << PIR_SENSOR_PIN),
        .pull_down_en = 1,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Starting Camera SD Card Example");

    /* Initialize camera */
    if (camera_is_supported())
    {
        if (camera_init() != ESP_OK)
        {
            ESP_LOGE(TAG, "Camera initialization failed, exiting");
            return;
        }
    }
    else
    {
        ESP_LOGW(TAG, "Camera not supported, continuing with SD card only");
    }

    /* Initialize SD card */
    if (sd_card_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card initialization failed, exiting");
        return;
    }

#ifdef CONFIG_EXAMPLE_FORMAT_SD_CARD
    /* Format SD card if requested */
    if (sd_card_format() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card formatting failed, exiting");
        sd_card_cleanup();
        return;
    }
#endif

    // Configure GPIO 12 as input with pull-down and interrupt on rising edge
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << PIR_SENSOR_PIN),
        .pull_down_en = 1,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    // Install GPIO ISR service and add handler
    // gpio_install_isr_service(0); // already installed
    gpio_isr_handler_add(PIR_SENSOR_PIN, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "Initiating camera warm-up delay (3 seconds)...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    /* Warm-up loop to discard first few frames */
    for (int i = 0; i < 100; i++)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Failed to get frame buffer during warm-up");
            continue;
        }
        esp_camera_fb_return(fb);
    }

    /* Capture a single photo */
    ESP_LOGI(TAG, "Capturing a single photo...");
    esp_err_t ret = capture_and_save_photo();
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Photo captured and saved successfully!");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to capture/save photo: %s", esp_err_to_name(ret));
    }

    /* Cleanup and exit */
    sd_card_cleanup();
    ESP_LOGI(TAG, "Application completed, entering idle state");

    /* Keep the task alive but idle */
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        if (intr_flag == 1) {
            ESP_LOGI(TAG, "Motion detected! Capturing photo...");
            // esp_err_t ret = capture_and_save_photo();
            // if (ret == ESP_OK)
            // {
            //     ESP_LOGI(TAG, "Photo captured and saved successfully!");
            // }
            // else
            // {
            //     ESP_LOGE(TAG, "Failed to capture/save photo: %s", esp_err_to_name(ret));
            // }
            intr_flag = 0;
        }
    }
}
