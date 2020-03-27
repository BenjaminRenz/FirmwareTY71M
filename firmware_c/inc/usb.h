/** \file
 * \brief handles usb hardware related stuff
 */
/*TODO list
    -device halt feature support
    -call user function to inform on attach/detach
    -alternative interface modes
    -build functions to set test mode for device recipient
    -do data transfers from and to storage with dma
*/


/*Infos, important
-when a transfer, or the last transfer when there are multiple, is copied to the usb endpoint register the MultistageBytesPoints goes to -1 or "nobytesleft" (0 is needed for zerobyte)
-if the transfer is actually completed the storage pointer is overwritten with a null pointer.
*/
#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED
#include <stdint.h>    //needed for uint32_t
#include <stddef.h>    //needed for sizeof("structs")
#include "nvic.h"      //Required to enable USB interrupts
#include "backlight.h" //TODO remove (For testing purposes)

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
    -reset to data0 when:
        -after startup of nuc (all endpoints and unplug)
        -after set configuration
        -after set interface
        -after ClearFeature Halt
    -after reset, startup and when endpoint resumes from halt we need to set it to data0
    -when reconfiguring a bulk ep, reset to data0
    -Special rules for control transfers;
        -setup packet is data0
        -first data packet is data1 (if we send data to host we need to change CTRL_IN to data1)
        -status stage is data1 (we always need to set this up)
    -Assumption: NUC does not automatically sets data0 for CTRL_OUT when receiving a setup packet
*/







/** used in order to access the bytes of the setup packet in the buffer of the usb ctrl out endpoint in a better readable way.
*/
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

/** Used for sending large packets which requiere multiple transfers, but zero length packets also need support. So the code when a multi transfer has no data left is this.
*/
#define no_bytes_left -1

enum{EP_IN=0b10, EP_OUT=0b01, EP_DISABELED=0b00};   //values of EP_STATE
enum{EP_IS_ISOCHR=1,EP_NOT_ISOCHR=0};               //values of EP_ISOCH
/** \brief stucture which holds the current state information for one endpoint
 *
 */
typedef struct USB_EP_CONFIG{
    uint32_t EP_STATE :2;
    uint32_t EP_ADDR :4;
    uint32_t EP_ISOCH :1;
    int32_t  MultiStage_Bytes_leftToSend_or_recieved; //signdness is used so 0 is zero length transfer, see no_bytes_left
    uint8_t* MultiStage_Storage_ptr;
    uint8_t* Storage_Start_ptr;
    void (*callbFuncP)(uint32_t epnum,uint8_t* data, uint32_t packetlength);
} USB_EP_CONFIG;


//WARNING total sram space is 512bytes, setup space is 8bytes+USB_NUM_OF_DEFINED_ENDP*64bytes
//-> do not enable all endpoints or decrease the USB_SRAM_ENDP_SIZE (default 64bytes)
//WARNING EP_IN/_OUT is labeled from the perspective of the usb host (PC), so EP_IN is from keyboard to PC
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


//BEGIN USER SETTINGS
#define USB_SRAM_SETUP_SIZE 0x08    //can only be multiples of 0x08
#define USB_SRAM_ENDP_SIZE  0x40    //can only be multiples of 0x08

//END USER SETTINGS
#define USB_BA 0x40060000
#define USB_SRAM_SETUP_START (*((uint32_t*)(USB_BA+0x100)))
enum {  USBDeviceStateDefault=1, //Mode after reset
        USBDeviceStateAddress=2, //Moder after receiving address by host
        USBDeviceStateConfigured=3, //Moder after SetConfiguration
        USBDeviceStateSuspended=4
};


