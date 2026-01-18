#pragma once

#include <SPI.h>

typedef enum {
  VOLATILE_WIPER_0,
  VOLATILE_WIPER_1,
  RESERVED,
  RESERVED1,
  VOLATILE_TCON_REGISTER,
  STATUS_REGISTER,
  RESERVED2,
} MCP4131_Register_Address_t;

typedef enum {
  B_TERMINAL,
  WIPER_TERMINAL,
  A_TERMINAL,
} Terminal_Type_t;

typedef enum {
  WRITE,
  INCREMENT,
  DECREMENT,
  READ,
} Command_t;

class MCP4131 {
public:
  void init(SPIClass *spi_bus_ptr, int mcp4131_cs_pin);
  void set_output_resistance(uint32_t resistance_ohms);
  void connect_wiper(void);
  void disconnect_wiper(void);

private:
  void _update_register(MCP4131_Register_Address_t address, Command_t command,
                        uint16_t value);

  SPIClass *_spi_bus_handle;
  int       _cs_pin;
  uint8_t   _current_resistance = 0x40; // Default at power-up for 7-bit devices
  uint16_t  _tcon_register_value = 0x1FF; // Default at start-up

  // Device properties
  uint32_t _full_scale_resistance = 100000;
  uint16_t _rs                    = _full_scale_resistance / 128;
};

extern MCP4131 mcp4131;