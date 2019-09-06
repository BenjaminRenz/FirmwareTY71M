/*TODO list
    -device halt feature support
    -call user function to inform on attach/detach
    -alternative interface modes
    -build functions to set test mode for device recipient
*/

#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED
#include <stdint.h>    //needed for uint32_t
#include <stddef.h>    //needed for sizeof("structs")
#include "nvic.h"      //Required to enable USB interrupts
#include "backlight.h" //TODO remove (For testing purposes)
#include "usb_classreq.h" //To handle all class requests like HID...
//Sources
/*
https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/usb.chm/usb6.htm
https://www.beyondlogic.org/usbnutshell/usb6.shtml#GetDescriptor
http://sdphca.ucsd.edu/lab_equip_manuals/usb_20.pdf
https://proyectosfie.webcindario.com/usb/libro/capitulo11.pdf   //Info on hid
https://wiki.osdev.org/Universal_Serial_Bus                     //Info on data toggle
*/


//Notes on data toggle:
/*  -Toggle are independent for CTRL_IN and CTRL_OUT endpoint
    -the NUC toggles automatically after each reception/transmission of the endpoint, so only handle the non automatic cases...
    -after reset, startup and when endpoint resumes from halt we need to set it to data0
    -when reconfiguring a bulk ep, reset to data0 TODO
    -The status stage after a setup packet happens with a data1 packet, for either CTRL_IN or CTRL_OUT
    -When there is a CTRL_IN transfer after the setup packet, data1 will be the first packet
    -Assumption: NUC does not automatically sets data0 for CTRL_OUT when receiving a setup packet
*/


uint8_t* ep2reportstorage=0;



enum{EP_IN=0b10, EP_OUT=0b01, EP_DISABELED=0b00};   //EP_STATE
enum{EP_IS_ISOCHR=1,EP_NOT_ISOCHR=0};               //EP_ISOCH

typedef struct USB_setup_packet{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint8_t wValueLow;
    uint8_t wValueHigh;
    uint8_t wIndexLow;
    uint8_t wIndexHigh;
    uint8_t wLengthLow;
    uint8_t wLengthHigh;
} USB_setup_packet;

#define no_bytes_left -1 //Sending large packets requires multiple transfers, but zero length transfers are also possible as status report, therefore "no bytes"=-1 and "zero length transfer"=0

typedef struct USB_EP_CONFIG{
    uint32_t EP_STATE :2;
    uint32_t EP_ADDR :4;
    uint32_t EP_ISOCH :1;
    int32_t  MultiStage_Bytes; //signdness is used so 0 is zero length transfer
    uint8_t* MultiStage_Storage_ptr;
} USB_EP_CONFIG;


