# Solomon Systech SSD1306 Arduino Library
Arduino library for OLED displays based on the Solomon Systech SSD1306 controller.

### Supported interfaces
| Interface              | Status | Notes |
|------------------------|:------:|-------|
| I2C, Unbuffered        | ❌ | Technically impossible as driver doesn't allow read access to display ram. |
| I2C, Buffered          | ✔️ | Works. |
| SPI, 3-Wires, Buffered | ❌ | Needs to be implemented. |
| SPI, 4-Wires, Buffered | ❌ | Needs to be implemented. |
