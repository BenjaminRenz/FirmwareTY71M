#ifndef USB_EPHANDLER_H_INCLUDED
#define USB_EPHANDLER_H_INCLUDED
uint8_t USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength);
uint8_t* ep2reportstorage=0;
uint8_t* ep2reportstorage_old=0;
uint8_t* ep3instorage=0;
void USB_configured_setup_ep(){//is being called after device has been configured to initialize
    ep2reportstorage=(uint8_t*)malloc(sizeof(uint8_t)*8);
    ep2reportstorage_old=(uint8_t*)malloc(sizeof(uint8_t)*8);
    getPressedKeys(keys);
    reportPressedKeys(0,keys,ep2reportstorage);
    USB_initiate_send(2,ep2reportstorage,8);
    ep3instorage=(uint8_t*)malloc(sizeof(uint8_t)*USB_SRAM_ENDP_SIZE);
}

void USB_endpoint_finished_last_send(uint32_t epnum){   //to provide endpoint with new data after transfer has finished
    switch(epnum){
        case 2:{     //HID endpoint, need to refill
            //TODO make a timer or something to account for setidle times
            uint8_t old_new_different=0;
            do{
                getPressedKeys(keys);
                for(uint8_t i=0;i<8;i++){   //backup old message
                    ep2reportstorage_old[i]=ep2reportstorage[i];
                }
                reportPressedKeys(0,keys,ep2reportstorage);
                for(uint8_t i=0;i<8;i++){
                    if(ep2reportstorage_old[i]!=ep2reportstorage[i]){
                        old_new_different=1;
                    }
                }
                if(old_new_different){
                    USB_initiate_send(2,ep2reportstorage,8);
                }
            }while(!old_new_different);
            break;
        }
        case 3:     //HID led in?
            break;
        default:
            break;
    }
}

#endif // USB_EPHANDLER_H_INCLUDED
