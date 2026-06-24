#pragma once

#include <EEPROM.h>
#include <ch32x035_usbd.h>

void init_usb_bootloader(EEPROMClass *eeprom_handle);
void deinit_usb_bootloader(void);
bool run_usb_bootloader_task(uint8_t eeprom_config_start_index);