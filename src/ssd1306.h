#ifndef SSD1306_H
#define SSD1306_H

/* Arduino libraries */
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

/* C/C++ libraries */
#include <errno.h>
#include <stdint.h>

/**
 *
 */
class ssd1306 : public Adafruit_GFX {

   public:
    /* Setup */
    ssd1306(uint8_t width, uint8_t height) : Adafruit_GFX(width, height), m_active_width(width), m_active_height(height), m_redraw_x_panel_min(width - 1), m_redraw_y_panel_min(height - 1) {}
    int setup(TwoWire& i2c_library, const uint8_t i2c_address, const int pin_res, uint8_t* const buffer);
    bool detect(void);
    int brightness_set(const float ratio);
    int inverted_set(const bool inverted);
    void invertDisplay(bool i);

    /* Pixel manipulation */
    int clear(void);
    int pixel_set(const uint8_t x, const uint8_t y, const uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    /* Output */
    int display(void);

    /* Commands */
    enum command {
        COMMAND_MEMORYMODE_SET = 0x20,
        COMMAND_COLUMN_ADDRESS_SET = 0x21,
        COMMAND_PAGE_ADDRESS_SET = 0x22,
        COMMAND_STARTLINE_SET = 0x40,
        COMMAND_CONTRAST_SET = 0x81,
        COMMAND_CHARGEPUMP_SET = 0x8D,
        COMMAND_SEGREMAP_SET = 0xA0,
        COMMAND_ENTIREON_DISABLED = 0xA4,
        COMMAND_ENTIREON_ENABLED = 0xA5,
        COMMAND_INVERSION_DISABLED = 0xA6,
        COMMAND_INVERSION_ENABLED = 0xA7,
        COMMAND_MULTIPLEX_SET = 0xA8,
        COMMAND_DISPLAY_OFF = 0xAE,
        COMMAND_DISPLAY_ON = 0xAF,
        COMMAND_SCANDIRECTION_INCREASING = 0xC0,
        COMMAND_SCANDIRECTION_DECREASING = 0xC8,
        COMMAND_PADS_CONFIGURATION = 0xDA,
        COMMAND_DISPLAY_OFFSET_SET = 0xD3,
        COMMAND_FREQUENCY_SET = 0xD5,
        COMMAND_PRECHARGE_PERIOD_SET = 0xD9,
        COMMAND_VCOMH_DESELECT_LEVEL_SET = 0xDB,
    };
    int command_send(const uint8_t command);
    int command_send(const uint8_t command, const uint8_t parameter);
    int command_send(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2);
    int data_send(const uint8_t data);
    int data_send(uint8_t* const data, const size_t length);

   protected:
    uint8_t m_active_width = 0;
    uint8_t m_active_height = 0;
    TwoWire* m_i2c_library = NULL;
    uint8_t m_i2c_address = 0;
    uint8_t* m_buffer = NULL;
    enum interface {
        INTERFACE_NONE,
        INTERFACE_I2C,
        INTERFACE_SPI_4WIRES,  // TODO
        INTERFACE_SPI_3WIRES,  // TODO
    } m_interface = INTERFACE_NONE;
    uint8_t m_redraw_x_panel_min;
    uint8_t m_redraw_x_panel_max = 0;
    uint8_t m_redraw_y_panel_min;
    uint8_t m_redraw_y_panel_max = 0;
    int m_rotation_handle(const uint8_t x, const uint8_t y, uint8_t& x_panel, uint8_t& y_panel) const;
};

#endif
