/* Self header */
#include "ssd1306.h"

/**
 *
 * @param[in] i2c_library
 * @param[in] i2c_address
 * @param[in] pin_res
 * @param[in] buffer A pointer to the buffer that will be used to store a local copy of the gdram, should be (m_active_width * (m_active_height / 8)) bytes.
 */
int ssd1306::setup(TwoWire& i2c_library, const uint8_t i2c_address, const int pin_res, uint8_t* const buffer) {

    /* Ensure i2c address is valid */
    if (i2c_address != 0x3C && i2c_address != 0x3D) {
        return -EINVAL;
    }

    /* Save parameters */
    m_interface = INTERFACE_I2C;
    m_i2c_library = &i2c_library;
    m_i2c_address = i2c_address;
    m_buffer = buffer;

    /* Perform reset */
    pinMode(pin_res, OUTPUT);
    digitalWrite(pin_res, LOW);
    delay(1);
    digitalWrite(pin_res, HIGH);
    delay(1);

    /* Configure panel */
    int res = 0;
    res |= command_send(COMMAND_DISPLAY_OFF);
    res |= command_send(COMMAND_FREQUENCY_SET, 0x80);
    res |= command_send(COMMAND_MULTIPLEX_SET, m_active_height - 1);
    res |= command_send(COMMAND_DISPLAY_OFFSET_SET, 0x00);
    res |= command_send(COMMAND_STARTLINE_SET | 0x00);
    res |= command_send(COMMAND_CHARGEPUMP_SET, 0x14);
    res |= command_send(COMMAND_MEMORYMODE_SET, 0x00);
    res |= command_send(COMMAND_SEGREMAP_SET | 0x01);
    res |= command_send(COMMAND_SCANDIRECTION_DECREASING);
    res |= command_send(COMMAND_PADS_CONFIGURATION, 0x02);
    res |= command_send(COMMAND_CONTRAST_SET, 0x64);
    res |= command_send(COMMAND_PRECHARGE_PERIOD_SET, 0xF1);
    res |= command_send(COMMAND_VCOMH_DESELECT_LEVEL_SET, 0x20);
    res |= command_send(COMMAND_ENTIREON_DISABLED);
    res |= command_send(COMMAND_INVERSION_DISABLED);
    res |= command_send(COMMAND_COLUMN_ADDRESS_SET, 0, m_active_width - 1);       // Column start and end
    res |= command_send(COMMAND_PAGE_ADDRESS_SET, 0, (m_active_height / 8) - 1);  // Page start and end
    for (size_t i = 0; i < (m_active_height / 8); i++) {
        for (size_t j = 0; j < m_active_width; j++) {
            res |= data_send(0x00);
        }
    }
    res |= command_send(COMMAND_DISPLAY_ON);
    if (res != 0) {
        return -EIO;
    }

    /* Return success */
    return 0;
}

/**
 *
 */
bool ssd1306::detect(void) {
    switch (m_interface) {
        case INTERFACE_I2C: {  // In I2C: ensure the device ack its address
            if (m_i2c_library != NULL) {
                m_i2c_library->beginTransmission(m_i2c_address);
                if (m_i2c_library->endTransmission() == 0) {
                    return true;
                }
            }
            return false;
        }

        case INTERFACE_SPI_3WIRES:
        case INTERFACE_SPI_4WIRES: {  // In SPI: there is no way to detect the device
            return true;
        }

        default: {
            return false;
        }
    }
}

/**
 *
 * @param[in] ratio
 * @return
 */
int ssd1306::brightness_set(const float ratio) {
    if (ratio < 0 || ratio > 1) {
        return -EINVAL;
    }
    return command_send(COMMAND_CONTRAST_SET, (uint8_t)(ratio * 255));
}

/**
 *
 */
