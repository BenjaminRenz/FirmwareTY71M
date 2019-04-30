/*TODO list
    .usb enable phy?
    -handle larger descriptor collection transmissions >0x40 bytes
    -device/endpoint halt feature support
    -call user function to inform on attach/detach
    -check if endpoint is busy
    -alternative interface modes

*/

#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED
#include <stdint.h>
#include "usb_descriptors.h"

//Sources
/*
https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/usb.chm/usb6.htm
https://www.beyondlogic.org/usbnutshell/usb6.shtml#GetDescriptor
http://sdphca.ucsd.edu/lab_equip_manuals/usb_20.pdf
*/
//Settings
#define USB_SRAM_SETUP_SIZE 0x08
#define USB_SRAM_ENDP_SIZE  0x40
//Control to host
#define USB_EP0_IN
#define USB_EP0_ADDR 0
#define USB_CTRL_IN 0 //endpoint number for control in (dev to host)
//Control from host
#define USB_EP1_OUT
#define USB_EP1_ADDR 0
#define USB_CTRL_OUT 1 //endpoint number for control out (host to dev)

//Define custom endpoints other than EP0/1
#define USB_EP2_IN
#define USB_EP2_ADDR 1
//#define USB_EP2_OUT
//#define USB_EP3_IN


//Register and Address definitions
#define USB_BA 0x40060000

#define USB_SRAM_SETUP_START (*((uint32_t*)(USB_BA+0x100)))
#define USB_SRAM_EP_START(epnum) (*((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum)))
#define USB_SRAM_EP_ADDR(epnum) ((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum))
#define USB_MAXPLD(epnum) (*((uint32_t*)(USB_BA+0x504+(epnum<<4))))

#define USB_INTEN (*((uint32_t*)(USB_BA+0x000)))
#define USB_INTSTS (*((uint32_t*)(USB_BA+0x004)))
#define USB_FADDR (*((uint32_t*)(USB_BA+0x008))) //Overlaps with ATTR
#define USB_ATTR (*((uint32_t*)(USB_BA+0x010))) //Overlaps with FADDR
#define USB_FLDET (*((uint32_t*)(USB_BA+0x014)))

#define USB_BUFFSEG_SETUP (*((uint32_t*)(USB_BA+0x018))) //Overlaps with USB_BUFFSEG(0)

#define USB_BUFFSEG(epnum) (*((uint32_t*)(USB_BA+0x500+(epnum<<4))))
#define USB_CFG(epnum) (*((uint32_t*)(USB_BA+0x508+(epnum<<4))))
#define USB_CFGP(epnum) (*((uint32_t*)(USB_BA+0x50C+(epnum<<4))))

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

static __inline void USB_EP_TO_DATAn(uint32_t ep,uint32_t n){
    if(n){ //setDATA1
        USB_CFG(ep)|=0x00000080;
    }else{ //setDATA0
        USB_CFG(ep)&=0xffffff7f;
    }
}

enum {USB_setup_ep_as_in=1,USB_setup_ep_as_out=0};
static __inline void USB_setup_ep(int ep,int in_or_out,int is_isochronous, int ep_addr){ //ep 0-7, in_or_out enum, is_isochronous bit, ep_addr 0-15
    uint32_t oldCFGval=USB_CFG(ep)&0x00000380;
    USB_CFG(ep)=oldCFGval+(0x20<<in_or_out)+(is_isochronous<<0x10)+ep_addr;
}

static __inline void USB_clear_setup_stall(int ep){
    USB_CFG(ep)|=0x00000200; //Clear Stall by setting this bit
}

#define USB_set_ep_stall_bit(ep)    (USB_CFGP(ep)|=0x00000002)
#define USB_clear_ep_stall_bit(ep)  (USB_CFGP(ep)&=0xfffffffd)
#define USB_get_ep_stall_bit(ep)    ((USB_CFGP(ep)&0x00000002)>>1)
static __inline void USB_set_ctrl_stall(){ //if error indicate that to the host
    USB_ep_set_stall_bit(USB_CTRL_IN);
    USB_ep_set_stall_bit(USB_CTRL_OUT);
}

static __inline void USB_stop_transOrRec(int ep){ //clear in/out ready flag after already written USB_MAXPLD
    USB_CFGP(ep)|=0x00000001;
}

