#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED

#include "usb_descriptors.h"

//Sources
/*
https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/usb.chm/usb6.htm
https://www.beyondlogic.org/usbnutshell/usb6.shtml#GetDescriptor
*/
//Settings
#define USB_SRAM_SETUP_SIZE 0x08
#define USB_SRAM_ENDP_SIZE  0x40
#define USB_EP0_IN
#define USB_EP0_ADDR 0
#define USB_EP1_OUT
#define USB_EP1_ADDR 0
//Define custom endpoints other than EP0/1
#define USB_EP2_IN
#define USB_EP2_ADDR 1
//#define USB_EP2_OUT
#define USB_EP3_IN


//Register and Address definitions
#define USB_BA 0x40060000

#define USB_SRAM_SETUP_START (*((uint32_t*)(USB_BA+0x100)))
#define USB_SRAM_EP_START(epnum) (*((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum)))
#define USB_SRAM_EP_ADDR(epnum) ((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum))
#define USB_MAXPLD(epnum) (*((uint32_t*)(USB_BA+0x504+0x10*epnum)))

#define USB_INTEN (*((uint32_t*)(USB_BA+0x000)))
#define USB_INTSTS (*((uint32_t*)(USB_BA+0x004)))
#define USB_FADDR (*((uint32_t*)(USB_BA+0x008))) //Overlaps with ATTR
#define USB_ATTR (*((uint32_t*)(USB_BA+0x010))) //Overlaps with FADDR
#define USB_FLDET (*((uint32_t*)(USB_BA+0x014)))

#define USB_BUFFSEG_SETUP (*((uint32_t*)(USB_BA+0x018))) //Overlaps with USB_BUFFSEG(0)

#define USB_BUFFSEG(epnum) (*((uint32_t*)(USB_BA+0x020+0x10*epnum)))
#define USB_CFG(epnum) (*((uint32_t*)(USB_BA+0x028+0x10*epnum)))
#define USB_CFGP(epnum) (*((uint32_t*)(USB_BA+0x02C+0x10*epnum)))

