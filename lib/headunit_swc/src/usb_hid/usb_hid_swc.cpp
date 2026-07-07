#include "usb_hid_swc.hpp"
#include <ch32x035_usbd.h>

#define CONSUMER_REPORT_MASK_CW_ROTATE           (1 << 0)
#define CONSUMER_REPORT_MASK_CCW_ROTATE          (1 << 1)
#define CONSUMER_REPORT_MASK_BUTTON_SHORT_PRESS  (1 << 2)
#define CONSUMER_REPORT_MASK_BUTTON_LONG_PRESS   (1 << 3)
#define CONSUMER_REPORT_MASK_BUTTON_DOUBLE_PRESS (1 << 4)

#define USB_STOP_COMMAND       (1 << 2)
#define USB_PLAY_PAUSE_COMMAND (1 << 3)

#define USB_UNKNOWN_COMMAND     0
#define USB_NEW_PACKET_DELAY_MS 50

#define DEF_USBD_UEP0_SIZE 64
#define DEF_USB_VID        0x1209
#define DEF_USB_PID        0x6789

/*******************************************************************************/
/* Device Descriptor */
const uint8_t MyDevDescr[] = {
    0x12, // bLength
    0x01, // bDescriptorType
    0x00,
    0x02,               // bcdUSB
    0x00,               // bDeviceClass
    0x00,               // bDeviceSubClass
    0x00,               // bDeviceProtocol
    DEF_USBD_UEP0_SIZE, // bMaxPacketSize0
    (uint8_t)DEF_USB_VID,
    (uint8_t)(DEF_USB_VID >> 8), // idVendor
    (uint8_t)DEF_USB_PID,
    (uint8_t)(DEF_USB_PID >> 8), // idProduct
    0x00,
    0x01, // bcdDevice
    0x01, // iManufacturer
    0x02, // iProduct
    0x00, // iSerialNumber
    0x01, // bNumConfigurations
};

/* Configuration Descriptor Set */
const uint8_t MyCfgDescr[] = {
    /* Configuration Descriptor */
    0x09, // bLength
    0x02, // bDescriptorType
    0x22, // wTotalLength (lower byte)
    0x00, // wTotalLength (upper byte)
    0x01, // bNumInterfaces
    0x01, // bConfigurationValue
    0x00, // iConfiguration
    0xA0, // bmAttributes: Bus Powered; Remote Wakeup
    0x32, // MaxPower: 100mA

    /* Interface Descriptor (HID Device) */
    0x09, // bLength
    0x04, // bDescriptorType
    0x00, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x01, // bNumEndpoints
    0x03, // bInterfaceClass
    0x00, // bInterfaceSubClass (none for HID)
    0x00, // bInterfaceProtocol (none for HID)
    0x00, // iInterface

    /* HID Descriptor */
    0x09, // bLength
    0x21, // bDescriptorType
    0x11, // bcdHID (upper byte)
    0x01, // bcdHID (lower byte)
    0x00, // bCountryCode
    0x01, // bNumDescriptors
    0x22, // bDescriptorType (HID Report)
    0x27, // wDescriptorLength (lower byte)
    0x00, // wDescriptorLength (upper byte)

    /* Endpoint Descriptor (Consumer Report) */
    0x07, // bLength
    0x05, // bDescriptorType
    0x81, // bEndpointAddress: IN Endpoint 1
    0x03, // bmAttributes (Interrupt)
    0x02, // wMaxPacketSize (lower byte)
    0x00, // wMaxPacketSize (upper byte)
    0x0A, // bInterval: 10mS
};

/* Consumer Report Descriptor buffer */
uint8_t ConsumerRepDesc[128];

