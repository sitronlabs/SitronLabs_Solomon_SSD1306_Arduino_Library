[![Designed by Sitron Labs](https://img.shields.io/badge/Designed_by-Sitron_Labs-FCE477.svg)](https://www.sitronlabs.com/)
[![Join the Discord community](https://img.shields.io/discord/552242187665145866.svg?logo=discord&logoColor=white&label=Discord&color=%237289da)](https://discord.gg/btnVDeWhfW)
![License](https://img.shields.io/github/license/sitronlabs/SitronLabs_Solomon_SSD1306_Arduino_Library.svg)
![Latest Release](https://img.shields.io/github/release/sitronlabs/SitronLabs_Solomon_SSD1306_Arduino_Library.svg)
[![Arduino Library Manager](https://www.ardu-badge.com/badge/Sitron%20Labs%20SSD1306%20Arduino%20Library.svg)](https://www.ardu-badge.com/Sitron%20Labs%20SSD1306%20Arduino%20Library)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/sitronlabs/library/Sitron_Labs_SSD1306_Arduino_Library.svg)](https://registry.platformio.org/libraries/sitronlabs/Sitron_Labs_SSD1306_Arduino_Library)

# Sitron Labs Solomon Systech SSD1306 Arduino Library

Arduino library for interfacing with OLED displays based on the Solomon Systech SSD1306 controller.

## Description

The SSD1306 is a single-chip CMOS OLED/PLED driver with controller for organic/polymer light emitting diode dot-matrix graphic display systems. It supports displays with resolutions up to 128x64 pixels and provides a simple interface for drawing graphics and text. This library provides a buffered interface that extends Adafruit's GFX library, allowing you to use all standard GFX drawing functions while maintaining efficient partial screen updates.

## Installation

### Arduino IDE

Install via the Arduino Library Manager by searching for "Sitron Labs SSD1306".

Alternatively, install manually:
1. Download or clone this repository
2. Place it in your Arduino `libraries` folder
3. Restart the Arduino IDE

### PlatformIO

Install via the PlatformIO Library Manager by searching for "Sitron Labs SSD1306".

Alternatively, add the library manually to your `platformio.ini` file:

```ini
lib_deps = 
    https://github.com/sitronlabs/SitronLabs_Solomon_SSD1306_Arduino_Library.git
```

## Hardware Connections

Connect the SSD1306 to your Arduino using I2C:

- VCC → 3.3V or 5V (check your board's specifications)
- GND → GND
- SDA → SDA (I2C Data)
- SCL → SCL (I2C Clock)
- RES → Any digital pin (reset pin, configure in code)

The I2C address is typically 0x3C or 0x3D (check your display module's documentation).

## Supported Interfaces

| Interface              | Status | Notes |
|------------------------|:------:|-------|
| I2C, Unbuffered        | ❌ | Technically impossible as driver doesn't allow read access to display ram. |
| I2C, Buffered          | ✔️ | Works. |
| SPI, 3-Wires, Buffered | ❌ | Needs to be implemented. |
| SPI, 4-Wires, Buffered | ❌ | Needs to be implemented. |

## Usage

### Basic Example

```cpp
#include <Wire.h>
#include <ssd1306.h>

// Display buffer (128x64 = 1024 bytes, 128x32 = 512 bytes)
// Buffer size = width * (height / 8)
uint8_t display_buffer[128 * 64 / 8];

// Create display object (width, height)
ssd1306 display(128, 64);

// I2C address (typically 0x3C or 0x3D)
const uint8_t I2C_ADDRESS = 0x3C;

// Reset pin
const int PIN_RES = 4;

void setup() {
  Serial.begin(9600);
  
  // Initialize I2C
  Wire.begin();
  
  // Setup the SSD1306 (I2C library, I2C address, reset pin, buffer)
  if (display.setup(Wire, I2C_ADDRESS, PIN_RES, display_buffer) != 0) {
    Serial.println("Failed to setup SSD1306");
    return;
  }
  
  // Detect the display
  if (!display.detect()) {
    Serial.println("SSD1306 not detected");
    return;
  }
  
  // Clear the display
  display.clear();
  display.display();
  
  Serial.println("SSD1306 initialized");
}

void loop() {
  // Clear display
  display.clear();
  
  // Use Adafruit GFX functions
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Hello, World!");
  display.println("SSD1306 OLED");
  
  // Draw a rectangle
  display.drawRect(10, 20, 100, 30, SSD1306_WHITE);
  
  // Draw a filled circle
  display.fillCircle(64, 50, 10, SSD1306_WHITE);
  
  // Update the display (only changed regions are sent)
  display.display();
  
  delay(1000);
}
```

### Using GFX Library Functions

```cpp
#include <Wire.h>
#include <ssd1306.h>

uint8_t display_buffer[128 * 64 / 8];
ssd1306 display(128, 64);
const uint8_t I2C_ADDRESS = 0x3C;
const int PIN_RES = 4;

void setup() {
  Wire.begin();
  display.setup(Wire, I2C_ADDRESS, PIN_RES, display_buffer);
  display.detect();
  
  // Clear and display
  display.clear();
  display.display();
}

void loop() {
  display.clear();
  
  // All Adafruit GFX functions are available
  display.drawLine(0, 0, 127, 63, SSD1306_WHITE);
  display.drawTriangle(20, 20, 40, 10, 30, 30, SSD1306_WHITE);
  display.fillRect(50, 20, 30, 20, SSD1306_WHITE);
  
  // Text functions
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 40);
  display.print("GFX!");
  
  display.display();
  delay(1000);
}
```

## API Reference

### setup(TwoWire &i2c_library, uint8_t i2c_address, int pin_res, uint8_t *buffer)

Initializes the SSD1306 display.

- `i2c_library`: I2C library instance to use (typically `Wire`)
- `i2c_address`: I2C address (0x3C or 0x3D)
- `pin_res`: GPIO pin number connected to the display's reset pin
- `buffer`: Pointer to display buffer (must be `width * (height / 8)` bytes)

Returns 0 on success, or -EINVAL if the I2C address is invalid, or -EIO on I2C communication failure.

### detect(void)

Detects if an SSD1306 display is present on the I2C bus.

Returns true if display is detected, false otherwise.

### clear(void)

Clears the display buffer (sets all pixels to off).

Returns 0 on success.

### display(void)

Updates the display with the current buffer contents. Only changed regions are sent to the display for efficiency.

Returns 0 on success, or -EINVAL if the interface is not properly initialized, or -EIO on I2C communication failure.

### brightness_set(float ratio)

Sets the display brightness.

- `ratio`: Brightness ratio from 0.0 (off) to 1.0 (maximum)

Returns 0 on success, or -EINVAL if ratio is out of range, or -EIO on I2C communication failure.

### inverted_set(bool inverted)

Sets the display inversion state.

- `inverted`: true to invert colors, false for normal

Returns 0 on success, or -EIO on I2C communication failure.

### pixel_set(uint8_t x, uint8_t y, uint16_t color)

Sets a single pixel in the buffer.

- `x`: X coordinate (0 to width-1)
- `y`: Y coordinate (0 to height-1)
- `color`: Pixel color (0 for off, non-zero for on)

Returns 0 on success, or -EINVAL if coordinates are out of range.

### drawPixel(int16_t x, int16_t y, uint16_t color)

Adafruit GFX compatible function to draw a pixel. Calls `pixel_set()` internally.

### invertDisplay(bool i)

Adafruit GFX compatible function to invert the display. Calls `inverted_set()` internally.

### GFX Library Functions

Since this library extends `Adafruit_GFX`, all standard GFX functions are available:
- `drawLine()`, `drawRect()`, `fillRect()`
- `drawCircle()`, `fillCircle()`
- `drawTriangle()`, `fillTriangle()`
- `print()`, `println()`, `setCursor()`, `setTextSize()`, `setTextColor()`
- And many more...

## Specifications

- Display resolution: Up to 128x64 pixels (configurable)
- Communication interface: I2C (SPI support planned)
- I2C addresses: 0x3C or 0x3D
- Buffer mode: Buffered (local frame buffer)
- Partial updates: Supported (only changed regions are updated)
- GFX library: Relies on the Adafruit GFX Library
- Rotation: Supported (0°, 90°, 180°, 270°)

