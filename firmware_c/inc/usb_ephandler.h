#ifndef USB_EPHANDLER_H_INCLUDED
#define USB_EPHANDLER_H_INCLUDED

#define USB_NUM_OF_DEFINED_HID 1
#define USB_NUM_OF_DEFINED_REPORTS 1
uint32_t USB_HID_PROTOCOL[USB_NUM_OF_DEFINED_HID]={1}; //0 for boot protocol,1 for report protocol, hid standard forces initialized value to be report protocol
uint8_t USB_HID_IDLE_RATE[USB_NUM_OF_DEFINED_REPORTS]={125}; //unit is 4ms, default for keyboard is 500ms, need fixing see TODOs, no support for different report id's just yet

uint8_t USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength);
void USB_configured_setup_ep(){//is being called after device has been configured
    timer1_start(); //Start timer for reporting keydata
}

void USB_endpoint_finished_last_send(uint32_t epnum){   //to provide endpoint with new data after transfer has finished
    //initiate_send checks for full buffer automatically, find out if we even need to do something here
    switch(epnum){
        default:
            break;
    }
}

#endif // USB_EPHANDLER_H_INCLUDED