/* Qualifier Descriptor */
const uint8_t MyQuaDesc[] = {
    0x0A,       // bLength
    0x06,       // bDescriptorType
    0x00, 0x02, // bcdUSB
    0x00,       // bDeviceClass
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    0x40,       // bMaxPacketSize0
    0x00,       // bNumConfigurations
    0x00        // bReserved
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
const uint8_t MyProdInfo[] = {0x3C, /* Size in bytes */ 0x03,
                              'R',  0,
                              'E',  0,
                              '_',  0,
                              'S',  0,
                              'W',  0,
                              'C',  0,
                              ' ',  0,
                              'b',  0,
                              'y',  0,
                              ' ',  0,
                              'P',  0,
                              'o',  0,
                              'u',  0,
                              'n',  0,
                              'a',  0,
                              'm',  0,
                              'u',  0,
                              ' ',  0,
                              'E',  0,
                              'l',  0,
                              'e',  0,
                              'c',  0,
                              't',  0,
                              'r',  0,
                              'o',  0,
                              'n',  0,
                              'i',  0,
                              'c',  0,
                              's',  0};

Descriptor_Configs_t descriptor_config = {
    .device_descriptor                 = MyDevDescr,
    .config_descriptor                 = MyCfgDescr,
    .language_descriptor               = MyLangDescr,
    .manufacturer_descriptor           = MyManuInfo,
    .prod_info_descriptor              = MyProdInfo,
    .consumer_report_descriptor        = ConsumerRepDesc,
    .consumer_report_descriptor_length = sizeof(ConsumerRepDesc),
};

void USB_HID_SWC::init_usb_hid_swc(Output_Mapping_t *mapping) {
  this->output_mapping = mapping;
  this->update_consumer_report_descriptor(this->output_mapping);
  /* Usb Init */
  USBFS_RCC_Init();
  USBFS_Device_Init(ENABLE, &descriptor_config, USB_DEVICE_MODE_HID);
  USB_Sleep_Wakeup_CFG();
}

void USB_HID_SWC::update_consumer_report_descriptor(Output_Mapping_t *mapping) {

  if (mapping) {

    Output_Command_t output_commands[] = {
        mapping->cw_rotate_output, mapping->ccw_rotate_output,
        mapping->button_short_press_output, mapping->button_long_press_output,
        mapping->button_double_press_output};

    uint8_t output_mapping_values[sizeof(output_commands)];

    for (size_t i = 0; i < sizeof(output_commands); i++) {
      uint8_t value = 0;
      switch (output_commands[i]) {
      case VOLUME_UP:
        value = 0xE9;
        break;
      case VOLUME_DOWN:
        value = 0xEA;
        break;
      case MUTE:
        value = 0xE2;
        break;
      case NEXT_TRACK:
        value = 0xB5;
        break;
      case PREVIOUS_TRACK:
        value = 0xB6;
        break;
      case PLAY_PAUSE:
        value = 0xCD;
        break;
      default:
        break;
      }
      output_mapping_values[i] = value;
    }

    const uint8_t descriptor[] = {
        0x05, 0x0C, /* Usage Page (Consumer) */
        0x09, 0x01, /* Usage (Consumer Control) */
        0xA1, 0x01, /* Collection (Application) */
        0x85, 0x01, /*   Report ID (1) */

        0x15, 0x00, /*   Logical Min (0) */
        0x25, 0x01, /*   Logical Max (1) */
        0x75, 0x01, /*   Report Size (1) using bitmask */
        0x95, 0x05, /*   Report Count (7) */

        /* Media control function bitmap */
        0x09, output_mapping_values[0], /*   CW */
        0x09, output_mapping_values[1], /*   CCW */
        0x09, output_mapping_values[2], /*   Short Press */
        0x09, output_mapping_values[3], /*   Long Press */
        0x09, output_mapping_values[4], /*   Double Press */

        0x81, 0x02, /*   Input (Data,Var,Abs) */

        /* Padding */
        0x75, 0x01, 0x95, 0x01, 0x81, 0x03, /*   Input (Const,Var,Abs) */

        0xC0 /* End Collection */
    };
    memcpy(ConsumerRepDesc, descriptor, sizeof(descriptor));
    descriptor_config.consumer_report_descriptor_length = sizeof(descriptor);
  }

  else {
    descriptor_config.consumer_report_descriptor_length = 0;
  }
}

void USB_HID_SWC::on_encoder_rotation(bool cw_rotation) {
  this->usb_hid_output_swc((cw_rotation) ? VOLUME_KNOB_INPUT_CW_ROTATION
                                         : VOLUME_KNOB_INPUT_CCW_ROTATION);
}

void USB_HID_SWC::on_button_short_press() {
  this->usb_hid_output_swc(VOLUME_KNOB_INPUT_BUTTON_SHORT_PRESS);
}

void USB_HID_SWC::on_button_held() {
  this->usb_hid_output_swc(VOLUME_KNOB_INPUT_BUTTON_LONG_PRESS);
}

void USB_HID_SWC::on_button_double_press() {
  this->usb_hid_output_swc(VOLUME_KNOB_INPUT_BUTTON_DOUBLE_PRESS);
}

void USB_HID_SWC::usb_hid_output_swc(Volume_Knob_Input_t input) {

  uint8_t consumer_report_mask = USB_UNKNOWN_COMMAND;

  switch (input) {
  case VOLUME_KNOB_INPUT_CW_ROTATION:
    consumer_report_mask = CONSUMER_REPORT_MASK_CW_ROTATE;
    break;

  case VOLUME_KNOB_INPUT_CCW_ROTATION:
    consumer_report_mask = CONSUMER_REPORT_MASK_CCW_ROTATE;
    break;

  case VOLUME_KNOB_INPUT_BUTTON_SHORT_PRESS:
    consumer_report_mask = CONSUMER_REPORT_MASK_BUTTON_SHORT_PRESS;
    break;

  case VOLUME_KNOB_INPUT_BUTTON_LONG_PRESS:
    consumer_report_mask = CONSUMER_REPORT_MASK_BUTTON_LONG_PRESS;
    break;

  case VOLUME_KNOB_INPUT_BUTTON_DOUBLE_PRESS:
    consumer_report_mask = CONSUMER_REPORT_MASK_BUTTON_DOUBLE_PRESS;
    break;

  default:
    consumer_report_mask = USB_UNKNOWN_COMMAND;
    break;
  }

  if (consumer_report_mask != USB_UNKNOWN_COMMAND && USBFS_DevEnumStatus) {
    uint8_t report[2] = {0x01 /* Report ID = 1 */, consumer_report_mask};
    if (USBFS_DevEnumStatus) {
      USBFS_Endp_DataUp(DEF_UEP1, report, sizeof(report), DEF_UEP_CPY_LOAD);
      int count = 0; /* Wait for max 100ms till we change the report */
      while (is_endpoint_tx_pending(DEF_UEP1) && count < 100) {
        count += 1;
        delay(1);
      }
      report[1] = 0x00; // Reset key press list to nothing
      count     = 0;
      USBFS_Endp_DataUp(DEF_UEP1, report, sizeof(report), DEF_UEP_CPY_LOAD);
      while (is_endpoint_tx_pending(DEF_UEP1) && count < 100) {
        count += 1;
        delay(1);
      }
    }
  }
}
