#ifndef HID_DESCRIPTOR_H_INCLUDED
#define HID_DESCRIPTOR_H_INCLUDED

const uint8_t HID_Descriptor[]={ //warning! first byte is least significant (important for fields with >1 bytes)
    0x12,                   //bLength           , 1byte, descriptor size in bytes
    0x01,                   //bDescriptorType   , 1byte, descriptor type (=1 for device descriptor)
    //the upper two are common to all usb descriptors
    0x20,0x00,              //bcdUSB            , 2byte, usb spec release number (USB 1.0/1.1/2.0)
    0x00,                   //bDeviceClass      , 1byte, device type or 0x00 to get info from Interface descriptors
    0x00,                   //bDeviceSubClass   , 1byte, subclass type (same as above)
    0x00,                   //bDeviceProtocol   , 1byte, protocol type (same as above)
    USB_DD_EP0_packet_size, //bMaxPacketSize    , 1byte, max Packet size for endpoint0
    0x55, 0x2A,             //idVendor          , 2byte, Assigned id from USB org, use random for hobby project
    0x01, 0x00,             //idProduct         , 2byte, Assigned by manufacturer (me)
    0x01, 0x00,             //bcdDevice         , 2byte, Device Release Number by manufacturer (me)
    0x01,                   //iManufacturer     , 1byte, index of manufacturer string (0x00 if not existing)
    0x02,                   //iProduct          , 1byte, index of productname sting
    0x03,                   //iSerialNumber     , 1byte, index of serialnum string
    USB_DD_NUM_CONFGR       //bNumConfigurations, 1byte, number of configuration Descriptors (e.g. for one bus powered and one self powered device 0x02)
};

const uint8_t REPORT_Descriptor[]={
    0x05, 0x01,

const uint8_t PHYSICAL_Descriptor[]={
#endif // HID_DESCRIPTOR_H_INCLUDED
