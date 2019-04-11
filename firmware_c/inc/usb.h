#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED

//Sources
/*
https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/usb.chm/usb6.htm
*/
#define USB_BA 0x40060000
#define USB_SRAM_SETUP (*((uint32_t*)(USB_BA+0x100)))
#define USB_SRAM_EP0 (*((uint32_t*)(USB_BA+0x108)))
#define USB_SRAM_EP0_MAXPLD 0x40
#define USB_SRAM_EP1 (*((uint32_t*)(USB_BA+0x148)))
#define USB_SRAM_EP1_MAXPLD 0x40

#define USB_INTEN (*((uint32_t*)(USB_BA+0x000)))
#define USB_INTSTS (*((uint32_t*)(USB_BA+0x004)))
#define USB_ATTR (*((uint32_t*)(USB_BA+0x010)))
#define USB_FLDET (*((uint32_t*)(USB_BA+0x014)))

#define USB_BUFFSEG (*((uint32_t*)(USB_BA+0x018))) //WARNING: Upper half conflicting with USB_BUFFSEG0

//Control in and out on address 0, handled by EP0 as IN and EP1 as OUT
#define USB_BUFFSEG0 (*((uint32_t*)(USB_BA+0x020)))
#define USB_MXPLD0 (*((uint32_t*)(USB_BA+0x024)))
#define USB_CFG0 (*((uint32_t*)(USB_BA+0x028)))
#define USB_CFGP0 (*((uint32_t*)(USB_BA+0x02C)))

#define USB_BUFFSEG1 (*((uint32_t*)(USB_BA+0x030)))
#define USB_MXPLD1 (*((uint32_t*)(USB_BA+0x034)))
#define USB_CFG1 (*((uint32_t*)(USB_BA+0x038)))
#define USB_CFGP1 (*((uint32_t*)(USB_BA+0x03C)))
//in address 1, handeled by EP2 as IN in interrupt mode, for keyboard
#define USB_BUFFSEG2 (*((uint32_t*)(USB_BA+0x040)))
#define USB_MXPLD2 (*((uint32_t*)(USB_BA+0x044)))
#define USB_CFG2 (*((uint32_t*)(USB_BA+0x048)))
#define USB_CFGP2 (*((uint32_t*)(USB_BA+0x04C)))

#define USB_BUFFSEG3 (*((uint32_t*)(USB_BA+0x050)))
#define USB_MXPLD3 (*((uint32_t*)(USB_BA+0x054)))
#define USB_CFG3 (*((uint32_t*)(USB_BA+0x058)))
#define USB_CFGP3 (*((uint32_t*)(USB_BA+0x05C)))


#define USB_is_attached_to_host() (USB_FLDET&0x00000001)

enum {USB_setup_ep_as_in=1,USB_setup_ep_as_out=0};
static __inline void USB_setup_ep(int ep,int in_or_out,int is_isochronous, int ep_addr){ //ep 0-7, in_or_out enum, is_isochronous bit, ep_addr 0-15
    uint32_t* cfgRegist=(uint32_t*)(USB_BA+0x508+(ep<<4));
    uint32_t oldCFGval=(*cfgRegist)&0x00000380;
    *((uint32_t*)(USB_BA+0x508+(ep<<4)))=oldCFGval+(0x20<<in_or_out)+(is_isochronous<<0x10)+ep_addr;
}

static __inline void USB_clear_setup_stall(int ep){
    *((uint32_t*)(USB_BA+0x508+(ep<<4)))|=0x00000200; //Clear Stall by setting this bit
}

static __inline void USB_set_stall(int ep){ //if error indicate that to the host

}

static __inline void USB_start_reset(){
    IPRSTC2|=(1<<27);
}
static __inline void USB_end_reset(){ //only call this after ready with irq setup?
    IPRSTC2&=~(1<<27); //only call this after buffer segmentation is complete (BUFSEG0)
}
static __inline void USB_init(){
    USB_start_reset();
    USB_INTEN=0x00000000;
    //Segment the Endpoint Buffers and setup the endpoints
    USB_BUFFSEG0=0x00000008; //Offset because of 8 setup bytes space
    USB_setup_ep(0,USB_setup_ep_as_in,0,0);
    USB_BUFFSEG1=0x00000048;
    USB_setup_ep(1,USB_setup_ep_as_out,0,0);
    USB_BUFFSEG2=0x00000088;
    USB_setup_ep(2,USB_setup_ep_as_in,0,1); //keyboard from host seen as in
    USB_BUFFSEG3=0x000000C8;
    USB_setup_ep(3,USB_setup_ep_as_in,0,2); //mouse emulation?

    USB_end_reset();
    //enable USB and BUS interrupt events
    USB_INTEN=0x00000002;
}