//Globals
USB_setup_packet* sup=(USB_setup_packet*)&USB_SRAM_SETUP_START;
uint32_t deviceState=USBDeviceStateDefault;
uint32_t wakeupHostEnabeled=1;
uint32_t activeConfiguration=0;
#define dontChangeAddress -1        //possible value of changeAddressTo
int8_t changeAddressTo=dontChangeAddress;  //variable needed to indicate to interrupt that the device address should change after the next transfer
USB_EP_CONFIG EP_CONFIG_ARRAY[8];
uint8_t USB_CTRL_IN=0;
uint8_t USB_CTRL_OUT=0;
uint8_t USB_NUM_OF_DEFINED_ENDP=0;

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

#define USB_set_ep_stall_bit(epnum)    (USB_CFGP(epnum)|=0x00000002)
#define USB_clear_ep_stall_bit(epnum)  (USB_CFGP(epnum)&=0xfffffffd)
#define USB_get_ep_stall_bit(epnum)    ((USB_CFGP(epnum)&0x00000002)>>1)

static __inline void USB_set_CTRL_stall(uint8_t debugid){ //if error indicate that to the host
    if(debugid){
        UART0_send_async("!",1,0);
    }
    USB_set_ep_stall_bit(USB_CTRL_IN);
    USB_set_ep_stall_bit(USB_CTRL_OUT);
}

#define USB_stop_transOrRec(epnum) USB_CFGP(epnum)|=0x00000001//clear in/out ready flag after already written USB_MAXPLD
#define USB_start_reset() IPRSTC2|=(1<<27) //Send Reset Signal to USB block

//only call this after ready with interrupt setup after buffer and after BUFSEG has completed
#define USB_end_reset() IPRSTC2&=~(1<<27)//Stop sending Reset Signal to USB block

#define USB_is_attached (USB_FLDET&0x00000001)


#include "usb_ephandler.h"          //To handle endpoints, fill data for keyboard eg.
#include "usb_descriptors.h"        //is here because some descriptor need to know how many endpoints we have an other details



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
            EP_CFG.MultiStage_Bytes_leftToSend_or_recieved=no_bytes_left; //do not send anything yet
        }else if(EP_CFG.EP_STATE==EP_OUT){
            USB_NUM_OF_DEFINED_ENDP++;
            if(EP_CFG.EP_ADDR==0){
                USB_CTRL_OUT=epnum;
            }
            EP_CFG.MultiStage_Bytes_leftToSend_or_recieved=0; //we have not received something yet
        }
        USB_EP_TO_DATAn(epnum,0); //transfers start with data0 in general, so make sure this is the case after a fresh start
        EP_CFG.callbFuncP=NULL;

    }
    /*USB_enable_phy(); //TODO find out when to do this...
    */

    USB_enable_controller(); //TODO find out when to do this...
    //enable USB, BUS and FLOATDET interrupt events
    USB_set_se0();  //set bus reset
    USB_INTSTS=0x80ff000f; //Clear all interrupts
    USB_INTEN=0x00000007;
}

/** \brief function that handles passing data from and to the endpoint
 *
 * Function is called when new data was recieved by an endpoint, or when a endpoint has transmitted all it's data and is possibly waiting for more data to transmit
 * Since USB transfers might be larger then the endpoint size this function passes new data into the endpoint's buffers.
 * USB_initiate_send() is also calling this function which will start copying the data into the endpoint buffer.
 * \param epnum The endpoint which needs to get new data / has recieved new data.
 */

