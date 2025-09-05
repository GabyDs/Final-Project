# Camera SD Card Example - Modular Structure

This example has been refactored into a modular structure for better maintainability and reusability. The application captures a single photo and saves it to the SD card.

## File Structure

### Core Application
- **`main.c`** - Main application entry point and high-level logic
- **`app_config.h`** - Application constants and pin definitions

### Camera Module
- **`camera_driver.h/.c`** - ESP32-CAM camera initialization and control
  - `camera_init()` - Initialize camera with predefined settings
  - `camera_capture_photo()` - Capture a photo and return frame buffer
  - `camera_return_frame_buffer()` - Return frame buffer to driver
  - `camera_is_supported()` - Check if camera is supported on platform

### SD Card Module
- **`sd_card_driver.h/.c`** - SDMMC SD card driver and filesystem management
  - `sd_card_init()` - Initialize and mount SD card
  - `sd_card_cleanup()` - Unmount and cleanup SD card
  - `sd_card_get_handle()` - Get SD card handle for direct operations
  - `sd_card_format()` - Format the SD card

### File Operations Module
- **`file_operations.h/.c`** - File I/O utilities for SD card
  - `file_write_binary()` - Write binary data to file (e.g., images)
  - `file_read_text()` - Read and display text file content
  - `file_write_text()` - Write text string to file

## Benefits of This Structure

1. **Modularity**: Each module has a specific responsibility
2. **Reusability**: Modules can be easily reused in other projects
3. **Testability**: Each module can be tested independently
4. **Maintainability**: Changes to one module don't affect others
5. **Readability**: Code is organized logically and easier to understand

## Configuration

The camera pin configuration is centralized in `app_config.h` and matches the AI-Thinker ESP32-CAM module pinout. Modify these definitions if using a different camera module.

## Building

The CMakeLists.txt has been updated to include all source files. Build the project normally with:

```bash
idf.py build
```

## Original File

The original monolithic code has been backed up as `sd_card_example_main.c.backup` for reference.
