#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED

#define USB_BA 0x40060000
#define USB_INTEN (*((uint32_t*)(USB_BA+0x000)))
#define USB_INTSTS (*((uint32_t*)(USB_BA+0x004)))
#define USB_FLDET (*((uint32_t*)(USB_BA+0x014)))

#define USB_is_attached_to_host() (USB_FLDET&0x00000001)

static __inline void USB_start_reset(){
    IPRSTC2|=(1<<27);
}
static __inline void USB_end_reset(){ //only call this after ready with irq setup?
    IPRSTC2&=~(1<<27); //only call this after buffer segmentation is complete (BUFSEG0)
}
static __inline void USB_init(){
    USB_start_reset();
    USB_INTEN=0x00000000;

    USB_end_reset();
}
void URBD_IRQHandler(void){
    //Read EPSTS (USB_EPSTS[31:8])+EPEVT7~0 (USB_INTSTS[23:16]) to find out state and endpoint
    //(?) read USB_ATTR to aknowledge bus events
    USB_INTSTS
}

#endif // USB_H_INCLUDED
