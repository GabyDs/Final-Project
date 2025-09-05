/**
 * @file sd_card_driver.c
 * @brief SD card driver implementation
 */

#include "sd_card_driver.h"
#include "app_config.h"
#include <esp_log.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sd_test_io.h"

#if SOC_SDMMC_IO_POWER_EXTERNAL
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#endif

static const char *TAG = "sd_card_driver";
static sdmmc_card_t *sd_card = NULL;

#ifdef CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS
/* Pin configuration for debugging */
static const char *pin_names[] = {"CLK", "CMD", "D0", "D1", "D2", "D3"};
static const int pins[] = {
    CONFIG_EXAMPLE_PIN_CLK,
    CONFIG_EXAMPLE_PIN_CMD,
    CONFIG_EXAMPLE_PIN_D0
#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    ,CONFIG_EXAMPLE_PIN_D1,
    CONFIG_EXAMPLE_PIN_D2,
    CONFIG_EXAMPLE_PIN_D3
#endif
};

#if CONFIG_EXAMPLE_ENABLE_ADC_FEATURE
static const int adc_channels[] = {
    CONFIG_EXAMPLE_ADC_PIN_CLK,
    CONFIG_EXAMPLE_ADC_PIN_CMD,
    CONFIG_EXAMPLE_ADC_PIN_D0
#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    ,CONFIG_EXAMPLE_ADC_PIN_D1,
    CONFIG_EXAMPLE_ADC_PIN_D2,
    CONFIG_EXAMPLE_ADC_PIN_D3
#endif
};
#endif // CONFIG_EXAMPLE_ENABLE_ADC_FEATURE

static pin_configuration_t pin_config = {
    .names = pin_names,
    .pins = pins,
#if CONFIG_EXAMPLE_ENABLE_ADC_FEATURE
    .adc_channels = adc_channels,
#endif
};
#endif // CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS

esp_err_t sd_card_init(void)
{
    ESP_LOGI(TAG, "Initializing SD card...");
    
    /* Configure filesystem mount options */
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    /* Configure SDMMC host */
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    
#if CONFIG_EXAMPLE_SDMMC_SPEED_HS
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
#elif CONFIG_EXAMPLE_SDMMC_SPEED_UHS_I_SDR50
    host.slot = SDMMC_HOST_SLOT_0;
    host.max_freq_khz = SDMMC_FREQ_SDR50;
    host.flags &= ~SDMMC_HOST_FLAG_DDR;
#elif CONFIG_EXAMPLE_SDMMC_SPEED_UHS_I_DDR50
    host.slot = SDMMC_HOST_SLOT_0;
    host.max_freq_khz = SDMMC_FREQ_DDR50;
#endif

    /* Initialize power control if needed */
#if CONFIG_EXAMPLE_SD_PWR_CTRL_LDO_INTERNAL_IO
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = CONFIG_EXAMPLE_SD_PWR_CTRL_LDO_IO_ID,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;

    esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LDO power control driver: %s", esp_err_to_name(ret));
        return ret;
    }
    host.pwr_ctrl_handle = pwr_ctrl_handle;
#endif

    /* Configure SD card slot */
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
#if EXAMPLE_IS_UHS1
    slot_config.flags |= SDMMC_SLOT_FLAG_UHS1;
#endif

#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    slot_config.width = 4;
#else
    slot_config.width = 1;
#endif

    /* Configure GPIO pins for SD card */
#ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = CONFIG_EXAMPLE_PIN_CLK;
    slot_config.cmd = CONFIG_EXAMPLE_PIN_CMD;
    slot_config.d0 = CONFIG_EXAMPLE_PIN_D0;
#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    slot_config.d1 = CONFIG_EXAMPLE_PIN_D1;
    slot_config.d2 = CONFIG_EXAMPLE_PIN_D2;
    slot_config.d3 = CONFIG_EXAMPLE_PIN_D3;
#endif
#endif

    /* Enable internal pullups for debugging */
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    /* Mount the filesystem */
    ESP_LOGI(TAG, "Mounting filesystem...");
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &sd_card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                         "Consider enabling CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card: %s. "
                         "Make sure SD card lines have pull-up resistors.", esp_err_to_name(ret));
#ifdef CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS
            check_sd_card_pins(&pin_config, sizeof(pins) / sizeof(pins[0]));
#endif
        }
        return ret;
    }
    
    ESP_LOGI(TAG, "Filesystem mounted successfully");
    sdmmc_card_print_info(stdout, sd_card);
    
    return ESP_OK;
}

void sd_card_cleanup(void)
{
    if (sd_card) {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, sd_card);
        ESP_LOGI(TAG, "SD card unmounted");
        sd_card = NULL;
    }
    
#if CONFIG_EXAMPLE_SD_PWR_CTRL_LDO_INTERNAL_IO
    // Note: In real implementation, you'd need to track the power control handle
    // and call sd_pwr_ctrl_del_on_chip_ldo() here
#endif
}

sdmmc_card_t* sd_card_get_handle(void)
{
    return sd_card;
}

esp_err_t sd_card_format(void)
{
    if (sd_card == NULL) {
        ESP_LOGE(TAG, "SD card not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Formatting SD card...");
    esp_err_t ret = esp_vfs_fat_sdcard_format(MOUNT_POINT, sd_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to format SD card: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "SD card formatted successfully");
    return ESP_OK;
}