void USB_process_rx_tx(uint32_t epnum){     //only enters this function when last transfer did complete with an ack
    if(EP_CONFIG_ARRAY[epnum].EP_STATE==EP_IN){ //From device to host PC
        //We will enter this function BEFORE we have transfered the packet which we will prepare now
        if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved!=no_bytes_left){ //bytes to send left
            if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved>=USB_SRAM_ENDP_SIZE){ //will be at least one additional transfer
                for(uint32_t i=0;i<USB_SRAM_ENDP_SIZE;i++){
                    USB_SRAM_EP_ADDR(epnum)[i]=EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i];
                }
                USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr+=USB_SRAM_ENDP_SIZE; //shift the storage pointer by endpoint size
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved-=USB_SRAM_ENDP_SIZE; //log how many bytes are left
                //UART0_send_async("um",2,0);
            }else{  //only this transfer left
                for(int32_t i=0;i<EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved;i++){
                    USB_SRAM_EP_ADDR(epnum)[i]=EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i];
                }
                USB_MAXPLD(epnum)=EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved;
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved=no_bytes_left; //transfer finished

                //UART0_send_async("u",1,0);
                //TODO free storage?
            }
        }else{  //now we are actually finished with transmitting, the interrupt says that the last packet has been sent by the ep
            //free pointer to MultiStage_Storage?
            EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=NULL;
            if(EP_CONFIG_ARRAY[epnum].callbFuncP){
                EP_CONFIG_ARRAY[epnum]->callbFuncP(epnum,EP_CONFIG_ARRAY[epnum].Storage_Start_ptr,EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr-EP_CONFIG_ARRAY[epnum].Storage_Start_ptr);
                EP_CONFIG_ARRAY[epnum].callbFuncP=NULL;     //Unset callback function
            }
            if(changeAddressTo!=dontChangeAddress){ //must happen after status stage has finished
                USB_FADDR&=0xffffff80;
                USB_FADDR|=0x000007f&changeAddressTo; //USB_FADDR overlaps overlaps with usbattr, take care
                changeAddressTo=dontChangeAddress;
            }
        }
    }else if(EP_CONFIG_ARRAY[epnum].EP_STATE==EP_OUT){                      //From host to device
        //We will enter this AFTER we have recieved data
        int32_t bytesInBuffer=USB_MAXPLD(epnum); //read how many bytes have been recieved
        if(bytesInBuffer<=EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved){ //we have not yet exeeded the maximum allowed storage space
            int32_t i=0;    //must be signed for following comparison with signed bytesInBuffer value
            while(i<bytesInBuffer){ //read out data from peripheral
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr[i]=USB_SRAM_EP_ADDR(epnum)[i];
                i++;
            }
            if(bytesInBuffer==USB_SRAM_ENDP_SIZE){ //there will be another transfer
                USB_MAXPLD(epnum)=USB_SRAM_ENDP_SIZE; //allow another transfer
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved-=bytesInBuffer;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr+=bytesInBuffer;
            }else{ //last transfer, we have finished recieving data now
                if(EP_CONFIG_ARRAY[epnum].callbFuncP){
                    EP_CONFIG_ARRAY[epnum].callbFuncP(epnum,EP_CONFIG_ARRAY.Storage_Start_ptr,EP_CONFIG_ARRAY.MultiStage_Bytes_leftToSend_or_recieved);
                    EP_CONFIG_ARRAY[epnum].callbFuncP=NULL;     //Unset callback function
                }
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved=no_bytes_left; //transfer finished
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=NULL;
            }
        }
    }
}

/** \brief used to prepare sending data to the host
 * The data is sent when the host starts polling the endpoint
 * \param epnum number of the endpoint on which data is going to be sent
 * \param data pointer to the data array which is going to be sent
 * \param packetLength length of the data packet in bytes, can be larger than endpoint size, then we will have multiple transfers.
 * \return 1 if the transfer is schedued, 0 if the buffer is already full with another transfer to process.
 */
uint8_t USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength, void (*callbFuncP)(uint32_t epnum,uint8_t* data, uint32_t packetlength)){
    if(EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved==no_bytes_left){ //check if busy
        EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=data;
        EP_CONFIG_ARRAY[epnum].Storage_Start_ptr=data;
        EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved=packetLength;
        EP_CONFIG_ARRAY[epnum].callbFuncP=callbFuncP;
        USB_process_rx_tx(epnum);
        return 1;
    }else{
        UART0_send_async("b1",2,0);
        return 0;
    }
}

