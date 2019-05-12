/*TODO list
    -handle larger descriptor collection transmissions >0x40 bytes
    -device/endpoint halt feature support
    -call user function to inform on attach/detach
    -alternative interface modes

*/

#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED
#include <stdint.h>    //needed for uint32_t
#include <stddef.h>    //needed for sizeof("structs")
#include "nvic.h"      //Required to enable USB interrupts
#include "backlight.h" //For testing purposes
//Sources
/*
https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/usb.chm/usb6.htm
https://www.beyondlogic.org/usbnutshell/usb6.shtml#GetDescriptor
http://sdphca.ucsd.edu/lab_equip_manuals/usb_20.pdf
https://proyectosfie.webcindario.com/usb/libro/capitulo11.pdf   //Info on hid
*/
enum{EP_IN=0b10, EP_OUT=0b01, EP_DISABELED=0b00}; //EP_STATE
enum{EP_IS_ISOCHR=1,EP_NOT_ISOCHR=0}; //EP_ISOCH
typedef struct USB_EP_CONFIG{
    uint32_t EP_STATE :2;
    uint32_t EP_ADDR :4;
    uint32_t EP_ISOCH :1;
    uint32_t MultiStage_Bytes_left;
    uint8_t* MultiStage_Storage_ptr;
} USB_EP_CONFIG;