int ssd1306::inverted_set(const bool inverted) {
    return command_send(inverted ? COMMAND_INVERSION_ENABLED : COMMAND_INVERSION_DISABLED);
}
void ssd1306::invertDisplay(bool i) {
    inverted_set(i);
}

/**
 *
 * @param dev_id
 * @return
 */
int ssd1306::clear(void) {

    /* Clear local buffer */
    for (uint8_t page = 0; page < m_active_height / 8; page++) {
        for (uint8_t column = 0; column < m_active_width; column++) {
            m_buffer[column + page * m_active_width] = 0;
        }
    }

    /* Store pending operation */
    m_redraw_x_panel_min = 0;
    m_redraw_x_panel_max = m_active_width - 1;
    m_redraw_y_panel_min = 0;
    m_redraw_y_panel_max = m_active_height - 1;

    /* Return success */
    return 0;
}

/**
 *
 * @param[in] x
 * @param[in] y
 * @param[in] color
 * @return
 */
int ssd1306::pixel_set(const uint8_t x, const uint8_t y, const uint16_t color) {

    /* Convert coordinates and ensure they are valid */
    uint8_t x_panel, y_panel;
    if (m_rotation_handle(x, y, x_panel, y_panel) < 0) {
        return -EINVAL;
    }

    /* Change the pixel within the page */
    uint8_t page_index = x_panel + (y_panel / 8) * m_active_width;
    uint8_t page = m_buffer[page_index];
    uint8_t page_modified = color ? (page | (1 << (y_panel & 7))) : (page & ~(1 << (y_panel & 7)));
    if (page_modified != page) {
        m_buffer[page_index] = page_modified;

        /* Store partial update information */
        if (x_panel < m_redraw_x_panel_min) m_redraw_x_panel_min = x_panel;
        if (x_panel > m_redraw_x_panel_max) m_redraw_x_panel_max = x_panel;
        if (y_panel < m_redraw_y_panel_min) m_redraw_y_panel_min = y_panel;
        if (y_panel > m_redraw_y_panel_max) m_redraw_y_panel_max = y_panel;
    }

    /* Return success */
    return 0;
}
void ssd1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
    pixel_set(x, y, color);
}

/**
 *
 */
int ssd1306::display(void) {
    int res;

    /* */
    uint8_t page_min = m_redraw_y_panel_min / 8;
    uint8_t page_max = m_redraw_y_panel_max / 8;
    uint8_t column_min = m_redraw_x_panel_min;
    uint8_t column_max = m_redraw_x_panel_max;

    /* */
    switch (m_interface) {
        case INTERFACE_I2C: {

            /* Set column and page addresses for partial update */
            res = 0;
            res |= command_send(COMMAND_COLUMN_ADDRESS_SET, column_min, column_max);
            res |= command_send(COMMAND_PAGE_ADDRESS_SET, page_min, page_max);
            if (res != 0) {
                return -EIO;
            }

            /* Send data */
            for (uint8_t page = page_min; page <= page_max; page++) {
                data_send(&m_buffer[page * m_active_width + column_min], (column_max - column_min) + 1);
            }
            break;
        }
        case INTERFACE_SPI_3WIRES:  // TODO
        case INTERFACE_SPI_4WIRES:  // TODO
        default: {
            return -EINVAL;
        }
    }

    /* Reset partial update region */
    m_redraw_x_panel_min = m_active_width - 1;
    m_redraw_x_panel_max = 0;
    m_redraw_y_panel_min = m_active_height - 1;
    m_redraw_y_panel_max = 0;

    /* Return success */
    return 0;
}

/**
 *
 * @param[in] command
 */
int ssd1306::command_send(const uint8_t command) {
    int res;

    switch (m_interface) {
        case INTERFACE_I2C: {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write(0x00);  // CO = 0, DC = 0
            m_i2c_library->write(command);
            res = m_i2c_library->endTransmission(true);
            if (res != 0) {
                return -EIO;
            }
            return 0;
        }

        case INTERFACE_SPI_3WIRES:  // TODO
        case INTERFACE_SPI_4WIRES:  // TODO
        default: {
            return -EINVAL;
        }
    }
}