static __inline void USB_start_reset(){
    IPRSTC2|=(1<<27); //Send Reset Signal to USB block
}
static __inline void USB_end_reset(){ //only call this after ready with interrupt setup after buffer and after BUFSEG has completed
    IPRSTC2&=~(1<<27); //Stop sending Reset Signal to USB block
}





void USB_init(){
    USB_start_reset();
    USB_INTEN=0x00000000;
    //Segment the Endpoint Buffers and setup the endpoints
    //USB_BUFFSEG for the setup buffer is already set to 0x00000000 on reset indicating no offset for setup storage space
    USB_CONFIGURE_EP_IF_DEFINED(0);
    USB_CONFIGURE_EP_IF_DEFINED(1);
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



/*Interrupt handle functions*/
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
void USB_PrepareToSendEp(uint32_t ep, uint8_t* data, uint32_t packetLength){
    if //TODO check if endpoint is busy
    for(uint32_t i=0;i<packetlength;i++){
        USB_SRAM_EP_ADDR(epnum)[i]=data[i];
    }
    USB_MAXPLD_ADDR(ep)=packetLength;
}

void USB_PrepareToSendEpAfterSetup(uint32_t ep, uint8_t* data, uint32_t packetLength){
    USB_EP_TO_DATAn(USB_CTRL_IN,1); //first data packet after setup stage must be data1
    if //TODO check if endpoint is busy
    for(uint32_t i=0;i<packetlength;i++){
        USB_SRAM_EP_ADDR(epnum)[i]=data[i];
    }
    USB_MAXPLD_ADDR(ep)=packetLength;
}

#define USB_PrepareToRecieveEp(epnum,storedest,packetLength) USB_MAXPLD_ADDR(epnum)=packetLength \
                                                             USB_EP##epnum##_EXPECTED_BYTES=packetLength \
                                                             USB_EP##epnum##_STOREPOINTER=storedest


//USB state
enum {  USBDeviceStateDefault, //Mode after reset
        USBDeviceStateAddress, //Moder after receiving address by host
        USBDeviceStateConfigured, //Moder after SetConfiguration, must reset all states and Data0 as next send
        USBDeviceStateSuspended
};

uint32_t wakeupHostEnabeled=1;


void URBD_IRQHandler(void){ //will be called for all activated USB_INTEN events
    //Read EPSTS (USB_EPSTS[31:8]) find out state and endpoint
    //(?) read USB_ATTR to aknowledge bus events

    static uint32_t deviceState=USBDeviceStateDefault;
    //Static variables whitch hold the expected bytes and destinsation address for next ep recieve call
    static USB_EP1_EXPECTED_BYTES=0;
    static USB_EP1_STOREPOINTER=NULL;

    if(USB_INTSTS&USB_INT_USB_MASK){ //Got some usb packets
        //Clear USB interrupt
        USB_INTST=USB_INT_USB_MASK;
        if(USB_INTSTS&USB_INT_SETUP_MASK){//the received packet was a setup packet, process it
            uint8_t* usbSRAM=USB_SRAM_SETUP; //TODO remove: get pointer to setup memory location
            //switch request type
            if(usbSRAM[0]&0x60==0x00){                                              //Standard request (defined by USB standard)
                /*FromHostToDevice  Request Type                DataLength
                  0                 0x00 (get status)           2
                  1                 0x01 (clear feature)        0
                  1                 0x03 (set feature)          0
                  1                 0x05 (set address)          0
                  0                 0x06 (get descriptor)       Descriptor length
                  1                 0x07 (set descriptor)       Descriptor length
                  0                 0x08 (get configuration)    1
                  0                 0x09 (set configuration)    0
                */
                if(usbSRAM[0]&0x1f==0x00){                                              //Standard Device Request
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){                     //device should send status to host
                        //Type Control Read (Setup->DataIn->StatusOut)
                        uint8_t selfPowered=((USB_CONFIG_Descriptor[8]&0x40)>>6);
                        uint8_t data[2]={(USB_D_REMOVE_WAKEUP<<1)+selfPowered,0x00};
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                        //Data Stage:
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));

                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){                     //host wants to clear a device feature
                        //Type Control Write (Setup->DataOut->StatusIn)
                        if((deviceState==USBDeviceStateAddress||deviceState==USBDeviceStateConfigured)
                        if(usbSTAM[2]==0x01){//Clear Endpoint Halt feature

                        }else if(usbSRAM[2]=0x00){//Clear Device can wakeup host
                            wakeupHostEnabeled=0;
                        }
                        //Status Stage
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,0,0); //send zero length packet to confirm reception

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){                     //host wants to set a device feature
                        if(usbSTAM[2]==0x01){//set Endpoint Halt feature

                        }else if(usbSRAM[2]=0x00){//set Device can wakeup host
                            wakeupHostEnabeled=1;
                        }

                        //Status Stage:
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,0,0); //send zero length packet to confirm reception

                    }else if((usbSRAM[1]==0x05)&&(!(usbSRAM[0]&0x80))){                     //host wants to set the devices address
                        uint32_t address=0;
                        address=usbSRAM[2]+usbSRAM[3]<<8;
                        USB_FADDR=0x000007f&address;
                        deviceState=USBDeviceStateAddress;
                        //Status stage:
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,0,0); //send zero length packet to confirm reception

                    }else if((usbSRAM[1]==0x06)&&( (usbSRAM[0]&0x80))){                     //device should return a descriptor (string,configuration,device, (NOT endpoint or Interface))
                        //Request is valid in all states of deviceState
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_PrepareToRecieveEp(USB_CTRL_OUT,0,NULL); //Get ready to get "zero byte" confirmation by host after Data stage
                        if(usbSRAM[3]==0x01){//type of decriptor to return (device descriptor)
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(1,USB_DEVICE_Descriptor,USB_DEVICE_Descriptor[0]);
                        }else if(usbSRAM[3]==0x02){ //type of decriptor to return (configuration descriptor)
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(1,USB_CONFIG_Descriptor,USB_CONFIG_Descriptor[0]);
                        }else if(usbSRAM[3]==0x03){ //type of decriptor to return (string descriptor)ion by host after Data stage
                            //Data Stage:
                            //Select the string descriptor based on the index (warning index is 0 indexed)
                            USB_PrepareToSendEpAfterSetup(1,USB_STRING_DESCRIPTOR_ARRAY[usbSRAM[2]],USB_STRING_DESCRIPTOR_ARRAY[usbSRAM[2]][0]);
                        }else{
                            //error: All other descriptor types are not allowed to accessed
                            USBsetStall();
                        }

                    }else if((usbSRAM[1]==0x07)&&(!(usbSRAM[0]&0x80))){                     //host wants to set a device descriptor

                    }else if((usbSRAM[1]==0x08)&&( (usbSRAM[0]&0x80))){                     //device should return which configuration is used (bConfigurationValue)
                        if(deviceState==USBDeviceStateAddress){
                            uint8_t data[1]={0x00};
                            //prepare Status Stage
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0);
                            //Data Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data)); //Needs to return zero value if not configured
                        }else if(deviceState==USBDeviceStateConfigured){
                            uint8_t data[1]={USB_CONFIG_Descriptor[5]};
                            //prepare Status Stage
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0);
                            //Data Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data)); //Needs to return configdesc value

                        }else{
                            USBsetStall();//error: device was not configured (undefined behavior)
                        }
                    }else if((usbSRAM[1]==0x09)&&(!(usbSRAM[0]&0x80))){                     //host wants to set a device to one of its configurations (bConfigurationValue)
                        if(deviceState==USBDeviceStateAddress){
                            if(usbSRAM[2]==USB_CONFIG_Descriptor[5]){
                                deviceState=USBDeviceStateConfigured; //Host selected one of our device configuration
                            }else if(usbSRAM[2]==0x00){
                                //Remain in USBDeviceStateAddress
                            }else{
                                USBsetStall(); //Invalid configuration selected
                            }
                        }else if(deviceState==USBDeviceStateAddress){
                            if(usbSRAM[2]==USB_CONFIG_Descriptor[5]){
                                //Remain/Switch to selected configuration (do nothing if only one exists)
                            }else if(usbSRAM[2]==0x00){
                                deviceState=USBDeviceStateAddress;//Return to in USBDevice address state
                            }else{
                                USBsetStall(); //Invalid configuration selected
                            }
                        }else{
                            USBsetStall(); //Undefined behavior in default mode
                        }
                    }else{//Undefined standard device request
                        USBsetStall();//ShouldNotHappen
                    }
                }else if(usbSRAM[0]&0x1f==0x01){ //Standard Interface Request
                     /*FromHostToDevice  Request Type                DataLength
                       0                 0x00 (get status)           2
                       1                 0x01 (clear feature)        0
                       1                 0x03 (set feature)          0
                       0                 0x0A (get interface)        1          (not used)
                       1                 0x11 (set interface)        0          (not used)
                    */
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //interface should send status to host
                        uint8_t data[2]={0x00,0x00};
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                        //Data Stage:
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));

                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a interface feature
                        //there are no supported features specified in the USB standard
                        //Status Stage, confirm
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a interface feature
                        //there are no supported features specified in the USB standard
                        //Status Stage, confirm
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);

                    }else if((usbSRAM[1]==0x0A)&&( (usbSRAM[0]&0x80))){ //device should return alternative setting???
                        //Let's hope that that's not happening, we
                        //TODO implement and switch interface based on usbSRAM[4]

                    }else if((usbSRAM[1]==0x11)&&(!(usbSRAM[0]&0x80))){ //host wants to set device interface to alternative setting
                        //no alternative interface given to host in description so should not happen
                        //TODO implement and switch interface based on usbSRAM[4]

                        USB_set_stall(); //Error not supported

                    }else{
                        USB_set_stall(); //Error not supported

                    }
                }else if(usbSRAM[0]&0x1f==0x02){ //Standard Endpoint
                    /*FromHostToDevice  Request Type                DataLength
                      0                 0x00 (get status)           2
                      1                 0x01 (clear feature)        0
                      1                 0x03 (set feature)          0
                    */

                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //endpoint should send status to host
                        if(usbSRAM[4]==0x10){//CRTL_IN
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_ep_get_stall_bit(USB_CTRL_IN)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));

                        }else if(usbSRAM[4]==0x00){//CTRL_OUT
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_ep_get_stall_bit(USB_CTRL_OUT)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));

                        }else if(usbSRAM[4]==0x11){//KEYBOARD IN
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_ep_get_stall_bit(2)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));

                        }else if(usbSRAM[4]==0x12){//MOUSE IN

                        }else{
                            USB_set_stall();
                        }
                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a endpoint feature
                        if(usbSRAM[4]==0x10){ //CTRL_IN
                            USB_clear_ep_stall_bit(CTRL_IN);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else if(usbSRAM[4]==0x00){ //CTRL_OUT
                            USB_clear_ep_stall_bit(CTRL_OUT);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else if(usbSRAM[4]==0x11){//KEYBOARD IN
                            USB_clear_ep_stall_bit(2);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else if(usbSRAM[4]==0x12){//KEYBOARD IN
                            USB_clear_ep_stall_bit(3);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else{
                            USB_set_ctrl_stall();
                        }

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a endpoint feature
                        if(usbSRAM[4]==0x10){ //CTRL_IN
                            USB_set_ep_stall_bit(CTRL_IN);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else if(usbSRAM[4]==0x00){ //CTRL_OUT
                            USB_set_ep_stall_bit(CTRL_OUT);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else if(usbSRAM[4]==0x11){//KEYBOARD IN
                            USB_set_ep_stall_bit(2);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else if(usbSRAM[4]==0x12){//KEYBOARD IN
                            USB_set_ep_stall_bit(3);
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }else{
                            USB_set_ctrl_stall();
                        }
                    }else{
                        USB_set_stall(); //Error not supported

                    }
                }else{
                    USB_set_stall(); //Error not supported

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
            USB_enable_phy();
        }else if(USB_ATTR&USB_ATTR_SUSPEND_MASK){ //Cable plugged out/host is sleeping (bus idle for <3mS)
            USB_disable_phy();
        }else if(USB_ATTR&USB_ATTR_TIMEOUT_MASK){ //Host not responding by ACK packets

        }else if(USB_ATTR&USB_ATTR_USBRST_MASK){ //Reset from host by se0
            USB_enable_controller();
            USB_enable_phy();
            //TODO reset controller

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
