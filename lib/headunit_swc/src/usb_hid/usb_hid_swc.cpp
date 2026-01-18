#include "usb_hid_swc.hpp"
#include "ch32x035_usbfs_device.h"

#define USB_NEXT_TRACK_COMMAND     (1 << 0)
#define USB_PREVIOUS_TRACK_COMMAND (1 << 1)
#define USB_STOP_COMMAND           (1 << 2)
#define USB_PLAY_PAUSE_COMMAND     (1 << 3)
#define USB_MUTE_COMMAND           (1 << 4)
#define USB_VOLUME_UP_COMMAND      (1 << 5)
#define USB_VOLUME_DOWN_COMMAND    (1 << 6)
#define USB_NEW_PACKET_DELAY_MS    50

void USB_HID_SWC::init_usb_hid_swc(void) {
  /* Usb Init */
  USBFS_RCC_Init();
  USBFS_Device_Init();
  USB_Sleep_Wakeup_CFG();
}

void USB_HID_SWC::on_encoder_rotation(bool cw_rotation) {
  if (USBFS_DevEnumStatus) {
    uint8_t command =
        (cw_rotation) ? USB_VOLUME_UP_COMMAND : USB_VOLUME_DOWN_COMMAND;
    this->_send_keyboard_command(command);
  }
}

void USB_HID_SWC::on_button_short_press() {
  this->_send_keyboard_command(USB_MUTE_COMMAND);
}

void USB_HID_SWC::on_button_held() {
  this->_send_keyboard_command(USB_NEXT_TRACK_COMMAND);
}

void USB_HID_SWC::on_button_double_press() {
  this->_send_keyboard_command(USB_PREVIOUS_TRACK_COMMAND);
}

void USB_HID_SWC::_send_keyboard_command(uint8_t command) {
  memset(this->_KB_Data_Pack, 0x00, sizeof(this->_KB_Data_Pack));
  this->_KB_Data_Pack[0] = 0x01; // Report ID is 0x01
  this->_KB_Data_Pack[1] = command;
  if (USBFS_DevEnumStatus) {
    USBFS_Endp_DataUp(DEF_UEP1, this->_KB_Data_Pack,
                      sizeof(this->_KB_Data_Pack), DEF_UEP_CPY_LOAD);
    delay(10);                     // Allow 10ms for packet to be read
    this->_KB_Data_Pack[1] = 0x00; // Reset key press list to nothing
    USBFS_Endp_DataUp(DEF_UEP1, this->_KB_Data_Pack,
                      sizeof(this->_KB_Data_Pack), DEF_UEP_CPY_LOAD);
  }
  delay(USB_NEW_PACKET_DELAY_MS);
}
