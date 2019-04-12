#ifndef UDB_DESCRIPTORS_H_INCLUDED
#define UDB_DESCRIPTORS_H_INCLUDED
//DEVICE DESCRIPTORS
#define USB_DESC_EP0_packet_size 0x08 //Depends on the allocated sram for this endpoint
const uint8_t* USB_Device_Descriptor={ //warning! first byte is least significant (important for fields with >1 bytes)
    0x,                   //bLength           , 1byte, descriptor size in bytes
    0x01,                   //bDescriptorType   , 1byte, descriptor type (=1 for device descriptor)
    //the upper two are common to all usb descriptors
    0x20,0x00,              //bcdUSB            , 2byte, usb spec release number (USB 1.0/1.1/2.0)
    0x00,                   //bDeviceClass      , 1byte, device type or 0x00 to get info from Interface descriptors
    0x00,                   //bDeviceSubClass   , 1byte, subclass type (same as above)
    0x00,                   //bDeviceProtocol   , 1byte, protocol type (same as above)
    USB_DESC_EP0_packet_size,    //bMaxPacketSize    , 1byte, max Packet size for endpoint0
    0x55, 0x2A,             //idVendor          , 2byte, Assigned id from USB org, use random for hobby project
    0x01, 0x00,             //idProduct         , 2byte, Assigned by manufacturer (me)
    0x01, 0x00,             //bcdDevice         , 2byte, Device Release Number by manufacturer (me)
    0x00,                   //iManufacturer     , 1byte, index of manufacturer string (0x00 if not existing)
    0x00,                   //iProduct          , 1byte, index of productname sting
    0x00,                   //iSerialNumber     , 1byte, index of serialnum string
    0x01                    //bNumConfigurations, 1byte, number of configuration Descriptors (e.g. for one bus powered and one self powered device 0x02)
};

//CONGIFURATION DESCRIPTORS
#define USB_DESC_NUM_IFACES 0x02
const uint8_t* USB_Config_Descriptor={
    0x,                   //bLength           , 1byte, descriptor size in bytes
    0x02,                   //bDescriptorType   , 1byte, descriptor type (=2 for configuration descriptor)
    ,, //wTotalLength   , 2byte, length of itself+interface descriptors+endpoints underneath in hirachy
    USB_DESC_NUM_IFACES, //bNumInterfaces , 1byte,
    0x01, //bConfigurationValue, 1byte,
    //iConfiguration, 1byte, index of string describing this configuration
    0xA0, //bmAttributes, 1byte, Bitconfig, D7=1, D6 Self Powered, D5 Remote Wkup, rest=0
    0xFA//bMaxPower, 1byte, max power consumption in 2mA*" (=500mA)
};

//INTERFACE DESCRIPTORS
const uint8_t* USB_INTERFACE_Descriptor={
    0x09,                   //bLength           , 1byte, descriptor size in bytes
    0x04,                   //bDescriptorType   , 1byte, descriptor type (=4 for interface)
    0x01,                   //bInterfaceNumber  , 1byte,
    0x00,                   //bAlternateSetting , 1byte, can be used to switch between interface modes (eg. soundcard frequencys) by defining multiple possible configs for one endpoint
    0x01,                   //bNumEndpoints     , 1byte, number if used endpoints for this interface
    0x03,                   //bInterfaceClass   , 1byte, base class code of interface (eg: 0x03 for HID), defined by USB Org
    0x0                     //bInterfaceSubClass, 1byte, sub class code of interface (eg. )


//ENDPOINT DESCRIPTORS
const uint8_t* USB_ENDPOINT_Descriptor={
#endif // UDB_DESCRIPTORS_H_INCLUDED
