#include "config_bootloader.hpp"
#include <version.h>

#define DEF_USBD_UEP0_SIZE  64
#define DEF_USBD_ENDP2_SIZE 64
#define DEF_USBD_ENDP3_SIZE 64
#define USB_COMMS_ENDPOINT  0x02

/* VID and PID for RE_SWC by Pounamu Electronics */
#define DEF_USB_VID 0x1209
#define DEF_USB_PID 0x6789

const char *firmware_version_str_ptr = FIRMWARE_VERSION;

enum BootloaderCommands {
  BOOTLOADER_COMMAND_AUTH                     = 0xA1,
  BOOTLOADER_COMMAND_END_AND_RESET            = 0xA2,
  BOOTLOADER_COMMAND_WRITE_EEPROM             = 0xA9,
  BOOTLOADER_COMMAND_VERIFY_EEPROM            = 0xA3,
  BOOTLOADER_COMMAND_GET_FIRMWARE_VERSION_STR = 0xB0,
};

EEPROMClass *eeprom_ptr;

/* USB Descriptors for the bootloader */
/* Device Descriptor */
const uint8_t MyDevDescr[] = {
    0x12,                        // bLength
    0x01,                        // bDescriptorType (Device)
    0x10,                        // bcdUSB 1.10 (lower byte)
    0x01,                        // bcdUSB 1.10 (upper byte)
    0xff,                        // bDeviceClass = vendor specific
    0x80,                        // bDeviceSubClass
    0x55,                        // bDeviceProtocol
    DEF_USBD_UEP0_SIZE,          // bMaxPacketSize0 64 == 0x40
    (uint8_t)DEF_USB_VID,        // idVendor  0x1209 (lower byte)
    (uint8_t)(DEF_USB_VID >> 8), // idVendor  0x1209 (upper byte)
    (uint8_t)DEF_USB_PID,        // idProduct 0x6789 (lower byte)
    (uint8_t)(DEF_USB_PID >> 8), // idProduct 0x6789 (upper byte)
    0x26,                        // bcdDevice 0x0026 (lower byte)
    0x00,                        // bcdDevice 0x0026 (upper byte)
    0x01, // iManufacturer (String Index) = no manufacturer data
    0x02, // iProduct (String Index) = no product data
    0x00, // iSerialNumber (String Index)  = no serial number data
    0x01, // bNumConfigurations 1
};

/* Configuration Descriptor */
const uint8_t MyCfgDescr[] = {
    /* Configure descriptor */
    0x09, // length = 9
    0x02, // type = configuration
    0x20, // total length (lower byte)
    0x00, // total length (upper byte)
    0x01, // num interfaces
    0x01, // config value
    0x00, // config index
    0x80, // config - bus pwoered, no remote start
    0x32, // max power (100mA)

    /* Interface 0 (CDC) descriptor */
    0x09, // length = 9
    0x04, // descriptor type = interface
    0x00, // interface number = 0
    0x00, // alternate setting = 0
    0x02, // number of endpoints = 2
    0xff, // interface class = vendor specific
    0x80, // interface subclass
    0x55, // interface protocol
    0x00, // interface index

    /* Endpoint descriptor */
    0x07,                         // length = 7
    0x05,                         // descriptor type = endpoint
    0x82,                         // endpoint address = 0x82 (IN Endpoint:2)
    0x02,                         // attributes = 0x02
    (uint8_t)DEF_USBD_ENDP2_SIZE, // endpoint size (lower byte)
    (uint8_t)(DEF_USBD_ENDP2_SIZE >> 8), // endpoint size (upper byte)
    0x00,                                // interval = 0

    /* Endpoint descriptor */
    0x07,                         // length = 7
    0x05,                         // descriptor type = endpoint
    0x02,                         // endpoint address = 0x02 (OUT Endpoint: 2)
    0x02,                         // attributes = 0x02
    (uint8_t)DEF_USBD_ENDP2_SIZE, // endpoint size (lower byte)
    (uint8_t)(DEF_USBD_ENDP2_SIZE >> 8), // endpoint size (upper byte)
    0x00,                                // interval = 0
};

/* Language Descriptor */
const uint8_t MyLangDescr[] = {
    0x04, // total length = 4
    0x03, // type = string
    0x09, // langugae code = US (lower byte)
    0x04, // language code = US (upper byte)
};