/** \brief Used to prepare sending an zerobytepacket on the ctrl_in endpoint to the host
 *
 * Function sets the Data bits for the control_in_endpoint which needs to send data1 for a zerobyte data stage.
 * The data bit for the control_in_endpoint is set to 0 in order to recieve the next setup packet which will be data0.
 *
 */
void USB_initiate_send_ctrl_zerobyte(){
    USB_EP_TO_DATAn(USB_CTRL_IN,1);
    USB_initiate_send(USB_CTRL_IN,NULL,0,NULL);
    USB_EP_TO_DATAn(USB_CTRL_OUT,0);
}

/** \brief Used to prepare reception of data at an usb endpoint
 *
 * \param epnum Endpoint number on which to recieve data
 * \param data Pointer to the location where the recieved data get's stored
 * \param MaxStorageSpace Maximum number of bytes that the storage location can hold (savety check)
 * \param callbFuncP Function pointer which get's called after all data has been recieved
 *
 */
void USB_initiate_recieve(uint32_t epnum, uint8_t* data, uint32_t MaxStorageSpace,void (*callbFuncP)(uint32_t epnum,uint8_t* data, uint32_t packetlength)){
    USB_MAXPLD(epnum)=minOf(USB_SRAM_ENDP_SIZE,MaxStorageSpace);    //recieve only the amount of data we can handle
    EP_CONFIG_ARRAY[epnum].Storage_Start_ptr=data;
    EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=data;
    EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved=MaxStorageSpace;
    EP_CONFIG_ARRAY[epnum].callbFuncP=callbFuncP;
}

void USB_initiate_recieve_ctrl_zerobyte(){
    //we are implicitly also going to send data to the host, starting with data1
    USB_EP_TO_DATAn(USB_CTRL_IN,1);
    //then we need to recieve a zerobyte from the host
    USB_EP_TO_DATAn(USB_CTRL_OUT,1);
    USB_initiate_recieve(USB_CTRL_OUT,NULL,0,NULL);
}