USB_EP_CONFIG EP0_CFG={
    .EP_STATE=EP_IN,
    .EP_ADDR=0,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP1_CFG={
    .EP_STATE=EP_OUT,
    .EP_ADDR=0,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP2_CFG={
    .EP_STATE=EP_IN,
    .EP_ADDR=1,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP3_CFG={
    .EP_STATE=EP_IN,
    .EP_ADDR=2,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP4_CFG={
    .EP_STATE=EP_DISABELED,
};
USB_EP_CONFIG EP5_CFG={
    .EP_STATE=EP_DISABELED,
};
USB_EP_CONFIG EP6_CFG={
    .EP_STATE=EP_DISABELED,
};
USB_EP_CONFIG EP7_CFG={
    .EP_STATE=EP_DISABELED,
};

USB_EP_CONFIG EP_CONFIG_ARRAY[8];
uint8_t USB_CTRL_IN=0;
uint8_t USB_CTRL_OUT=0;
uint8_t USB_NUM_OF_DEFINED_ENDP=0;
//BEGIN USER SETTINGS
#define USB_SRAM_SETUP_SIZE 0x08
#define USB_SRAM_ENDP_SIZE  0x40

//END USER SETTINGS

//Globals
uint32_t wakeupHostEnabeled=1;
uint32_t activeConfiguration=0;

#include "usb_descriptors.h"

//Register and Address definitions
#define USB_BA 0x40060000

#define USB_SRAM_SETUP_START (*((uint32_t*)(USB_BA+0x100)))
#define USB_SRAM_EP_START(epnum) (*((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum)))
#define USB_SRAM_EP_ADDR(epnum) ((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum))
#define USB_MAXPLD(epnum) (*((uint32_t*)(USB_BA+0x504+(epnum<<4))))

#define USB_INTEN (*((uint32_t*)(USB_BA+0x000)))
#define USB_INTSTS (*((uint32_t*)(USB_BA+0x004)))
#define USB_FADDR (*((uint32_t*)(USB_BA+0x008))) //Overlaps with ATTR, be careful when writing high bytes
#define USB_ATTR (*((uint32_t*)(USB_BA+0x010)))
#define USB_FLDET (*((uint32_t*)(USB_BA+0x014)))
#define USB_BUFFSEG_SETUP (*((uint32_t*)(USB_BA+0x018))) //Overlaps with USB_BUFFSEG(0)

#define USB_DRVSE0 (*((uint32_t*)(USB_BA+0x090)))

#define USB_BUFFSEG(epnum) (*((uint32_t*)(USB_BA+0x500+(epnum<<4))))
#define USB_CFG(epnum) (*((uint32_t*)(USB_BA+0x508+(epnum<<4))))
#define USB_CFGP(epnum) (*((uint32_t*)(USB_BA+0x50C+(epnum<<4))))

#define USB_is_attached_to_host() (USB_FLDET&0x00000001)

static __inline void USB_EP_TO_DATAn(uint32_t ep,uint32_t n){
    if(n){ //setDATA1
        USB_CFG(ep)|=0x00000080;
    }else{ //setDATA0
        USB_CFG(ep)&=0xffffff7f;
    }
}

#define USB_clear_setup_stall(epnum)   (USB_CFG(ep))|=0x00000200 //Clear Stall by setting this bit

#define USB_set_ep_stall_bit(epnum)    (USB_CFGP(epnum)|=0x00000002)
#define USB_clear_ep_stall_bit(epnum)  (USB_CFGP(epnum)&=0xfffffffd)
#define USB_get_ep_stall_bit(epnum)    ((USB_CFGP(epnum)&0x00000002)>>1)
static __inline void USB_set_ctrl_stall(){ //if error indicate that to the host
    USB_set_ep_stall_bit(USB_CTRL_OUT);
    USB_set_ep_stall_bit(USB_CTRL_IN);
}

#define USB_stop_transOrRec(epnum) USB_CFGP(epnum)|=0x00000001//clear in/out ready flag after already written USB_MAXPLD
#define USB_start_reset() IPRSTC2|=(1<<27) //Send Reset Signal to USB block

//only call this after ready with interrupt setup after buffer and after BUFSEG has completed
#define USB_end_reset() IPRSTC2&=~(1<<27)//Stop sending Reset Signal to USB block


/*Interrupt handle functions*/
//#define USB_INT_EP_MASK (((1<<USB_NUM_OF_DEFINED_ENDP)-1)<<16) //matches to all defined endpoints
#define USB_INT_EP_MASK(epnum) (1<<(16+epnum)) //individual endpoint interrupt
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

static __inline void USB_set_se0(){
    USB_DRVSE0|=0x00000001;
}

void USB_clear_se0(){
    USB_DRVSE0&=0xfffffffe;
}

void USB_init(){
    USB_start_reset();
    USB_INTEN=0x00000000;
    USB_end_reset();
    USB_ATTR=0x000007D0; //Seems to get ignored
    EP_CONFIG_ARRAY[0]=EP0_CFG;
    EP_CONFIG_ARRAY[1]=EP1_CFG;
    EP_CONFIG_ARRAY[2]=EP2_CFG;
    EP_CONFIG_ARRAY[3]=EP3_CFG;
    EP_CONFIG_ARRAY[4]=EP4_CFG;
    EP_CONFIG_ARRAY[5]=EP5_CFG;
    EP_CONFIG_ARRAY[6]=EP6_CFG;
    EP_CONFIG_ARRAY[7]=EP7_CFG;
    //USB_BUFFSEG for the setup buffer is already set to 0x00000000 on reset indicating no offset for setup storage space
    for(uint32_t epnum;epnum<(sizeof(EP_CONFIG_ARRAY)/sizeof(EP_CONFIG_ARRAY[0]));epnum++){
        //Segment the Endpoint Buffers and setup the endpoints
        USB_EP_CONFIG EP_CFG=EP_CONFIG_ARRAY[epnum];
        USB_BUFFSEG(epnum)=USB_SRAM_SETUP_SIZE+epnum*USB_SRAM_ENDP_SIZE;
        uint32_t oldCFGval=USB_CFG(epnum)&0x00000380;
        USB_CFG(epnum)=oldCFGval+(EP_CFG.EP_STATE<<5)+(EP_CFG.EP_ISOCH<<0x10)+EP_CFG.EP_ADDR;
        //Check which ones are the ctrl endpoints and remember index

        if(EP_CFG.EP_STATE==EP_IN){
            USB_NUM_OF_DEFINED_ENDP++;
            if(EP_CFG.EP_ADDR==0){
                USB_CTRL_IN=epnum;
            }
        }else if(EP_CFG.EP_STATE==EP_OUT){
            USB_NUM_OF_DEFINED_ENDP++;
            if(EP_CFG.EP_ADDR==0){
                USB_CTRL_OUT=epnum;
            }
        }
    }

    /*USB_enable_phy(); //TODO find out when to do this...
    */

    USB_enable_controller(); //TODO find out when to do this...
    //enable USB, BUS and FLOATDET interrupt events
    USB_INTEN=0x00000007;
    USB_set_se0();
}



#define USB_is_attached (USB_FLDET&0x00000001)
void USB_PrepareToSendEp(uint32_t epnum, uint8_t* data, uint32_t packetLength){
    //TODO check if IN endpoint
    if(!EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left){ //check if endpoint is busy
        if(packetLength>USB_SRAM_ENDP_SIZE){ //Do we have to split it into multistage transfers?
            for(uint32_t i=0;i<USB_SRAM_ENDP_SIZE;i++){
                USB_SRAM_EP_ADDR(epnum)[i]=data[i];
            }
            USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
            EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=data+USB_SRAM_ENDP_SIZE;
            EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left=packetLength-USB_SRAM_ENDP_SIZE;
        }else{ //can be handeled in one transfer
            for(uint32_t i=0;i<packetLength;i++){
                USB_SRAM_EP_ADDR(epnum)[i]=data[i];
            }
            USB_MAXPLD(epnum)=packetLength;
        }
    }else{
        //TODO ERROR
    }
}

static __inline void USB_PrepareToSendEpAfterSetup(uint32_t epnum, uint8_t* data, uint32_t packetLength){
    //first data packet after setup stage must be data1
    USB_EP_TO_DATAn(USB_CTRL_IN,1);
    if(!EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left){ //check if endpoint is busy
        if(packetLength>USB_SRAM_ENDP_SIZE){ //Do we have to split it into multistage transfers?
            for(uint32_t i=0;i<USB_SRAM_ENDP_SIZE;i++){
                USB_SRAM_EP_ADDR(epnum)[i]=data[i];
            }
            USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
            EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=data+USB_SRAM_ENDP_SIZE;
            EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left=packetLength-USB_SRAM_ENDP_SIZE;

        }else{ //can be handeled in one transfer
            for(uint32_t i=0;i<packetLength;i++){
                USB_SRAM_EP_ADDR(epnum)[i]=data[i];
            }
            USB_MAXPLD(epnum)=packetLength;
        }
    }else{
        //TODO ERROR
    }
}

static __inline void USB_PrepareToRecieveEp(uint32_t epnum,uint8_t* storedest,uint32_t packetLength){
    if(!EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left){ //Are we already waiting for some bytes on this ep?
        if(packetLength>USB_SRAM_ENDP_SIZE){ //Do we have to split it into multistage transfers?
            USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
            EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=storedest+USB_SRAM_ENDP_SIZE;
            EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left=packetLength-USB_SRAM_ENDP_SIZE;

        }else{ //can be handeled in one transfer
            USB_MAXPLD(epnum)=packetLength;
            EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left=packetLength;
            EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=storedest;
        }
    }else{
        //TODO ERROR
    }
}

//USB state
enum {  USBDeviceStateDefault, //Mode after reset
        USBDeviceStateAddress, //Moder after receiving address by host
        USBDeviceStateConfigured, //Moder after SetConfiguration, must reset all states and Data0 as next send
        USBDeviceStateSuspended
};

#define USB_NUM_OF_DEFINED_HID 1 //TODO change to 2?
uint32_t USB_HID_PROTOCOL[USB_NUM_OF_DEFINED_HID]={0};
void URBD_IRQHandler(void){ //will be called for all activated USB_INTEN events
    static uint32_t deviceState=USBDeviceStateDefault;
    if(USB_INTSTS&USB_INT_USB_MASK){ //Got some USB packets to process
        if(USB_INTSTS&USB_INT_SETUP_MASK){//the received packet was a setup packet, process it
            USB_INTSTS=USB_INT_SETUP_MASK; //Clear USB setup interrupt
            uint8_t* usbSRAM=(uint8_t*)&USB_SRAM_SETUP_START; //TODO remove: get pointer to setup memory location
            //switch request type
            if((usbSRAM[0]&0x60)==0x00){                                              //Standard request (defined by USB standard)
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
                if((usbSRAM[0]&0x1f)==0x00){                                              //Standard Device Request
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){                     //device should send status to host
                        //Defined for all device states for "device status"
                        uint8_t selfPowered=((USB_CONFIG_Descriptor1[8]&0x40)>>6);
                        uint8_t data[2]={(USB_D_REMOTE_WAKEUP<<1)+selfPowered,0x00};
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                        //Data Stage:
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));

                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){                     //host wants to clear a device feature
                        if((deviceState==USBDeviceStateConfigured)&&(deviceState==USBDeviceStateAddress)&&(usbSRAM[2]==0x01)){ //clear DEVICE_REMOTE_WAKEUP
                            wakeupHostEnabeled=0;
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,0,0); //send zero length packet to confirm reception
                        }else{  //unsupported for device
                            USB_set_ctrl_stall();
                        }

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){                     //host wants to set a device feature
                        if(usbSRAM[2]==0x01){//set Device can wakeup host
                            wakeupHostEnabeled=1;
                        }else{ //as a full speed device we do not support any other features
                            USB_set_ctrl_stall();
                        }
                        //Status Stage:
                        USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,0,0); //send zero length packet to confirm reception

                    }else if((usbSRAM[1]==0x05)&&(!(usbSRAM[0]&0x80))){                     //host wants to set the devices address
                        uint32_t address=usbSRAM[2]+(usbSRAM[3]<<8);
                        if(deviceState==USBDeviceStateConfigured){
                            USB_set_ctrl_stall();
                        }else{
                            if(address){ //Set new address if it's not zero
                                USB_FADDR=0x000007f&address;
                                deviceState=USBDeviceStateAddress;
                            }else{
                                if(deviceState==USBDeviceStateAddress){ //if it's zero and in address state switch to default state
                                    deviceState=USBDeviceStateDefault;
                                }
                            }
                            //Status stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,0,0); //send zero length packet to confirm reception
                        }

                    }else if((usbSRAM[1]==0x06)&&( (usbSRAM[0]&0x80))){                     //device should return a descriptor (string,configuration,device, (NOT endpoint or Interface))
                        //Request is valid in all states of deviceState
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                        if(usbSRAM[3]==0x01){//type of decriptor to return (device descriptor)
                            //Status Stage:
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0);
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,(uint8_t*)USB_DEVICE_Descriptor,USB_DEVICE_Descriptor[0]);
                        }else if(usbSRAM[3]==0x02){ //type of decriptor to return (configuration descriptor)
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,(uint8_t*)USB_CONFIGURATION_DESCRIPTOR_ARRAY[usbSRAM[2]],USB_CONFIGURATION_DESCRIPTOR_ARRAY[usbSRAM[2]][0]);
                        }else if(usbSRAM[3]==0x03){ //type of decriptor to return (string descriptor)ion by host after Data stage
                            //Data Stage:
                            //TODO check if Language id matches and select accordingly            if(usbSRAM[4]==0x09&&usbSRAM[5]==0x04){
                            //Select the string descriptor based on the index (warning index is 0 indexed)
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,(uint8_t*)USB_STRING_DESCRIPTOR_ARRAY[usbSRAM[2]],USB_STRING_DESCRIPTOR_ARRAY[usbSRAM[2]][0]);
                        }else{
                            //error: All other descriptor types are not allowed to accessed
                            USB_set_ctrl_stall();
                        }

                    }else if((usbSRAM[1]==0x07)&&(!(usbSRAM[0]&0x80))){                     //host wants to set a device descriptor
                        //Device does not support creation of new descriptors or changing existing ones so:
                        USB_set_ctrl_stall();
                    }else if((usbSRAM[1]==0x08)&&( (usbSRAM[0]&0x80))){                     //device should return which configuration is used (bConfigurationValue)
                        if(deviceState==USBDeviceStateAddress){
                            uint8_t data[1]={0x00};
                            //prepare Status Stage
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0);
                            //Data Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data)); //Needs to return zero value if not configured
                        }else if(deviceState==USBDeviceStateConfigured){
                            uint8_t data[1]={USB_CONFIG_Descriptor1[5]};
                            //prepare Status Stage
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0);
                            //Data Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data)); //Needs to return configdesc value

                        }else{
                            USB_set_ctrl_stall();//error: device was not configured (undefined behavior)
                        }
                    }else if((usbSRAM[1]==0x09)&&(!(usbSRAM[0]&0x80))){                     //host wants to set a device to one of its configurations (bConfigurationValue)
                        if(deviceState==USBDeviceStateDefault){
                            USB_set_ctrl_stall();//ShouldNotHappen
                        }else{
                            if(usbSRAM[2]){ //if the selected configuration is not zero
                                if(usbSRAM[2]<=USB_DD_NUM_CONFGR){ //if this configuration exists (as we numbered them from 0x01 to USB_DD_NUM_CONFGR)
                                    activeConfiguration=usbSRAM[2];
                                    //TODO call user function to notify on configuration change
                                    deviceState=USBDeviceStateConfigured;
                                    //Status Stage
                                    USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                                }else{
                                    USB_set_ctrl_stall(); //host tries to select nonexistent configuration, abort
                                }
                            }else{
                                deviceState=USBDeviceStateAddress;
                                //Status Stage
                                USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                            }
                        }

                    }else{//Undefined standard device request
                        USB_set_ctrl_stall();
                    }
                }else if((usbSRAM[0]&0x1f)==0x01){ //Standard Interface Request
                     /*FromHostToDevice  Request Type                DataLength
                       0                 0x00 (get status)           2
                       1                 0x01 (clear feature)        0
                       1                 0x03 (set feature)          0
                       0                 0x0A (get interface)        1          (not used)
                       1                 0x11 (set interface)        0          (not used)
                    */
                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //interface should send status to host
                        if(deviceState==USBDeviceStateAddress&&(usbSRAM[4]&0x7f)){ //if in address state and something else than endpoint zero gets selected then send request error
                                USB_set_ctrl_stall();
                        }else{
                            uint8_t data[2]={0x00,0x00}; //Interface status is always 0x0000
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));
                        }

                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a interface feature
                        USB_set_ctrl_stall();
                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a interface feature
                        USB_set_ctrl_stall();
                    }else if((usbSRAM[1]==0x0A)&&( (usbSRAM[0]&0x80))){ //device should return alternative setting???
                        //Let's hope that that's not happening, we
                        //TODO implement and switch interface based on usbSRAM[4]

                    }else if((usbSRAM[1]==0x11)&&(!(usbSRAM[0]&0x80))){ //host wants to set device interface to alternative setting
                        //no alternative interface given to host in description so should not happen
                        //TODO implement and switch interface based on usbSRAM[4]

                        USB_set_ctrl_stall(); //Error not supported

                    }else{
                        USB_set_ctrl_stall(); //Error not supported

                    }
                }else if((usbSRAM[0]&0x1f)==0x02){ //Standard Endpoint
                    /*FromHostToDevice  Request Type                DataLength
                      0                 0x00 (get status)           2
                      1                 0x01 (clear feature)        0
                      1                 0x03 (set feature)          0
                    */

                    if      ((usbSRAM[1]==0x00)&&( (usbSRAM[0]&0x80))){ //endpoint should send status to host
                        if      (usbSRAM[4]==0x10){//CRTL_IN
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_get_ep_stall_bit(USB_CTRL_IN)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));
                        }else if(usbSRAM[4]==0x00){//CTRL_OUT
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_get_ep_stall_bit(USB_CTRL_OUT)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));
                        }else if(usbSRAM[4]==0x11){//KEYBOARD IN
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_get_ep_stall_bit(2)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));
                        }else if(usbSRAM[4]==0x12){//MOUSE IN
                            //Type Control Read (Setup->DataIn->StatusOut)
                            uint8_t data[1]={USB_get_ep_stall_bit(3)};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));
                        }else{
                            USB_set_ctrl_stall();
                        }
                    }else if((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //host wants to clear a endpoint feature
                        if(usbSRAM[2]==0x00){ //Endpoint halt
                            if(usbSRAM[4]==0x10){ //CTRL_IN
                                USB_clear_ep_stall_bit(USB_CTRL_IN);
                                //Status Stage
                                USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                            }else if(usbSRAM[4]==0x00){ //CTRL_OUT
                                USB_clear_ep_stall_bit(USB_CTRL_OUT);
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
                        }else{
                            USB_set_ctrl_stall();
                        }

                    }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //host wants to set a endpoint feature
                        if(usbSRAM[2]==0x00){
                            if(usbSRAM[4]==0x10){ //CTRL_IN
                                USB_set_ep_stall_bit(USB_CTRL_IN);
                                //Status Stage
                                USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                            }else if(usbSRAM[4]==0x00){ //CTRL_OUT
                                USB_set_ep_stall_bit(USB_CTRL_OUT);
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
                            USB_set_ctrl_stall();
                        }

                    }else{
                        USB_set_ctrl_stall(); //Error not supported

                    }
                }else{
                    USB_set_ctrl_stall(); //Error not supported

                }
            }else if((usbSRAM[0]&0x60)==0x20){ //Class request (defined by USB class in use eg. HID)
                //Set Selector Unit Control
                //Get Report

                if      ((usbSRAM[1]==0x01)&&(!(usbSRAM[0]&0x80))){ //Host wants the device to send a hid report descriptor

                    //TODO if(usbSRAM[])
                }else if((usbSRAM[1]==0x03)&&(!(usbSRAM[0]&0x80))){ //Get Protocol
                    for(uint32_t i=0;i<USB_NUM_OF_DEFINED_HID;i++){
                        if(usbSRAM[4]<=USB_NUM_OF_DEFINED_HID){
                            uint8_t data[1]={USB_HID_PROTOCOL[i]};
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_PrepareToRecieveEp(USB_CTRL_OUT,NULL,0); //Get ready to get "zero byte" confirmation by host after Data stage
                            //Data Stage:
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,data,sizeof(data));
                        }
                    }
                }else if((usbSRAM[1]==0x0a)&&(!(usbSRAM[0]&0x80))){ //Set Idle

                }else if((usbSRAM[1]==0x0b)&&(!(usbSRAM[0]&0x80))){ //Set Protocol
                    for(uint32_t i=0;i<USB_NUM_OF_DEFINED_HID;i++){
                        if(usbSRAM[4]<=USB_NUM_OF_DEFINED_HID){
                            USB_HID_PROTOCOL[i]=usbSRAM[2];
                            //Status Stage
                            USB_PrepareToSendEpAfterSetup(USB_CTRL_IN,NULL,0);
                        }
                    }
                }else if((usbSRAM[1]==0x0A)&&(!(usbSRAM[0]&0x80))){ //Get Report

                }else{
                    USB_set_ctrl_stall(); //Should not happen
                }
            }else if((usbSRAM[0]&0x60)==0x40){ //Vendor Request (defined by USER/VENDOR)
                USB_set_ctrl_stall(); //Should not happen
            }

            //Clear Interrupt
            USB_INTSTS=USB_INT_SETUP_MASK;
        } //if INT_SETUP_MASK end
        else{ //matches to all ep interrupts
            for(uint32_t epnum=0;epnum<USB_NUM_OF_DEFINED_ENDP;epnum++){
                if(USB_INTSTS&USB_INT_EP_MASK(epnum)){
                    USB_INTSTS=USB_INT_EP_MASK(epnum);//Clear this interrupt flag
                    if(EP_CONFIG_ARRAY[epnum].EP_STATE==EP_IN){ //From device to host
                        if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left){ //We still have to transmit data
                            if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left>USB_SRAM_ENDP_SIZE){ //will be at least one additional transfer
                                for(uint32_t i=0;i<USB_SRAM_ENDP_SIZE;i++){
                                    USB_SRAM_EP_ADDR(epnum)[i]=EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i];
                                }
                                USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
                                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr+=USB_SRAM_ENDP_SIZE;
                                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left-=USB_SRAM_ENDP_SIZE;
                            }else{
                                for(uint32_t i=0;i<EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left;i++){
                                    USB_SRAM_EP_ADDR(epnum)[i]=EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i];
                                }
                                USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
                                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=NULL;
                                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left=0;
                            }
                        }else{ //Host wants data but we don't have requested him to ask for any
                            USB_set_ctrl_stall();
                        }
                    }else if(EP_CONFIG_ARRAY[epnum].EP_STATE==EP_OUT){                      //From host to device
                        if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left){ //We still have to recieve data
                            uint32_t dataInBuffer=USB_MAXPLD(epnum);
                            for(uint32_t i=0;i<dataInBuffer;i++){
                                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i]=USB_SRAM_EP_ADDR(epnum)[i];
                            }
                            if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left>USB_SRAM_ENDP_SIZE){ //we expect another transfer
                                if(dataInBuffer==USB_SRAM_ENDP_SIZE){
                                    EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left-=dataInBuffer;
                                    EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr+=USB_SRAM_ENDP_SIZE;
                                }else{ //host did not send us enough or to much data
                                    USB_set_ctrl_stall();
                                }
                            }else{
                                if(dataInBuffer==EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left){ //we got as many bytes as we expected
                                    EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_left=0;
                                    EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=NULL;
                                }else{//host did not send us enough or to much data
                                    USB_set_ctrl_stall();
                                }
                            }
                        }else{
                            //this is just the host confirming the reception with a zero byte transfer
                        }
                    }
                } //closing brace of ep has intterupt
            } //closing brace of for loop
        } //closing brace of all left interrupts
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
        USB_INTSTS=USB_INT_BUS_MASK;

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