/**
 *
 * @param[in] command
 * @param[in] parameter
 */
int ssd1306::command_send(const uint8_t command, const uint8_t parameter) {
    int res;
    switch (m_interface) {

        case INTERFACE_I2C: {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write(0x00);  // CO = 0, DC = 0
            m_i2c_library->write(command);
            m_i2c_library->write(parameter);
            res = m_i2c_library->endTransmission(true);
            if (res != 0) {
                return -EIO;
            }
            return 0;
        }

        case INTERFACE_SPI_3WIRES:  // TODO
        case INTERFACE_SPI_4WIRES:  // TODO
        default: {
            return -EINVAL;
        }
    }
}

/**
 *
 * @param[in] command
 * @param[in] parameter1
 * @param[in] parameter2
 */
int ssd1306::command_send(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2) {
    int res;
    switch (m_interface) {

        case INTERFACE_I2C: {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write(0x00);  // CO = 0, DC = 0
            m_i2c_library->write(command);
            m_i2c_library->write(parameter1);
            m_i2c_library->write(parameter2);
            res = m_i2c_library->endTransmission(true);
            if (res != 0) {
                return -EIO;
            }
            return 0;
        }

        case INTERFACE_SPI_3WIRES:  // TODO
        case INTERFACE_SPI_4WIRES:  // TODO
        default: {
            return -EINVAL;
        }
    }
}

/**
 *
 */
int ssd1306::data_send(const uint8_t data) {
    int res;
    switch (m_interface) {

        case INTERFACE_I2C: {
            m_i2c_library->beginTransmission(m_i2c_address);
            m_i2c_library->write(0x40);  // CO = 0, DC = 1
            m_i2c_library->write(data);
            res = m_i2c_library->endTransmission(true);
            if (res != 0) {
                return -EIO;
            }
            return 0;
        }

        case INTERFACE_SPI_3WIRES:  // TODO
        case INTERFACE_SPI_4WIRES:  // TODO
        default: {
            return -EINVAL;
        }
    }
}

/**
 *
 */
int ssd1306::data_send(uint8_t* const data, const size_t length) {
    int res;
    switch (m_interface) {

        case INTERFACE_I2C: {
            for (size_t i = 0; i < length;) {
                m_i2c_library->beginTransmission(m_i2c_address);
                m_i2c_library->write(0x40);  // CO = 0, DC = 1
                i += m_i2c_library->write(&data[i], length - i);
                res = m_i2c_library->endTransmission(true);
                if (res != 0) {
                    return -EIO;
                }
            }
            return 0;
        }

        case INTERFACE_SPI_3WIRES:  // TODO
        case INTERFACE_SPI_4WIRES:  // TODO
        default: {
            return -EINVAL;
        }
    }
}

/**
 *
 */
int ssd1306::m_rotation_handle(const uint8_t x, const uint8_t y, uint8_t& x_panel, uint8_t& y_panel) const {
    switch (rotation) {
        case 0: {
            if (x >= m_active_width || y >= m_active_height) return -EINVAL;
            x_panel = x;
            y_panel = y;
            break;
        }
        case 1: {
            if (x >= m_active_height || y >= m_active_width) return -EINVAL;
            x_panel = m_active_width - y - 1;
            y_panel = x;
            break;
        }
        case 2: {
            if (x >= m_active_width || y >= m_active_height) return -EINVAL;
            x_panel = m_active_width - x - 1;
            y_panel = m_active_height - y - 1;
            break;
        }
        case 3: {
            if (x >= m_active_height || y >= m_active_width) return -EINVAL;
            x_panel = y;
            y_panel = m_active_height - x - 1;
            break;
        }
        default: {
            return -EINVAL;
        }
    }
    return 0;
}