#define wLength ((((uint32_t)(sup->wLengthHigh))<<8)+sup->wLengthLow)
#define wValue ((((uint32_t)(sup->wValueHigh))<<8)+sup->wValueLow)
void USBD_IRQHandler(void){ //will be called for all activated USB_INTEN events
    if(USB_INTSTS&USB_INT_USB_MASK){ //Got some USB packets to process
        if(USB_INTSTS&USB_INT_SETUP_MASK){//the received packet was a setup packet, process it
            //TODO block any further packets to be recieved with strop transaction??
            //TODO test unstall when next packet is recieved (device qualifier descriptor)
            USB_clear_ep_stall_bit(USB_CTRL_IN);
            USB_clear_ep_stall_bit(USB_CTRL_OUT);
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
                        USB_initiate_recieve_ctrl_zerobyte(); //Get ready to get "zero byte" confirmation by host after Data stage
                        USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength),NULL); //Data Stage
                    }else if((sup->bRequest==0x01)&&(!(sup->bmRequestType&0x80))){                     //host wants to clear a device feature
                        if((deviceState==USBDeviceStateConfigured)&&(deviceState==USBDeviceStateAddress)&&(sup->wValueLow==0x01)){ //clear DEVICE_REMOTE_WAKEUP
                            wakeupHostEnabeled=0;
                            //Status Stage
                            USB_initiate_send_ctrl_zerobyte(); //send zero length packet to confirm
                        }else{  //unsupported for device
                            USB_set_CTRL_stall(1);
                        }

                    }else if((sup->bRequest==0x03)&&(!(sup->bmRequestType&0x80))){                     //host wants to set a device feature
                        if(sup->wValueLow==0x01){//set Device can wakeup host
                            wakeupHostEnabeled=1;
                            //Status Stage:
                            USB_initiate_send_ctrl_zerobyte(); //send zero length packet to confirm reception
                        }else{ //as a full speed device we do not support any other features
                            USB_set_CTRL_stall(2);
                        }
                    }else if((sup->bRequest==0x05)&&(!(sup->bmRequestType&0x80))){                     //host wants to set the devices address
                        //TODO support test mode
                        if(deviceState==USBDeviceStateConfigured){
                            USB_set_CTRL_stall(3);
                        }else{
                            if(sup->wValueLow){ //Set new address if it's not zero
                                deviceState=USBDeviceStateAddress;
                                changeAddressTo=sup->wValueLow;
                                UART0_send_async("a",1,0);
                                USB_initiate_send_ctrl_zerobyte();
                            }else{
                                if(deviceState==USBDeviceStateAddress){ //if it's zero and in address state switch to default state
                                    deviceState=USBDeviceStateDefault;
                                    changeAddressTo=0;
                                    USB_initiate_send_ctrl_zerobyte();
                                }
                            }
                        }
                    }else if((sup->bRequest==0x06)&&( (sup->bmRequestType&0x80))){                     //device should return a descriptor (string,configuration,device, (NOT endpoint or Interface))
                        //Request is valid in all states of deviceState
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_initiate_recieve_ctrl_zerobyte(); //Get ready to get "zero byte" confirmation by host after Data stage
                        if(sup->wValueHigh==0x01){//type of decriptor to return (device descriptor)
                            //UART0_send_async("2",1,0);
                            //Data Stage:
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_DEVICE_Descriptor,minOf(USB_DEVICE_Descriptor[0],wLength),NULL);
                            //remove? USB_EP_TO_DATAn(USB_CTRL_IN,1);
                            //USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_DEVICE_Descriptor,wLength);
                        }else if(sup->wValueHigh==0x02){ //type of decriptor to return (configuration descriptor)
                            //Data Stage:
                            //UART0_send_async("3",1,0);
                            //FIX? this is returned quiet often possibly wrong TODO
                            uint32_t ConfigDescriptorTotalLength=(USB_CONFIGURATION_DESCRIPTOR_ARRAY[sup->wValueLow][3]<<8)+USB_CONFIGURATION_DESCRIPTOR_ARRAY[sup->wValueLow][2];
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_CONFIGURATION_DESCRIPTOR_ARRAY[sup->wValueLow],minOf(ConfigDescriptorTotalLength,wLength),NULL);
                        }else if(sup->wValueHigh==0x03){ //type of decriptor to return (string descriptor)ion by host after Data stage
                            //Data Stage:
                            //TODO check if Language id matches and select accordingly            if(sup->wIndexLow==0x09&&usbSRAM[5]==0x04){
                            //Select the string descriptor based on the index (warning index is 0 indexed)
                            //UART0_send_async("4",1,0);
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_STRING_DESCRIPTOR_ARRAY[sup->wValueLow],minOf(USB_STRING_DESCRIPTOR_ARRAY[sup->wValueLow][0],wLength),NULL);
                        }else if(sup->wValueHigh==0x06){ //device qualifier descriptor
                            //USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_DEVICE_QUALIFIER_Descriptor,minOf(USB_DEVICE_QUALIFIER_Descriptor[0],wLength));
                            USB_set_CTRL_stall(42);
                            USB_set_ep_stall_bit(USB_CTRL_IN);
                        }else{
                            //error: All other descriptor types are not allowed to accessed
                            USB_set_CTRL_stall(5);
                        }

                    }else if((sup->bRequest==0x07)&&(!(sup->bmRequestType&0x80))){                     //host wants to set a device descriptor
                        UART0_send_async("o",1,0);
                        //Device does not support creation of new descriptors or changing existing ones so:
                        USB_set_CTRL_stall(6);
                    }else if((sup->bRequest==0x08)&&( (sup->bmRequestType&0x80))){                     //device should return which configuration is used (bConfigurationValue)
                        UART0_send_async("b",1,0);
                        if(deviceState==USBDeviceStateAddress){
                            uint8_t not_configured_data[1]={0x00};
                            //prepare Status Stage
                            USB_initiate_recieve_ctrl_zerobyte(); //for following zerobyte from host
                            //Data Stage
                            USB_initiate_send(USB_CTRL_IN,not_configured_data,minOf(sizeof(not_configured_data),wLength),NULL); //Needs to return zero value if not configured
                        }else if(deviceState==USBDeviceStateConfigured){
                            uint8_t data[1]={USB_CONFIG_Descriptor1[5]};
                            //prepare Status Stage
                            USB_initiate_recieve_ctrl_zerobyte();
                            //Data Stage
                            USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength),NULL); //Needs to return configdesc value
                        }else{
                            USB_set_CTRL_stall(7);//error: device was not configured (undefined behavior)
                        }
                    }else if((sup->bRequest==0x09)&&(!(sup->bmRequestType&0x80))){                     //host wants to set a device to one of its configurations (bConfigurationValue)
                        if(deviceState==USBDeviceStateDefault){
                            USB_set_CTRL_stall(8);//ShouldNotHappen
                        }else{
                            if(sup->wValueLow){ //if the selected configuration is not zero
                                if(sup->wValueLow<=USB_DD_NUM_CONFGR){ //if this configuration exists (as we numbered them from 0x01 to USB_DD_NUM_CONFGR)
                                    activeConfiguration=sup->wValueLow;
                                    //TODO call user function to notify on configuration change
                                    deviceState=USBDeviceStateConfigured;
                                    USB_configured_setup_ep();
                                    //Status Stage
                                    USB_initiate_send_ctrl_zerobyte();
                                }else{
                                    USB_set_CTRL_stall(9); //host tries to select nonexistent configuration, abort
                                }
                            }else{
                                deviceState=USBDeviceStateAddress;
                                //Status Stage
                                USB_initiate_send_ctrl_zerobyte();
                            }
                        }

                    }else{//Undefined standard device request
                        USB_set_CTRL_stall(10);
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
                                USB_set_CTRL_stall(11);
                        }else{
                            uint8_t data[2]={0x00,0x00}; //Interface status is always 0x0000
                            //Status Stage (prepare beforehand) which confirms reception
                            USB_initiate_recieve_ctrl_zerobyte();
                            //Data Stage:
                            USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength),NULL);
                        }

                    }else if((sup->bRequest==0x01)&&(!(sup->bmRequestType&0x80))){ //host wants to clear a interface feature
                        USB_set_CTRL_stall(12);
                    }else if((sup->bRequest==0x03)&&(!(sup->bmRequestType&0x80))){ //host wants to set a interface feature
                        USB_set_CTRL_stall(13);
                    }else if((sup->bRequest==0x0A)&&( (sup->bmRequestType&0x80))){ //device should return alternative setting???
                        //TODO implement and switch interface based on sup->wIndexLow
                        //not yet supported, multiple configurations
                        USB_set_CTRL_stall(14);
                    }else if((sup->bRequest==0x11)&&(!(sup->bmRequestType&0x80))){ //host wants to set device interface to alternative setting
                        //no alternative interface given to host in description so should not happen
                        //TODO implement and switch interface based on sup->wIndexLow

                        USB_set_CTRL_stall(15); //Error not supported

                    }else if((sup->bRequest==0x06)&&((sup->bmRequestType&0x80))){ //host wants descriptor like hid from this interface
                        if(sup->wValueHigh==0x21){ //hid descriptor
                            //was already sent as part of configuration descriptor
                            USB_set_CTRL_stall(4);
                        }else if(sup->wValueHigh==0x22){ //report descriptor
                            UART0_send_async("n",1,0);
                            USB_initiate_recieve_ctrl_zerobyte();
                            USB_initiate_send(USB_CTRL_IN,(uint8_t*)USB_HID_REPORT_DESCRIPTOR_ARRAY[sup->wValueLow],minOf(sizeof(REPORT_Descriptor_Keyboard),wLength),NULL);
                        }else{
                            USB_set_CTRL_stall(30);
                        }
                    }else{
                        USB_set_CTRL_stall(16); //Error not supported

                    }
                }else if((sup->bmRequestType&0x1f)==0x02){ //Standard Endpoint
                    /*FromHostToDevice  Request Type                DataLength
                      0                 0x00 (get status)           2
                      1                 0x01 (clear feature)        0
                      1                 0x03 (set feature)          0
                    */

                    if      ((sup->bRequest==0x00)&&( (sup->bmRequestType&0x80))){ //endpoint should send status to host
                        if(deviceState==USBDeviceStateDefault||(deviceState==USBDeviceStateAddress&&(sup->wIndexLow&0x7f))){ //if in default state or in address and endpoint is !=zero
                            USB_set_CTRL_stall(17);
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
                                USB_set_CTRL_stall(18);
                            break;
                        }
                        //Status Stage (prepare beforehand) which confirms reception
                        USB_initiate_recieve_ctrl_zerobyte();
                        //Data Stage:
                        USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength),NULL);
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
                                    USB_set_CTRL_stall(19);
                                break;
                            }
                            USB_initiate_send_ctrl_zerobyte();
                        }else{
                            USB_set_CTRL_stall(20);
                        }
                    }else if((sup->bRequest==0x03)&&(!(sup->bmRequestType&0x80))){ //host wants to set a endpoint feature
                        if(deviceState==USBDeviceStateConfigured&&sup->wValueLow==0x00){ //ENDPOINT halt should be set
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
                                    USB_set_CTRL_stall(21);
                                break;
                            }
                            USB_initiate_send_ctrl_zerobyte();
                        }else{
                            USB_set_CTRL_stall(22);
                        }
                    }else{
                        USB_set_CTRL_stall(23); //Error not supported

                    }
                }else{
                    USB_set_CTRL_stall(24); //Error not supported

                }
            }else if((sup->bmRequestType&0x60)==0x20){ //Class request (defined by USB class in use eg. HID)
                //Class specific functions (HID)
                //reference https://www.renesas.com/eu/en/doc/products/mpumcu/apn/rl78/002/r01an0546ej0215_usb.pdf
                //UART0_send_async("?",1,0);
                if      ((sup->bRequest==0x01)&&( (sup->bmRequestType&0x80))){ //get report
                    if(sup->wValueHigh==1){         //INPUT
                        //TODO FIX!!!! and move to user functions
                        USB_initiate_send(USB_CTRL_IN,report_hid_out,8,NULL);
                        USB_initiate_recieve_ctrl_zerobyte();
                        USB_HID_get_report();
                    }else{
                        USB_set_CTRL_stall(99);
                    }
                }else if((sup->bRequest==0x02)&&( (sup->bmRequestType&0x80))){ //get idle rate
                    USB_initiate_recieve_ctrl_zerobyte();
                    uint8_t data[1]={USB_HID_IDLE_RATE[sup->wValueLow]};
                    USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength),NULL);
                }else if((sup->bRequest==0x03)&&( (sup->bmRequestType&0x80))){ //Get Protocol, requiered for boot devices
                    USB_initiate_recieve_ctrl_zerobyte();    //plan to recieve zerobyte as confirmation of next transfer
                    uint8_t data[1]={USB_HID_PROTOCOL[sup->wIndexLow]};
                    USB_initiate_send(USB_CTRL_IN,data,minOf(sizeof(data),wLength),NULL);
                }else if((sup->bRequest==0x06)&&( (sup->bmRequestType&0x80))){ //Get HID Report descriptor
                    if(sup->wValueHigh==1){ //report type input
                        UART0_send_async("ir",2,0);
                        USB_initiate_recieve_ctrl_zerobyte();
                        USB_initiate_send(USB_CTRL_IN,(uint8_t*)(USB_HID_REPORT_DESCRIPTOR_ARRAY[sup->wValueLow]),minOf(sizeof(USB_HID_REPORT_DESCRIPTOR_ARRAY[sup->wValueLow]),wLength),NULL);
                    }else if(sup->wValueHigh==3){ //report type feature
                        UART0_send_async("e2",2,0);
                        //TODO needed
                    }
                }else if((sup->bRequest==0x09)&&(!(sup->bmRequestType&0x80))){ //Set report
                    uint8_t tempdata[64]={0};       //TODO don't hardcode this size 64
                    if(sup->wValueHigh==1){  //request for input
                        if(sup->wValueLow!=0 || sup->wValueLow<=(sizeof(USB_SET_REPORT_CALBF_FOR_REP_ID)/sizeof(USB_SET_REPORT_CALBF_FOR_REP_ID[0]))){
                            USB_initiate_recieve(USB_CTRL_OUT,tempdata,sizeof(tempdata)/sizeof(tempdata[0]),USB_SET_REPORT_CALBF_FOR_REP_ID[wValue]);
                            USB_initiate_send_ctrl_zerobyte();
                        }else{
                            //report id not specified or invalid report id, either way an error
                            USB_set_CTRL_stall(25);
                        }
                    }else{
                        USB_set_CTRL_stall(26);
                    }
                }else if((sup->bRequest==0x0a)&&(!(sup->bmRequestType&0x80))){ //Set Idle
                    //TODO set timer to wait for idletime
                    //TODO if idletime is zero only reply if a key has been pressed or released (state changed)
                    //TODO fix, one can define report id's in the report descriptor which start from 1 and can be used to set individual idle times
                    if(sup->wValueLow!=0){//set idle is report specific
                        USB_HID_IDLE_RATE[(sup->wValueLow)-1]=sup->wValueHigh;
                    }else{                  //applied to all reports
                        for(int currentinterface=0;currentinterface<USB_NUM_OF_DEFINED_REPORTS;currentinterface++){
                            USB_HID_IDLE_RATE[currentinterface]=sup->wValueHigh;
                        }
                    }
                    UART0_send_async("p",1,0);
                    //TODO driver could start to request input from now on
                    USB_initiate_send_ctrl_zerobyte();
                }else if((sup->bRequest==0x0b)&&(!(sup->bmRequestType&0x80))){ //Set Protocol
                    USB_HID_PROTOCOL[sup->wValueLow]=sup->wValueHigh;
                    UART0_send_async("y",1,0);
                    USB_initiate_send_ctrl_zerobyte();
                }else{
                    USB_set_CTRL_stall(27); //Should not happen
                }
            }else if((sup->bmRequestType&0x60)==0x40){ //Vendor Request (defined by USER/VENDOR)
                USB_set_CTRL_stall(28); //Should not happen
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
        }else if(USB_ATTR&USB_ATTR_USBRST_MASK){ //Reset from host by se0
            USB_enable_controller();
            USB_enable_phy();
            UART0_send_async("r",1,0);
            for(uint32_t epnum=0;epnum<(sizeof(EP_CONFIG_ARRAY)/sizeof(EP_CONFIG_ARRAY[0]));epnum++){ //TODO check if reset is sufficient
                EP_CONFIG_ARRAY[epnum].MultiStage_Bytes_leftToSend_or_recieved=no_bytes_left;
                EP_CONFIG_ARRAY[epnum].MultiStage_Storage_ptr=0;
                USB_stop_transOrRec(epnum);
                USB_EP_TO_DATAn(epnum,0); //All controll transfers start with data0 in general, so make sure this is the case after reset
            }
            USB_INTSTS=0x80ff000f;
            deviceState=USBDeviceStateDefault;
            USB_FADDR&=0xffffff80; //careful, overlaps with usbarrtib
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
