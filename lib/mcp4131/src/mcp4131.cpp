#include "mcp4131.hpp"

#define WIPER_RESISTANCE 75
#define FULL_SCALE_RESISTANCE 100000
#define MCP4131_RESOLUTION_OHMS (FULL_SCALE_RESISTANCE / 128)

#define TCON_R0B_BM (1 << 0)
#define TCON_R0W_BM (1 << 1)
#define TCON_R0A_BM (1 << 2)
#define TCON_R0HW_BM (1 << 3)

void MCP4131::init(SPIClass *spi_bus_ptr, int mcp4131_cs_pin)
{
    this->_spi_bus_handle = spi_bus_ptr;
    this->_cs_pin = mcp4131_cs_pin;
    pinMode(this->_cs_pin, OUTPUT);
    digitalWrite(this->_cs_pin, HIGH);
    this->_spi_bus_handle->begin(this->_cs_pin);
}

void MCP4131::set_output_resistance(uint32_t resistance_ohms)
{
    /* Calculate the required R_AB steps required */
    uint8_t value = 0;
    if (resistance_ohms > MCP4131_RESOLUTION_OHMS)
    {
        value = (resistance_ohms - WIPER_RESISTANCE) / MCP4131_RESOLUTION_OHMS;
    }
    if (this->_current_resistance != value)
    {
        _update_register(VOLATILE_WIPER_0, WRITE, value);
    }
}

void MCP4131::connect_wiper(void)
{
    this->_tcon_register_value |= TCON_R0W_BM;
    this->_update_register(VOLATILE_TCON_REGISTER, WRITE, _tcon_register_value);
}

void MCP4131::disconnect_wiper(void)
{
    this->_tcon_register_value &= ~TCON_R0W_BM;
    this->_update_register(VOLATILE_TCON_REGISTER, WRITE, _tcon_register_value);
}

void MCP4131::_update_register(MCP4131_Register_Address_t address,
                               Command_t command, uint16_t value)
{
    /* Mask all data fields with bit-size to ensure correct packet gen */
    uint16_t packet = ((uint8_t)address & 0b1111) << 12;
    packet |= ((uint8_t)command & 0b11) << 10;
    packet |= (value & 0b111111111);

    this->_spi_bus_handle->beginTransaction(
        SPISettings(250000, MSBFIRST, SPI_MODE0));
    digitalWrite(this->_cs_pin, LOW);
    delayMicroseconds(1);
    this->_spi_bus_handle->transfer16(address);
    this->_spi_bus_handle->transfer16(value);
    this->_spi_bus_handle->endTransaction();
    digitalWrite(this->_cs_pin, HIGH);
}