//WARNING total sram space is 512bytes, setup space is 8bytes+USB_NUM_OF_DEFINED_ENDP*64bytes
//-> do not enable all endpoints or decrease the USB_SRAM_ENDP_SIZE
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
    .EP_STATE=EP_OUT,
    .EP_ADDR=1,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP4_CFG={
    .EP_STATE=EP_IN,
    .EP_ADDR=2,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP5_CFG={
    .EP_STATE=EP_OUT,
    .EP_ADDR=2,
    .EP_ISOCH=EP_NOT_ISOCHR
};
USB_EP_CONFIG EP6_CFG={
    .EP_STATE=EP_DISABELED,
};
USB_EP_CONFIG EP7_CFG={
    .EP_STATE=EP_DISABELED,
};


//BEGIN USER SETTINGS
#define USB_SRAM_SETUP_SIZE 0x08    //can only be multiples of 0x08
#define USB_SRAM_ENDP_SIZE  0x40    //can only be multiples of 0x08

//END USER SETTINGS
#define USB_BA 0x40060000
#define USB_SRAM_SETUP_START (*((uint32_t*)(USB_BA+0x100)))

//Globals
USB_setup_packet* sup=(USB_setup_packet*)&USB_SRAM_SETUP_START;
uint32_t wakeupHostEnabeled=1;
uint32_t activeConfiguration=0;
#define dontChangeAddress -1
int8_t changeAddressTo=dontChangeAddress;  //variable needed to indicate to interrupt that the address should change after the transfer
USB_EP_CONFIG EP_CONFIG_ARRAY[8];
uint8_t USB_CTRL_IN=0;
uint8_t USB_CTRL_OUT=0;
uint8_t USB_NUM_OF_DEFINED_ENDP=0;
#include "usb_descriptors.h"

//Register and Address definitions
#define USB_SRAM_EP_START(epnum) (*((uint32_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum)))
#define USB_SRAM_EP_ADDR(epnum) ((uint8_t*)(USB_BA+0x100+USB_SRAM_SETUP_SIZE+USB_SRAM_ENDP_SIZE*epnum))
#define USB_MAXPLD(epnum) (*((uint32_t*)(USB_BA+0x504+((0x000000ff&epnum)<<4))))
#define USB_EPSTS (*((uint32_t*)(USB_BA+0x00c)))
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

static __inline uint32_t minOf(uint32_t arg1, uint32_t arg2){
    return (arg1<arg2)?arg1:arg2;
}

#define USB_clear_setup_stall(epnum)   (USB_CFG(ep))|=0x00000200 //Clear Stall by setting this bit

#define USB_set_ep_stall_bit(epnum)    (USB_CFGP(epnum)|=0x00000002)
#define USB_clear_ep_stall_bit(epnum)  (USB_CFGP(epnum)&=0xfffffffd)
#define USB_get_ep_stall_bit(epnum)    ((USB_CFGP(epnum)&0x00000002)>>1)
static __inline void USB_set_ctrl_stall(uint8_t debugid){ //if error indicate that to the host
    debugr=255;
    if(debugid){
        UART0_send_async("!",1,0);
    }
    //USB_set_ep_stall_bit(USB_CTRL_OUT); TEST don't stall out endpoint to recieve further instructions from host
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
    USB_ATTR=0x000007D0; //TODO still valid? Seems to get ignored
    EP_CONFIG_ARRAY[0]=EP0_CFG;
    EP_CONFIG_ARRAY[1]=EP1_CFG;
    EP_CONFIG_ARRAY[2]=EP2_CFG;
    EP_CONFIG_ARRAY[3]=EP3_CFG;
    EP_CONFIG_ARRAY[4]=EP4_CFG;
    EP_CONFIG_ARRAY[5]=EP5_CFG;
    EP_CONFIG_ARRAY[6]=EP6_CFG;
    EP_CONFIG_ARRAY[7]=EP7_CFG;
    //USB_BUFFSEG for the setup buffer is already set to 0x00000000 on reset indicating no offset for setup storage space
    for(uint32_t epnum=0;epnum<(sizeof(EP_CONFIG_ARRAY)/sizeof(EP_CONFIG_ARRAY[0]));epnum++){
        //Segment the Endpoint Buffers and setup the endpoints
        USB_EP_CONFIG EP_CFG=EP_CONFIG_ARRAY[epnum];
        USB_BUFFSEG(epnum)=(USB_SRAM_SETUP_SIZE+epnum*USB_SRAM_ENDP_SIZE);
        uint32_t oldCFGval=USB_CFG(epnum)&0x00000380;
        USB_CFG(epnum)=oldCFGval+(EP_CFG.EP_STATE<<5)+(EP_CFG.EP_ISOCH<<0x10)+EP_CFG.EP_ADDR;
        //Check which ones are the ctrl endpoints and remember index

        if(EP_CFG.EP_STATE==EP_IN){
            USB_NUM_OF_DEFINED_ENDP++;
            if(EP_CFG.EP_ADDR==0){
                USB_CTRL_IN=epnum;
            }
            EP_CFG.MultiStage_Bytes=no_bytes_left; //do not send anything yet
        }else if(EP_CFG.EP_STATE==EP_OUT){
            USB_NUM_OF_DEFINED_ENDP++;
            if(EP_CFG.EP_ADDR==0){
                USB_CTRL_OUT=epnum;
            }
            EP_CFG.MultiStage_Bytes=0; //we have not recieved something yet
        }
        USB_EP_TO_DATAn(epnum,0); //All controll transfers start with data0 in general, so make sure this after a fresh start
    }
    /*USB_enable_phy(); //TODO find out when to do this...
    */

    USB_enable_controller(); //TODO find out when to do this...
    //enable USB, BUS and FLOATDET interrupt events
    USB_set_se0();
    USB_INTSTS=0x80ff000f; //Clear all interrupts
    USB_INTEN=0x00000007;
}


#define USB_is_attached (USB_FLDET&0x00000001)

void USB_process_rx_tx(uint32_t epnum){
    if(EP_CONFIG_ARRAY[epnum].EP_STATE==EP_IN){ //From device to host
        if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes!=no_bytes_left){ //bytes to send left
            if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes>=USB_SRAM_ENDP_SIZE){ //will be at least one additional transfer
                for(uint32_t i=0;i<USB_SRAM_ENDP_SIZE;i++){
                    USB_SRAM_EP_ADDR(epnum)[i]=EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i];
                }
                USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr+=USB_SRAM_ENDP_SIZE; //shift the storage pointer by endpoint size
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes-=USB_SRAM_ENDP_SIZE; //log how many bytes are left
                //UART0_send_async("um",2,0);
            }else{  //only this transfer left
                for(int32_t i=0;i<EP_CONFIG_ARRAY[epnum].MultiStage_Bytes;i++){
                    USB_SRAM_EP_ADDR(epnum)[i]=EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i];
                }
                if(epnum==USB_CTRL_IN){
                    if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes==0){
                        USB_EP_TO_DATAn(USB_CTRL_IN,1); //send zerobyte with data1
                        USB_EP_TO_DATAn(USB_CTRL_OUT,0);//reset for next reception to data0 for setup
                    }else{
                        USB_EP_TO_DATAn(USB_CTRL_OUT,1); //reviece next zerobyte with data1, therfore data0 for setup
                    }
                }
                USB_MAXPLD(epnum)=EP_CONFIG_ARRAY[epnum].MultiStage_Bytes;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=NULL;
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes=no_bytes_left; //transfer finished

                //UART0_send_async("u",1,0);
                //TODO free storage?
            }
        }else{
            if(changeAddressTo!=dontChangeAddress){ //must happen after status stage has finished
                USB_FADDR&=0xffffff80;
                USB_FADDR|=0x000007f&changeAddressTo; //Careful overlaps with usbattr
                changeAddressTo=dontChangeAddress;
            }
            //normal,another interrupt triggered debugr=255; //we dont have any data to send, not even a zero byte transfer, but host still has requested data -> error
        }
    }else if(EP_CONFIG_ARRAY[epnum].EP_STATE==EP_OUT){                      //From host to device
        int32_t bytesInBuffer=USB_MAXPLD(epnum); //read how many bytes have been recieved
        if(bytesInBuffer<=EP_CONFIG_ARRAY[epnum].MultiStage_Bytes){ //we have not yet exeeded the maximum allowed storage space
            int32_t i=0;
            while(i<bytesInBuffer){ //read out data from peripheral
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i]=USB_SRAM_EP_ADDR(epnum)[i];
                i++;
            }
            if(bytesInBuffer==USB_SRAM_ENDP_SIZE){ //there will be another transfer
                USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE; //allow another transfer
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes-=bytesInBuffer;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr+=bytesInBuffer;
            }else{ //last transfer
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes=no_bytes_left; //transfer finished
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=NULL;
                //TODO remove hack
                if(epnum==2){//keyboard transfer finished
                    reportPressedKeys(0,keys,ep2reportstorage);
                    USB_initiate_send(2,ep2reportstorage,8);
                    debugg=255;
                }
            }
        }
    }
}

void USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength){
    if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes==no_bytes_left){ //check if busy
        USB_EP_TO_DATAn(epnum,1);
        EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=data;
        EP_CONFIG_ARRAY[epnum].MultiStage_Bytes=packetLength;
        USB_process_rx_tx(epnum);
    }else{
        debugb=255;
        debugr=255;
        UART0_send_async("q",1,0);
    }
}

void USB_initiate_send_zerobyte(){ //status stage is always data1
    if(EP_CONFIG_ARRAY[USB_CTRL_IN].MultiStage_Bytes==no_bytes_left){ //check if busy
        //USB_EP_TO_DATAn(USB_CTRL_IN,1);
        EP_CONFIG_ARRAY[USB_CTRL_IN].MultiStage_Storage_ptr=NULL;
        EP_CONFIG_ARRAY[USB_CTRL_IN].MultiStage_Bytes=0; //zero length packet
        USB_process_rx_tx(USB_CTRL_IN);     //send zerobyte
    }else{
        debugg=255;
        debugr=255;
        UART0_send_async("w",1,0);
    }
}

void USB_initiate_recieve_zerobyte(){
    //USB_EP_TO_DATAn(USB_CTRL_OUT,1); //wait for zerobyte
    USB_MAXPLD(USB_CTRL_OUT)=0;
    EP_CONFIG_ARRAY[USB_CTRL_OUT].MultiStage_Storage_ptr=NULL;
    EP_CONFIG_ARRAY[USB_CTRL_OUT].MultiStage_Bytes=0; //Zerobyte needs to be revieved
}

