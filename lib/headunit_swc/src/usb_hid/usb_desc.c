/********************************** (C) COPYRIGHT
 * ******************************* File Name          : composite_km_desc.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/04/06
 * Description        : All descriptors for the keyboard and mouse composite
 * device.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*******************************************************************************/
/* Header File */
#include "usb_desc.h"

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
    DEF_IC_PRG_VER, // bcdDevice
    0x01,           // iManufacturer
    0x02,           // iProduct
    0x03,           // iSerialNumber
    0x01,           // bNumConfigurations
};

/* Configuration Descriptor Set */
const uint8_t MyCfgDescr[] = {
    /* Configuration Descriptor */
    0x09, // bLength
    0x02, // bDescriptorType
    0x22,
    0x00, // wTotalLength
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

    /* HID Descriptor (Keyboard) */
    0x09, // bLength
    0x21, // bDescriptorType
    0x11,
    0x01, // bcdHID
    0x00, // bCountryCode
    0x01, // bNumDescriptors
    0x22, // bDescriptorType
    0x27,
    0x00, // wDescriptorLength

    /* Endpoint Descriptor (Keyboard) */
    0x07, // bLength
    0x05, // bDescriptorType
    0x81, // bEndpointAddress: IN Endpoint 1
    0x03, // bmAttributes (Interrupt)
    0x02,
    0x00, // wMaxPacketSize
    0x0A, // bInterval: 10mS
};

/* Keyboard Report Descriptor */
const uint8_t ConsumerRepDesc[] = {
    0x05, 0x0C, /* Usage Page (Consumer) */
    0x09, 0x01, /* Usage (Consumer Control) */
    0xA1, 0x01, /* Collection (Application) */
    0x85, 0x01, /*   Report ID (1) */

    0x15, 0x00, /*   Logical Min (0) */
    0x25, 0x01, /*   Logical Max (1) */
    0x75, 0x01, /*   Report Size (1) */
    0x95, 0x07, /*   Report Count (7) */

    /* Media control function bitmap */
    0x09, 0xB5, /*   Scan Next Track */
    0x09, 0xB6, /*   Scan Previous Track */
    0x09, 0xB7, /*   Stop */
    0x09, 0xCD, /*   Play/Pause */
    0x09, 0xE2, /*   Mute */
    0x09, 0xE9, /*   Volume Up */
    0x09, 0xEA, /*   Volume Down */

    0x81, 0x02, /*   Input (Data,Var,Abs) */

    /* Padding */
    0x75, 0x01, 0x95, 0x01, 0x81, 0x03, /*   Input (Const,Var,Abs) */

    0xC0 /* End Collection */
};

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
const uint8_t MyLangDescr[] = {0x04, 0x03, 0x09, 0x04};

/* Manufacturer Descriptor */
const uint8_t MyManuInfo[] = {0x0E, 0x03, 'w', 0,   'c', 0,   'h',
                              0,    '.',  0,   'c', 0,   'n', 0};

/* Product Information */
const uint8_t MyProdInfo[] = {0x12, 0x03, 'C', 0,   'H', 0,   '3', 0,   '2',
                              0,    'x',  0,   '0', 0,   '3', 0,   '5', 0};

/* Serial Number Information */
const uint8_t MySerNumInfo[] = {0x16, 0x03, '0', 0, '1', 0, '2', 0,
                                '3',  0,    '4', 0, '5', 0, '6', 0,
                                '7',  0,    '8', 0, '9', 0};
