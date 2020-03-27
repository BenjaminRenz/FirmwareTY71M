/** \file
 * \brief user customizable code to handle HID events
 */
#include "ledCodes.h"

#ifndef USB_EPHANDLER_H_INCLUDED
#define USB_EPHANDLER_H_INCLUDED

#define USB_NUM_OF_DEFINED_HID 1
#define USB_NUM_OF_DEFINED_REPORTS 1
uint32_t USB_HID_PROTOCOL[USB_NUM_OF_DEFINED_HID]={1}; //0 for boot protocol,1 for report protocol, hid standard forces initialized value to be report protocol
uint8_t USB_HID_IDLE_RATE[USB_NUM_OF_DEFINED_REPORTS]={125}; //unit is 4ms, default for keyboard is 500ms, need fixing see TODOs, no support for different report id's just yet

void hid_report_id1_leds(uint32_t epnum,uint8_t* data, uint32_t packetlength);
void (*USB_SET_REPORT_CALBF_FOR_REP_ID[])(uint32_t epnum,uint8_t* data, uint32_t packetlength) ={
    &hid_report_id1_leds
};

uint8_t USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength, void (*callbFuncP)(uint32_t epnum,uint8_t* data, uint32_t packetlength));

/** \brief is beeing called after the device has been configured
 * resets data bits to data0 for all endpoints and starts timer to start reporting key data
 */
void USB_configured_setup_ep(){
    for(uint32_t epnum=0;epnum<(sizeof(EP_CONFIG_ARRAY)/sizeof(EP_CONFIG_ARRAY[0]));epnum++){
        USB_EP_TO_DATAn(epnum,0);
    }
    USB_stop_transOrRec(2);
    timer1_start();
}

void USB_HID_get_report(){

}

/** \brief callback when the host has sent a report for the LED satus over the CTRL_OUT endpoint
 *
 */
void hid_report_id1_leds(uint32_t epnum,uint8_t* data, uint32_t packetlength){
    /*if(data[0]&LED_NUMLOCK_MASK){
        keys[][].blue=255
    }else{
        keys[][].blue=0;
    }*/
    if(data[0]&LED_CAPSLOCK_MASK){
        keys[0][3].blue=255;    //turn on shift key
    }else{
        keys[0][3].blue=0;
    }
}

void USB_endpoint_finished_last_send(uint32_t epnum){   //to provide endpoint with new data after transfer has finished
    //initiate_send checks for full buffer automatically, find out if we even need to do something here
}

#endif // USB_EPHANDLER_H_INCLUDED