void USB_initiate_recieve(uint32_t epnum, uint8_t* data, uint32_t MaxStorageSpace){ //is also compatible with zero length transfers
    //TODO change this depending on MAXStorageSpace? might be neccesary
    USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE; //set to full size of usb buffer
    EP_CONFIG_ARRAY[USB_CTRL_OUT].MultiStage_Storage_ptr=data;
    EP_CONFIG_ARRAY[epnum].MultiStage_Bytes=MaxStorageSpace;
}

//USB state
enum {  USBDeviceStateDefault=1, //Mode after reset
        USBDeviceStateAddress=2, //Moder after receiving address by host
        USBDeviceStateConfigured=3, //Moder after SetConfiguration, must reset all states and Data0 as next send TODO?
        USBDeviceStateSuspended=4
};


#define wLength ((((uint32_t)(sup->wLengthHigh))<<8)+sup->wLengthLow)
#define wValue ((((uint32_t)(sup->wValueHigh))<<8)+sup->wValueLow)
#define USB_NUM_OF_DEFINED_HID 1
#define USB_NUM_OF_DEFINED_REPORTS 1
uint32_t USB_HID_PROTOCOL[USB_NUM_OF_DEFINED_HID]={1}; //0 for boot protocol,1 for report protocol, hid standard forces initialized value to be report protocol
uint8_t USB_HID_IDLE_RATE[USB_NUM_OF_DEFINED_REPORTS]={125}; //unit is 4ms, default for keyboard is 500ms, need fixing see TODOs, no support for different report id's just yet
void USBD_IRQHandler(void){ //will be called for all activated USB_INTEN events
    static uint32_t deviceState=USBDeviceStateDefault;
    if(USB_INTSTS&USB_INT_USB_MASK){ //Got some USB packets to process
        if(USB_INTSTS&USB_INT_SETUP_MASK){//the received packet was a setup packet, process it
            //TODO block any further packets to be recieved with strop transaction??
            //TODO test unstall when next packet is recieved (device qualifier descriptor)
            USB_clear_ep_stall_bit(USB_CTRL_IN);
            //switch request type
            //print8shortHEX(sup->bRequest); TODO remove
            if((sup->bmRequestType&0x60)==0x00){                                              //Standard request (defined by USB standard)
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
                if((sup->bmRequestType&0x1f)==0x00){                                              //Standard Device Request
                    if      ((sup->bRequest==0x00)&&( (sup->bmRequestType&0x80))){                     //device should send status to host
                        //Defined for all device states for "device status"
                        uint8_t selfPowered=((USB_CONFIG_Descriptor1[8]&0x40)>>6);
                        uint8_t data[2]={(USB_D_REMOTE_WAKEUP<<1)+selfPowered,0x00};
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_initiate_recieve_zerobyte(); //Get ready to get "zero byte" confirmation by host after Data stage
                        //Data Stage:
                        USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength));

                    }else if((sup->bRequest==0x01)&&(!(sup->bmRequestType&0x80))){                     //host wants to clear a device feature
                        if((deviceState==USBDeviceStateConfigured)&&(deviceState==USBDeviceStateAddress)&&(sup->wValueLow==0x01)){ //clear DEVICE_REMOTE_WAKEUP
                            wakeupHostEnabeled=0;
                            //Status Stage
                            USB_initiate_send_zerobyte(); //send zero length packet to confirm
                        }else{  //unsupported for device
                            USB_set_ctrl_stall(1);
                        }

                    }else if((sup->bRequest==0x03)&&(!(sup->bmRequestType&0x80))){                     //host wants to set a device feature
                        if(sup->wValueLow==0x01){//set Device can wakeup host
                            wakeupHostEnabeled=1;
                            //Status Stage:
                            USB_initiate_send_zerobyte(); //send zero length packet to confirm reception
                        }else{ //as a full speed device we do not support any other features
                            USB_set_ctrl_stall(2);
                        }
                    }else if((sup->bRequest==0x05)&&(!(sup->bmRequestType&0x80))){                     //host wants to set the devices address
                        //TODO support test mode
                        if(deviceState==USBDeviceStateConfigured){
                            USB_set_ctrl_stall(3);
                        }else{
                            if(sup->wValueLow){ //Set new address if it's not zero
                                deviceState=USBDeviceStateAddress;
                                changeAddressTo=sup->wValueLow;
                                UART0_send_async("a",1,0);
                                USB_initiate_send_zerobyte();
                            }else{
                                if(deviceState==USBDeviceStateAddress){ //if it's zero and in address state switch to default state
                                    deviceState=USBDeviceStateDefault;
                                    changeAddressTo=0;
                                    USB_initiate_send_zerobyte();
                                }
                            }
                        }
                    }else if((sup->bRequest==0x06)&&( (sup->bmRequestType&0x80))){                     //device should return a descriptor (string,configuration,device, (NOT endpoint or Interface))
                        //Request is valid in all states of deviceState
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_initiate_recieve_zerobyte(); //Get ready to get "zero byte" confirmation by host after Data stage
                        if(sup->wValueHigh==0x01){//type of decriptor to return (device descriptor)
                            //UART0_send_async("2",1,0);
                            //Data Stage:
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_DEVICE_Descriptor,minOf(USB_DEVICE_Descriptor[0],wLength));
                            //remove? USB_EP_TO_DATAn(USB_CTRL_IN,1);
                            //USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_DEVICE_Descriptor,wLength);
                        }else if(sup->wValueHigh==0x02){ //type of decriptor to return (configuration descriptor)
                            //Data Stage:
                            //UART0_send_async("3",1,0);
                            //FIX? this is returned quiet often possibly wrong TODO
                            uint32_t ConfigDescriptorTotalLength=(USB_CONFIGURATION_DESCRIPTOR_ARRAY[sup->wValueLow][3]<<8)+USB_CONFIGURATION_DESCRIPTOR_ARRAY[sup->wValueLow][2];
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_CONFIGURATION_DESCRIPTOR_ARRAY[sup->wValueLow],minOf(ConfigDescriptorTotalLength,wLength));
                        }else if(sup->wValueHigh==0x03){ //type of decriptor to return (string descriptor)ion by host after Data stage
                            //Data Stage:
                            //TODO check if Language id matches and select accordingly            if(sup->wIndexLow==0x09&&usbSRAM[5]==0x04){
                            //Select the string descriptor based on the index (warning index is 0 indexed)
                            //UART0_send_async("4",1,0);
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_STRING_DESCRIPTOR_ARRAY[sup->wValueLow],minOf(USB_STRING_DESCRIPTOR_ARRAY[sup->wValueLow][0],wLength));
                        }else if(sup->wValueHigh==0x06){ //device qualifier descriptor
                            UART0_send_async("5",1,0);
                            //USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_DEVICE_QUALIFIER_Descriptor,minOf(USB_DEVICE_QUALIFIER_Descriptor[0],wLength));
                            USB_set_ep_stall_bit(USB_CTRL_IN);
                            UART0_send_async("q",1,0);
                        }else{
                            //error: All other descriptor types are not allowed to accessed
                            USB_set_ctrl_stall(5);
                        }

                    }else if((sup->bRequest==0x07)&&(!(sup->bmRequestType&0x80))){                     //host wants to set a device descriptor
                        UART0_send_async("o",1,0);
                        //Device does not support creation of new descriptors or changing existing ones so:
                        USB_set_ctrl_stall(6);
                    }else if((sup->bRequest==0x08)&&( (sup->bmRequestType&0x80))){                     //device should return which configuration is used (bConfigurationValue)
                        UART0_send_async("b",1,0);
                        if(deviceState==USBDeviceStateAddress){
                            uint8_t data[1]={0x00};
                            //prepare Status Stage
                            USB_initiate_recieve_zerobyte(); //for following zerobyte from host
                            //Data Stage
                            USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength)); //Needs to return zero value if not configured
                        }else if(deviceState==USBDeviceStateConfigured){
                            uint8_t data[1]={USB_CONFIG_Descriptor1[5]};
                            //prepare Status Stage
                            USB_initiate_recieve_zerobyte();
                            //Data Stage
                            USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength)); //Needs to return configdesc value

                        }else{
                            USB_set_ctrl_stall(7);//error: device was not configured (undefined behavior)
                        }
                    }else if((sup->bRequest==0x09)&&(!(sup->bmRequestType&0x80))){                     //host wants to set a device to one of its configurations (bConfigurationValue)
                        if(deviceState==USBDeviceStateDefault){
                            USB_set_ctrl_stall(8);//ShouldNotHappen
                        }else{
                            if(sup->wValueLow){ //if the selected configuration is not zero
                                if(sup->wValueLow<=USB_DD_NUM_CONFGR){ //if this configuration exists (as we numbered them from 0x01 to USB_DD_NUM_CONFGR)
                                    activeConfiguration=sup->wValueLow;
                                    //TODO call user function to notify on configuration change
                                    deviceState=USBDeviceStateConfigured;
                                    //Status Stage
                                    USB_initiate_send_zerobyte();
                                }else{
                                    USB_set_ctrl_stall(9); //host tries to select nonexistent configuration, abort
                                }
                            }else{
                                deviceState=USBDeviceStateAddress;
                                //Status Stage
                                USB_initiate_send_zerobyte();
                            }
                        }

                    }else{//Undefined standard device request
                        USB_set_ctrl_stall(10);
                    }
                }else if((sup->bmRequestType&0x1f)==0x01){ //Standard Interface Request
                     /*FromHostToDevice  Request Type                DataLength
                       0                 0x00 (get status)           2
                       1                 0x01 (clear feature)        0
                       1                 0x03 (set feature)          0
                       0                 0x0A (get interface)        1          (not used)
                       1                 0x11 (set interface)        0          (not used)
                    */
                    if      ((sup->bRequest==0x00)&&( (sup->bmRequestType&0x80))){ //interface should send status to host
                        if(deviceState==USBDeviceStateAddress&&(sup->wIndexLow&0x7f)){ //if in address state and something else than endpoint zero gets selected then send request error
                                USB_set_ctrl_stall(11);
                        }else{
                            uint8_t data[2]={0x00,0x00}; //Interface status is always 0x0000
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_initiate_recieve_zerobyte();
                            //Data Stage:
                            USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength));
                        }

                    }else if((sup->bRequest==0x01)&&(!(sup->bmRequestType&0x80))){ //host wants to clear a interface feature
                        USB_set_ctrl_stall(12);
                    }else if((sup->bRequest==0x03)&&(!(sup->bmRequestType&0x80))){ //host wants to set a interface feature
                        USB_set_ctrl_stall(13);
                    }else if((sup->bRequest==0x0A)&&( (sup->bmRequestType&0x80))){ //device should return alternative setting???
                        //TODO implement and switch interface based on sup->wIndexLow
                        //not yet supported, multiple configurations
                        USB_set_ctrl_stall(14);
                    }else if((sup->bRequest==0x11)&&(!(sup->bmRequestType&0x80))){ //host wants to set device interface to alternative setting
                        //no alternative interface given to host in description so should not happen
                        //TODO implement and switch interface based on sup->wIndexLow

                        USB_set_ctrl_stall(15); //Error not supported

                    }else if((sup->bRequest==0x06)&&((sup->bmRequestType&0x80))){ //host wants descriptor like hid from this interface
                        if(sup->wValueHigh==0x21){ //hid descriptor
                            //was already sent as part of configuration descriptor
                            UART0_send_async("i",1,0);
                            USB_set_ctrl_stall(4);
                        }else if(sup->wValueHigh==0x22){ //report descriptor
                            UART0_send_async("n",1,0);
                            USB_initiate_recieve_zerobyte();
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_HID_REPORT_DESCRIPTOR_ARRAY[sup->wValueLow],minOf(sizeof(REPORT_Descriptor_Keyboard),wLength));
                        }else{
                            USB_set_ctrl_stall(30);
                        }
                    }else{
                        USB_set_ctrl_stall(16); //Error not supported

                    }
                }else if((sup->bmRequestType&0x1f)==0x02){ //Standard Endpoint
                    /*FromHostToDevice  Request Type                DataLength
                      0                 0x00 (get status)           2
                      1                 0x01 (clear feature)        0
                      1                 0x03 (set feature)          0
                    */

                    if      ((sup->bRequest==0x00)&&( (sup->bmRequestType&0x80))){ //endpoint should send status to host
                        if(deviceState==USBDeviceStateDefault||(deviceState==USBDeviceStateAddress&&(sup->wIndexLow&0x7f))){ //if in default state or in address and endpoint is !=zero
                            USB_set_ctrl_stall(17);
                        }
                        uint8_t data[2]={0,0};
                        switch(sup->wIndexLow){
                            case 0x80://CRTL_IN
                                data[0]=USB_get_ep_stall_bit(USB_CTRL_IN);
                            break;
                            case 0x00://CTRL_OUT
                                data[0]=USB_get_ep_stall_bit(USB_CTRL_OUT);
                            break;
                            case 0x81:////KEYBOARD IN
                                data[0]=USB_get_ep_stall_bit(2);
                            break;
                            case 0x01:////KEYBOARD OUT
                                data[0]=USB_get_ep_stall_bit(3);
                            break;
                            case 0x82:////MOUSE IN
                                data[0]=USB_get_ep_stall_bit(4);
                            break;
                            case 0x02://MOUSE OUT
                                data[0]=USB_get_ep_stall_bit(5);
                            break;
                            default:
                                USB_set_ctrl_stall(18);
                            break;
                        }
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_initiate_recieve_zerobyte();
                        //Data Stage:
                        USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength));
                    }else if((sup->bRequest==0x01)&&(!(sup->bmRequestType&0x80))){ //host wants to clear a endpoint feature
                        if(deviceState==USBDeviceStateConfigured&&sup->wValueLow==0x00){ //ENDPOINT halt should be cleared
                            switch(sup->wIndexLow){
                                case 0x81:////KEYBOARD IN
                                    USB_clear_ep_stall_bit(2);
                                    USB_EP_TO_DATAn(2,0);
                                break;
                                case 0x01:////KEYBOARD OUT
                                    USB_clear_ep_stall_bit(3);
                                    USB_EP_TO_DATAn(3,0);
                                break;
                                case 0x82:////MOUSE IN
                                    USB_clear_ep_stall_bit(4);
                                    USB_EP_TO_DATAn(4,0);
                                break;
                                case 0x02://MOUSE OUT
                                    USB_clear_ep_stall_bit(5);
                                    USB_EP_TO_DATAn(5,0);
                                break;
                                default:
                                    USB_set_ctrl_stall(19);
                                break;
                            }
                            USB_initiate_send_zerobyte();
                        }else{
                            USB_set_ctrl_stall(20);
                        }
                    }else if((sup->bRequest==0x03)&&(!(sup->bmRequestType&0x80))){ //host wants to set a endpoint feature
                        if(deviceState==USBDeviceStateConfigured&&sup->wValueLow==0x00){ //ENDPOINT halt should be cleared
                            switch(sup->wIndexLow){
                                case 0x81:////KEYBOARD IN
                                    USB_set_ep_stall_bit(2);
                                break;
                                case 0x01:////KEYBOARD OUT
                                    USB_set_ep_stall_bit(3);
                                break;
                                case 0x82:////MOUSE IN
                                    USB_set_ep_stall_bit(4);
                                break;
                                case 0x02://MOUSE OUT
                                    USB_set_ep_stall_bit(5);
                                break;
                                default:
                                    USB_set_ctrl_stall(21);
                                break;
                            }
                            USB_initiate_send_zerobyte();
                        }else{
                            USB_set_ctrl_stall(22);
                        }
                    }else{
                        USB_set_ctrl_stall(23); //Error not supported

                    }
                }else{
                    USB_set_ctrl_stall(24); //Error not supported

                }
            }else if((sup->bmRequestType&0x60)==0x20){ //Class request (defined by USB class in use eg. HID)
                //Class specific functions (HID)
                //reference https://www.renesas.com/eu/en/doc/products/mpumcu/apn/rl78/002/r01an0546ej0215_usb.pdf
                //UART0_send_async("?",1,0);
                if      ((sup->bRequest==0x01)&&(!(sup->bmRequestType&0x80))){ //set report
                    //Not supported
                    USB_set_ctrl_stall(25);
                }else if((sup->bRequest==0x02)&&( (sup->bmRequestType&0x80))){ //get idle rate
                    USB_initiate_recieve_zerobyte();
                    uint8_t data[1]={USB_HID_IDLE_RATE[sup->wValueLow]};
                    USB_initiate_send(USB_CTRL_IN,data,minOf(data,sup->wLengthLow));
                }else if((sup->bRequest==0x03)&&( (sup->bmRequestType&0x80))){ //Get Protocol, requiered for boot devices
                    uint8_t data[1]={USB_HID_PROTOCOL[sup->wIndexLow]};
                    //Status Stage (prepare beforehand) which confirms reception
                    USB_initiate_recieve_zerobyte();
                    //Data Stage:
                    USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength));
                }else if((sup->bRequest==0x06)&&( (sup->bmRequestType&0x80))){ //Get HID Report descriptor

                }else if((sup->bRequest==0x0a)&&(!(sup->bmRequestType&0x80))){ //Set Idle
                    //TODO set timer to wait for idletime
                    //TODO fix, one can define report id's in the report descriptor which start from 1 and can be used to set individual idle times
                    if(sup->wValueLow!=0){//set idle applies to only on interface
                        USB_HID_IDLE_RATE[sup->wValueLow]=sup->wValueHigh;
                    }
                    ep2reportstorage=(uint8_t*)malloc(sizeof(uint8_t)*8);
                    reportPressedKeys(0,keys,ep2reportstorage);
                    USB_initiate_send(2,ep2reportstorage,8);
                    UART0_send_async("p",1,0);
                    //TODO driver could start to request input from now on
                    USB_initiate_send_zerobyte();
                }else if((sup->bRequest==0x0b)&&(!(sup->bmRequestType&0x80))){ //Set Protocol
                    USB_HID_PROTOCOL[sup->wValueLow]=sup->wValueHigh;
                    UART0_send_async("y",1,0);
                    USB_initiate_send_zerobyte();
                }else{
                    USB_set_ctrl_stall(26); //Should not happen
                }
            }else if((sup->bmRequestType&0x60)==0x40){ //Vendor Request (defined by USER/VENDOR)
                USB_set_ctrl_stall(27); //Should not happen
            }

            //Clear Interrupt
            USB_INTSTS=USB_INT_SETUP_MASK;
        } //if INT_SETUP_MASK end
        else{ //matches to all ep interrupts
            for(uint32_t epnum=0;epnum<USB_NUM_OF_DEFINED_ENDP;epnum++){
                if(USB_INTSTS&USB_INT_EP_MASK(epnum)){
                    USB_INTSTS=USB_INT_EP_MASK(epnum);//Clear this interrupt flag
                    switch((USB_EPSTS>>(8+3*epnum))&0x00000007){
                    case USB_EPSTS_SETUPACK:    //ignore setupt ack,
                        break;
                    case USB_EPSTS_INNAK:
                        UART0_send_async("e",1,0); //TODO ?
                        break;
                    default:
                        USB_process_rx_tx(epnum);
                        break;
                    }

                }
            }
        }
    }else if(USB_INTSTS&USB_INT_BUS_MASK){ //BUS event
        if(USB_ATTR&USB_ATTR_RESUME_MASK){  //Resume from suspend
            USB_enable_controller();
            USB_enable_phy();
        }else if(USB_ATTR&USB_ATTR_SUSPEND_MASK){ //Cable plugged out/host is sleeping (bus idle for <3mS)
            USB_disable_phy();
        }else if(USB_ATTR&USB_ATTR_TIMEOUT_MASK){ //Host not responding by ACK packets
        }else if(USB_ATTR&USB_ATTR_USBRST_MASK){ //Reset from host by se0 //something is not quiet working here
            USB_enable_controller();
            USB_enable_phy();
            UART0_send_async("r",1,0);
            for(uint32_t epnum=0;epnum<(sizeof(EP_CONFIG_ARRAY)/sizeof(EP_CONFIG_ARRAY[0]));epnum++){ //TODO check if reset is sufficient
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes=no_bytes_left;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=0;
                USB_stop_transOrRec(epnum);
                USB_EP_TO_DATAn(epnum,0); //All controll transfers start with data0 in general, so make sure this is the case after reset
            }
            USB_INTSTS=0x80ff000f;
            deviceState=USBDeviceStateDefault;
            USB_FADDR&=0xffffff80; //careful overlaps with usbarrtib
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
