#ifndef HID_DESCRIPTOR_H_INCLUDED
#define HID_DESCRIPTOR_H_INCLUDED

/*
https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
*/



const uint8_t REPORT_Descriptor_Keyboard[]={
    0x05, 0x01,             //USAGE_PAGE (Generic Desktop
    0x09, 0x06,             //USAGE (Keyboard)
    0xa1, 0x01,             //COLLECTION BEGIN (Application)
    0x05, 0x07,               //USAGE(Keyboard)
    0x19, 0xe0,               //USAGE_MINIMUM(first modifier Key - LCTRL)
    0x29, 0xe7,               //USAGE_MAXIMUM(last modifier Key - RGUI)
    0x15, 0x00,               //LOGICAL_MINIMUM(0x00)
    0x25, 0x01,               //LOGICAL_MINIMUM(0x01) (button can be pressed ore released)
    0x95, 0x08,               //REPORT_COUNT(8)
    0x75, 0x01,               //REPORT_SIZE(1)
    0x81, 0x02,               //INPUT(Data,Variable,Absolute)

    0x05, 0x07,               //USAGE(Keyboard)
    0x19, 0x00,               //USAGE_MINIMUM(0)
    0x29, 0x68,               //USAGE_MAXIMUM(104)
    0x95, 0x01,               //REPORT_COUNT(6) //6 keys because no more are supported by bios
    0x75, 0x05,               //REPORT_SIZE(8)
    0x81, 0x01,               //INPUT(Constant) //END PADDING

    0xC0                   //COLLECTION END
};

const uint8_t REPORT_Descriptor_Mouse[]={
    0x05, 0x01,             //USAGE_PAGE (Generic Desktop)
    0x09, 0x02,             //USAGE (Mouse)
    0xa1, 0x01,             //COLLECTION BEGIN (Application)
    0x09, 0x01,               //USAGE (Pointer)
    0xa1, 0x00,               //Collection(Physical)
    0x05, 0x09,                 //USAGE(Button)
    0x19, 0x01,                 //USAGE_MINIMUM(Button 1)
    0x29, 0x03,                 //USAGE_MAXIMUM(Button 3) (for 3 button mouse)
    0x15, 0x00,                 //LOGICAL_MINIMUM(0x00)
    0x25, 0x01,                 //LOGICAL_MINIMUM(0x01) (button can be pressed ore released)
    0x95, 0x03,                 //REPORT_COUNT(3)
    0x75, 0x01,                 //REPORT_SIZE(1bit)
    0x81, 0x02,                 //INPUT(Data,Variable,Absolute)
    0x95, 0x01,                 //REPORT_COUNT(1) //START PADDING (to fill 8 bits)
    0x75, 0x05,                 //REPORT_SIZE(5)
    0x81, 0x01,                 //INPUT(Constant) //END PADDING

    0x05, 0x01,                 //USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                 //USAGE(X)
    0x09, 0x31,                 //USAGE(Y)
    0x09, 0x38,                 //USAGE(Wheel)
    0x15, 0x81,                 //LOGICAL_MINIMUM(0x81=-127)
    0x25, 0x7F,                 //LOGICAL_MAXIMUM(0x7F=127)
    0x95, 0x03,                 //REPORT_COUNT(3)
    0x75, 0x08,                 //REPORT_SIZE(8bit)
    0x81, 0x06,                 //INPUT(Data,Variable,Relative)

    0xc0, 0xc0              //COLLECTION END, COLLECTION END
};

const uint8_t* USB_HID_REPORT_DESCRIPTOR_ARRAY[]={
    REPORT_Descriptor_Keyboard,
    REPORT_Descriptor_Mouse
};

const uint8_t PHYSICAL_Descriptor[]={
    0
};
#endif // HID_DESCRIPTOR_H_INCLUDED