/* Manufacturer Descriptor */
const uint8_t MyManuInfo[] = {
    0x28, // total length
    0x03, // type = string
    'P',  0,   'o', 0,   'u', 0,   'n', 0,   'a', 0,   'm', 0,   'u',
    0,    ' ', 0,   'E', 0,   'l', 0,   'e', 0,   'c', 0,   't', 0,
    'r',  0,   'o', 0,   'n', 0,   'i', 0,   'c', 0,   's', 0,
};

/* Product Information */
const uint8_t MyProdInfo[] = {0x0E, 0x03, 'R', 0,   'E', 0,   '_',
                              0,    'S',  0,   'W', 0,   'C', 0};

Descriptor_Configs_t usb_bootloader_descriptors = {
    .device_descriptor       = MyDevDescr,
    .config_descriptor       = MyCfgDescr,
    .language_descriptor     = MyLangDescr,
    .manufacturer_descriptor = MyManuInfo,
    .prod_info_descriptor    = MyProdInfo,
};

void init_usb_bootloader(EEPROMClass *eeprom_handle) {
  eeprom_ptr = eeprom_handle;
  /* Usb Init */
  USBFS_RCC_Init();
  USBFS_Device_Init(ENABLE, &usb_bootloader_descriptors,
                    USB_DEVICE_MODE_BOOTLOADER);
}

void deinit_usb_bootloader(void) {
  USBFS_Device_Init(DISABLE, nullptr, USB_DEVICE_MODE_BOOTLOADER);
}

bool run_usb_bootloader_task(uint8_t eeprom_config_start_index) {
  bool bootloader_complete = false;
  /* We check to see if there is a packet to read or a packet to send */
  bool rx_waiting = is_endpoint_rx_pending(USB_COMMS_ENDPOINT);
  if (rx_waiting) {
    uint8_t local_rx_packet[64];
    uint8_t bytes_received = get_endpoint_rx_message(
        USB_COMMS_ENDPOINT, local_rx_packet, sizeof(local_rx_packet));
    if (bytes_received) {
      uint8_t tx_packet[64];
      uint8_t tx_packet_length;
      uint8_t values_length;
      uint8_t response_code   = 0xff; // Fail by default
      bool    send_ack_packet = true; // Only ack packet by default

      /* We have data to read here */
      switch (local_rx_packet[0]) {
      case ((uint8_t)BOOTLOADER_COMMAND_AUTH):
        response_code = 0x00;
        break;

      case ((uint8_t)BOOTLOADER_COMMAND_WRITE_EEPROM):
        values_length = local_rx_packet[1];
        if (values_length) {
          for (uint8_t index = 0; index < values_length; index++) {
            eeprom_ptr->write(eeprom_config_start_index + index,
                              local_rx_packet[2 + index]);
          }
          response_code = 0x00;
        }
        break;

      case ((uint8_t)BOOTLOADER_COMMAND_VERIFY_EEPROM):
        /* {command, length of values (including offset), 0x00, offset (4 bytes
         * (little endian), values}*/
        response_code = 0x00;
        break;

      case ((uint8_t)BOOTLOADER_COMMAND_GET_FIRMWARE_VERSION_STR):
        send_ack_packet = false; // We are sending a custom packet
        tx_packet[0]    = local_rx_packet[0];
        tx_packet[1]    = 0x00;
        tx_packet[2]    = strlen(firmware_version_str_ptr);
        tx_packet[3]    = 0x00;
        for (uint8_t i = 0; i < strlen(firmware_version_str_ptr); i++) {
          tx_packet[i + 4] = (uint8_t)firmware_version_str_ptr[i];
        }
        tx_packet_length = 4 + strlen(firmware_version_str_ptr);
        break;

      case ((uint8_t)BOOTLOADER_COMMAND_END_AND_RESET):
        bootloader_complete = true; // We can now exit the bootloader
        response_code       = 0x00;
        break;

      default:
        break;
      }

      if (send_ack_packet) {
        tx_packet[0]     = local_rx_packet[0];
        tx_packet[1]     = 0x00;
        tx_packet[2]     = 0x01;
        tx_packet[3]     = 0x00;
        tx_packet[4]     = response_code;
        tx_packet_length = 5;
      }
      USBFS_Endp_DataUp(USB_COMMS_ENDPOINT, tx_packet, tx_packet_length,
                        DEF_UEP_CPY_LOAD);
    }
  }
  return bootloader_complete;
}