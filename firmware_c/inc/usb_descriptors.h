#ifndef UDB_DESCRIPTORS_H_INCLUDED
#define UDB_DESCRIPTORS_H_INCLUDED
/*Reference:
https://www.beyondlogic.org/usbnutshell/usb5.shtml#InterfaceDescriptors
*/

//General Settings
int32_t USB_D_SELF_POWERED=0;         //(zero or one) if the device is powered by usb (0) or self powered (1)
int32_t USB_D_REMOVE_WAKEUP=1;        //device can wakeup the sleeping host

#define USB_DD_EP0_packet_size  0x08 //allocated sram for setup packets
#define USB_CD_POWER            0xFA //Power consumption of the device in x*2mA (=500mA)
#define USB_CD_NUM_IFACES       0x02 //how many interface should the device report to the pc (one mouse+one keyboard=2)
#define USB_EPD_pollingtime     0x0A //poll interval in x*1ms (10ms)=100Hz polling rate

//DEVICE DESCRIPTORS
const uint8_t USB_DEVICE_Descriptor[]={ //warning! first byte is least significant (important for fields with >1 bytes)
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
    0x00,                   //iManufacturer     , 1byte, index of manufacturer string (0x00 if not existing)
    0x00,                   //iProduct          , 1byte, index of productname sting
    0x00,                   //iSerialNumber     , 1byte, index of serialnum string
    0x01                    //bNumConfigurations, 1byte, number of configuration Descriptors (e.g. for one bus powered and one self powered device 0x02)
};

//CONGIFURATION DESCRIPTORS
const uint8_t USB_CONFIG_Descriptor[]={
    0x??,                   //bLength           , 1byte, descriptor size in bytes
    0x02,                   //bDescriptorType   , 1byte, descriptor type (=2 for configuration descriptor)
    0x??,0x??,              //wTotalLength      , 2byte, length of itself+interface descriptors+endpoints underneath in hirachy
    USB_CD_NUM_IFACES,      //bNumInterfaces    , 1byte,
    0x01,                   //bConfigurationValue, 1byte,
                            //iConfiguration    , 1byte, index of string describing this configuration
    0xA0,                   //bmAttributes      , 1byte, Bitconfig, D7=1, D6 Self Powered, D5 Remote Wkup, rest=0
    USB_CD_POWER            //bMaxPower         , 1byte, max power consumption in x*2mA
};

//INTERFACE DESCRIPTORS
const uint8_t USB_INTERFACE_Descriptor[]={
    0x09,                   //bLength           , 1byte, descriptor size in bytes
    0x04,                   //bDescriptorType   , 1byte, descriptor type (=0x04 for interface)
    0x00,                   //bInterfaceNumber  , 1byte, interface number can either be specified or if 0x00 will be determined/incremented automatically
    0x00,                   //bAlternateSetting , 1byte, can be used to switch between interface modes (eg. soundcard frequencys) by defining multiple possible configs for one endpoint
    0x01,                   //bNumEndpoints     , 1byte, number if used endpoints for this interface
    0x03,                   //bInterfaceClass   , 1byte, base class code of interface (eg: 0x03 for HID), defined by USB Org
    0x01,                   //bInterfaceSubClass, 1byte, sub class code of interface (eg. HID: 0x00 for none and 0x01 for boot interface)
    0x01,                   //bInterfaceProtocol, 1byte, protocol code (e.g.: 0x00 none, 0x01 keyboard, 0x02 Mouse)
    0x00,                   //iInterface        , 1byte, index of string for Descriptor (can be 0x00 to indicate that there is none)
}

//ENDPOINT DESCRIPTORS
const uint8_t USB_ENDPOINT_Descriptor[]={
    0x07,                   //bLength           , 1byte, descriptor size in bytes
    0x05,                   //bDescriptorType   , 1byte, descriptor type (=0x05 for interface)
    0x??,                   //bEndpointAddress  , 1byte, lower nibble (0-3) for endpoint number, bit 7 for out=0 in=1
    0x??,                   //bmAttributes      , 1byte, transfer type bit 0-1 (control=0b00,isoch=0b01,bulk=0b10,inter=0b11) (bit 2-7 only used for isoch)
    0x??,0x??,              //wMaxPacketSize    , 2byte, maximum Size of packets the endpoint can recieve/send
    USB_EPD_pollingtime     //bIntercal         , 1byte, polling time for interrupt in x*1mS (for isoch must be 1)
}

//STRING DESCRIPTORS
const uint8_t USB_STRING_Descriptor_1[]={

}
#endif // UDB_DESCRIPTORS_H_INCLUDED