enum {  USB_INT_EP7_MASK=(1<<23),USB_INT_EP6_MASK=(1<<22),
        USB_INT_EP5_MASK=(1<<21),USB_INT_EP4_MASK=(1<<20),
        USB_INT_EP3_MASK=(1<<19),USB_INT_EP2_MASK=(1<<18),
        USB_INT_EP1_MASK=(1<<17),USB_INT_EP0_MASK=(1<<16),

        USB_INT_SETUP_MASK=(1<<31), //capture host requesting device info via descriptor packets
        USB_INT_FLDET_MASK=(1<<2),  //attach/detach event
        USB_INT_USB_MASK=(1<<1),    //
        USB_INT_BUS_MASK=(1<<0)     //
};

enum {  USB_ATTR_TIMEOUT_MASK=(1<<3),USB_ATTR_RESUME_MASK=(1<<2),
        USB_ATTR_SUSPEND_MASK=(1<<1),USB_ATTR_USBRST_MASK=(1<<0)
};

enum {  USB_EPSTS_INACK=0x000,USB_EPSTS_INNAK=0x001, //Possible bus states for endpoints
        USB_EPSTS_OUTDATA0=0x010,USB_EPSTS_SETUPACK=0x011,
        USB_EPSTS_OUTDATA1=0x110,USB_EPSTS_ISOCHREND=0x111,
};
static __inline void USB_stop_transactions(int ep){

}
static __inline void USB_enable_phy(void){
    USB_ATTR|=0x00000010; //physical part enable bit set
}
static __inline void USB_disable_phy(void){
    USB_ATTR&=0xffffffef; //physical part enable bit unset
}
static __inline void USB_enable_host_wakeup(void){ //also called SE1
    USB_ATTR|=0x00000020; //host wakeup enable bit set
}
static __inline void USB_disable_host_wakeup(void){
    USB_ATTR&=0xffffffdf; //host wakeup enable bit unset
}
void USB_sendEndpoint(uint32_t ep, uint8_t* data, uint32_t packetlength){
    if //check if endpoint is busy
    //copy data to EP0 sram
    //write MAXPLD

}

void URBD_IRQHandler(void){ //will be called for all activated USB_INTEN events
    //Read EPSTS (USB_EPSTS[31:8])+EPEVT7~0 (USB_INTSTS[23:16]) to find out state and endpoint
    //(?) read USB_ATTR to aknowledge bus events
    if(USB_INTSTS&USB_INT_SETUP_MASK){//the received packet was a setup packet, return out descriptor info from descriptor.h
        //copy sram to other memory location
        byte* usbBuffer=?;
        if(usbBuffer[0]&0x60==0x00){ //Standard request (defined by USB standard)
            if(usbBuffer[0]&0x03==0x00){ //Device
                if(usbBuffer[1]=0x00){ //GET_STATUS

                }
            }else if(usbBuffer[0]&0x03==0x01){ //Interface

            }else if(usbBuffer[0]&0x03==0x02){ //Endpoint

            }
        }else if(usbBuffer[0]&0x60==0x20){ //Class request (defined by USB class in use eg. HID)

        }else if(usbBuffer[0]&0x60==0x40){ //Vendor Request (defined by USER/VENDOR)

        }

    }else if(USB_INTSTS&USB_INT_BUS_MASK){ //BUS event
        if(USB_ATTR&USB_ATTR_RESUME_MASK){  //Host not responding by ACK packets

        }else if(USB_ATTR&USB_ATTR_SUSPEND_MASK){ //Resume from suspend

        }else if(USB_ATTR&USB_ATTR_TIMEOUT_MASK){ //Cable plugged out/host sleep

        }else if(USB_ATTR&USB_ATTR_USBRST_MASK){ //Reset from host by se0

        }
        //Clear BUS Interrupt
        USB_INTST=USB_INT_BUS_MASK;
    }else if(USB_INTSTS&USB_INT_USB_MASK){ //SETUP/IN/OUT ACK event
    }
    }else if(USB_INTSTS&USB_INT_FLDET_MASK){
        //IGNORE
    }
}

#endif // USB_H_INCLUDED