#define USB_CONFIGURE_EP_IF_DEFINED(epnum) \
    #if defined USB_EP##epnum##_IN \
    USB_BUFFSEG(epnum)=USB_SRAM_SETUP_SIZE+epnum*USB_SRAM_ENDP_SIZE; \
    USB_setup_ep(epnum,USB_setup_ep_as_in,0,USB_EP##epnum##_ADDR); \
    #endif \
    #if defined USB_EP##epnum##_OUT \
    USB_BUFFSEG(epnum)=USB_SRAM_SETUP_SIZE+3*USB_SRAM_ENDP_SIZE; \
    USB_setup_ep(epnum,USB_setup_ep_as_out,0,USB_EP##epnum##_ADDR); \
    #endif \

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
static __inline void USB_end_reset(){ //only call this after ready with interrupt setup after buffer and after BUFSEG has completed
    IPRSTC2&=~(1<<27);
}
void USB_init(){
    USB_start_reset();
    USB_INTEN=0x00000000;
    //Segment the Endpoint Buffers and setup the endpoints

    //USB_BUFFSEG for the setup buffer is already set to 0x00000000 on reset indicating no offset
    USB_BUFFSEG(0)=USB_SRAM_SETUP_SIZE; //Offset because of 8 setup bytes space
    USB_setup_ep(0,USB_setup_ep_as_in,0,0);
    USB_BUFFSEG(1)=USB_SRAM_SETUP_SIZE+1*USB_SRAM_ENDP_SIZE;
    USB_setup_ep(1,USB_setup_ep_as_out,0,0);
    //Custom Endpoint Configuration
    USB_CONFIGURE_EP_IF_DEFINED(2);
    USB_CONFIGURE_EP_IF_DEFINED(3);
    USB_CONFIGURE_EP_IF_DEFINED(4);
    USB_CONFIGURE_EP_IF_DEFINED(5);

    USB_enable_controller(); //TODO find out when to do this...
    USB_end_reset();
    //enable USB, BUS and FLOATDET interrupt events
    USB_INTEN=0x00000007;
}

#define USB_INT_EP_MASK(#1) (1<<(16+#1)) //individual ep interrupt
#define USB_INT_SETUP_MASK (1<<31) //check if usb_mask as this is an special usb packet
#define USB_INT_FLDET_MASK (1<<2) //attach/detach event
#define USB_INT_USB_MASK (1<<1) //got some usb data
#define USB_INT_BUS_MASK (1<<0) //bus state changed

enum {  USB_ATTR_TIMEOUT_MASK=(1<<3),USB_ATTR_RESUME_MASK=(1<<2),
        USB_ATTR_SUSPEND_MASK=(1<<1),USB_ATTR_USBRST_MASK=(1<<0)
};

enum {  USB_EPSTS_INACK=0x000,USB_EPSTS_INNAK=0x001, //Possible bus states for endpoints
        USB_EPSTS_OUTDATA0=0x010,USB_EPSTS_SETUPACK=0x011,
        USB_EPSTS_OUTDATA1=0x110,USB_EPSTS_ISOCHREND=0x111,
};
static __inline void USB_stop_transactions(int ep){

}
//USB ATTR
static __inline void USB_enable_phy(void){
    USB_ATTR|=0x00000010; //physical part enable bit set
}
static __inline void USB_disable_phy(void){
    USB_ATTR&=0xffffffef; //physical part enable bit unset
}
static __inline void USB_enable_controller(void){
    USB_ATTR|=0x00000080;
}
static __inline void USB_disable_controller(void){
    USB_ATTR&=0xffffff7f;
}

static __inline void USB_enable_host_wakeup(void){ //also called SE1
    USB_ATTR|=0x00000020; //host wakeup enable bit set
}
static __inline void USB_disable_host_wakeup(void){
    USB_ATTR&=0xffffffdf; //host wakeup enable bit unset
}

#define USB_is_attached (USB_FLDET&0x00000001)
void USB_sendEndpoint(uint32_t ep, uint8_t* data, uint32_t packetlength){
    if //check if endpoint is busy
    for(uint32_t i=0;i<packetlength;i++){
        USB_SRAM_EP_ADDR(epnum)[i]=data[i];
    }
    USB_MAXPLD_ADDR(ep)=packetlength;
}

static __inline void USB_setAddress(uint32_t addr){
    USB_FADDR=0x000007f&addr;
}

void URBD_IRQHandler(void){ //will be called for all activated USB_INTEN events
    //Read EPSTS (USB_EPSTS[31:8])+EPEVT7~0 (USB_INTSTS[23:16]) to find out state and endpoint
    //(?) read USB_ATTR to aknowledge bus events


    if(USB_INTSTS&USB_INT_USB_MASK){ //Got some usb packets
        //Clear USB interrupt
        USB_INTST=USB_INT_USB_MASK;

        if(USB_INTSTS&USB_INT_SETUP_MASK){//the received packet was a setup packet, process it
            //copy sram to other memory location
            uint8_t* usbSRAM=USB_SRAM_SETUP;
            //switch request type
            if(usbSRAM[0]&0x60==0x00){            //Standard request (defined by USB standard)
                if(usbSRAM[0]&0x1f==0x00){        //Standard Device Request
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //device should send status to host
                        uint8_t data[2];
                        data[0]=(USB_D_REMOVE_WAKEUP<<1)+USB_D_SELF_POWERED;
                        data[1]=0;
                        USB_sendEndpoint(0,&data,2);
                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a device feature
                        if()
                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a device feature

                    }else if((usbSRAM[1]==0x05)&&(!(usbSRAM[0]&0x80))){ //host wants to set the devices address
                        uint32_t address=0;
                        //TODO get address
                        USB_setAddress(address);
                    }else if((usbSRAM[1]==0x06)&&( (usbSRAM[0]&0x80))){ //device should return device descriptor
                        USB_sendEndpoint(0,USB_DEVICE_Descriptor,USB_DEVICE_Descriptor[0]);
                    }else if((usbSRAM[1]==0x07)&&(!(usbSRAM[0]&0x80))){ //host wants to set a device descriptor

                    }else if((usbSRAM[1]==0x08)&&( (usbSRAM[0]&0x80))){ //device should return configuration descriptor

                    }else if((usbSRAM[1]==0x09)&&(!(usbSRAM[0]&0x80))){ //host wants to set a device configuration descriptor

                    }else{
                        //ShouldNotHappen
                    }
                }else if(usbSRAM[0]&0x1f==0x01){ //Standard Interface Request
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //interface should send status to host

                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a interface feature

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a interface feature

                    }else if((usbSRAM[1]==0x0A)&&( (usbSRAM[0]&0x80))){ //device should return alternative setting???

                    }else if((usbSRAM[1]==0x11)&&(!(usbSRAM[0]&0x80))){ //host wants to set device interface to alternative setting

                    }else{
                        //ShouldNotHappen
                    }
                }else if(usbSRAM[0]&0x1f==0x02){ //Standard Endpoint
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //endpoint should send status to host

                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a endpoint feature

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a endpoint feature

                    }else if((usbSRAM[1]==0x12)&&( (usbSRAM[0]&0x80))){ //device should send an endpoint sync frame

                    }else{
                        //ShouldNotHappen
                    }
                }else{
                    //ShouldNotHappen
                }
            }else if(usbSRAM[0]&0x60==0x20){ //Class request (defined by USB class in use eg. HID)

            }else if(usbSRAM[0]&0x60==0x40){ //Vendor Request (defined by USER/VENDOR)

            }

            //Clear Interrupt
            USB_INTST=USB_INT_SETUP_MASK;
        } //if INT_SETUP_MASK end
        #if defined USB_EP0_IN || define USB_EP0_OUT //check if ep0 is defined
        else if(USB_INTSTS&USB_INT_EP_MASK(0)){

        }
        #endif
        #if defined USB_EP1_IN || define USB_EP1_OUT
        else if(USB_INTSTS&USB_INT_EP_MASK(1)){

        }
        #endif
        #if defined USB_EP2_IN || define USB_EP2_OUT
        else if(USB_INTSTS&USB_INT_EP_MASK(2)){

        }
        #endif
        #if defined USB_EP3_IN || define USB_EP3_OUT
        else if(USB_INTSTS&USB_INT_EP_MASK(3)){

        }
        #endif
        #if defined USB_EP4_IN || define USB_EP4_OUT
        else if(USB_INTSTS&USB_INT_EP_MASK(4)){

        }
        #endif

        //Clear interrupt (EP are autocleared by writing USB_MASK)
        USB_INTST=USB_INT_USB_MASK;
    }else if(USB_INTSTS&USB_INT_BUS_MASK){ //BUS event
        if(USB_ATTR&USB_ATTR_RESUME_MASK){  //Resume from suspend
            USB_enable_controller();
        }else if(USB_ATTR&USB_ATTR_SUSPEND_MASK){ //Cable plugged out/host is sleeping (bus idle for <3mS)

        }else if(USB_ATTR&USB_ATTR_TIMEOUT_MASK){ //Host not responding by ACK packets

        }else if(USB_ATTR&USB_ATTR_USBRST_MASK){ //Reset from host by se0

        }
        //Clear BUS Interrupt
        USB_INTST=USB_INT_BUS_MASK;

    }else if(USB_INTSTS&USB_INT_FLDET_MASK){ //Called when connecting/disconnecting usb cable
        if(USB_is_attached){
            //TODO call user function to inform on attach
            USB_enable_controller();
        }else{
            //TODO call user function to inform on detach
            USB_disable_controller();
        }
        //Clear Attach/Detach Interrupt
        USB_INTSTS=USB_INT_FLDET_MASK;
    }
}

#endif // USB_H_INCLUDED
