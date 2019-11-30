#ifndef USB_EPHANDLER_H_INCLUDED
#define USB_EPHANDLER_H_INCLUDED
uint8_t USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength);
void USB_configured_setup_ep(){//is being called after device has been configured to initialize
    reportPressedKeys(0,keys,report_hid_out);
    //USB_initiate_send(2,report_hid_out,8);
    timer1_start();
}

void USB_endpoint_finished_last_send(uint32_t epnum){   //to provide endpoint with new data after transfer has finished
    switch(epnum){
        default:
            break;
    }
}

#endif // USB_EPHANDLER_H_INCLUDED
