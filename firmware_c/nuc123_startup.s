	.syntax unified
	.arch armv6-m
	
	.cpu cortex-m0
	
	
	.section .vectorTable
	.align	2
	.long	__StackStart          /* End of Stack */
	.long	Reset_Handler         /* Reset Handler, this function is called when mcu starts */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved*/
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_Handler           /* SVCall Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	PendSV_Handler        /* PendSV Handler */
	.long	SysTick_Handler       /* SysTick Handler */

    .long   BOD_IRQHandler
    .long   WDT_IRQHandler
    .long   EINT0_IRQHandler
    .long   EINT1_IRQHandler
    .long   GPAB_IRQHandler
    .long   GPCD_IRQHandler
    .long   PWMA_IRQHandler
    .long   Default_Handler
    .long   TMR0_IRQHandler
    .long   TMR1_IRQHandler
    .long   TMR2_IRQHandler
    .long   TMR3_IRQHandler
    .long   UART0_IRQHandler
    .long   UART1_IRQHandler
    .long   SPI0_IRQHandler
    .long   SPI1_IRQHandler
    .long   Default_Handler
    .long   Default_Handler
    .long   Default_Handler
    .long   I2C1_IRQHandler
    .long   Default_Handler
    .long   Default_Handler
    .long   Default_Handler
    .long   USBD_IRQHandler
    .long   PS2_IRQHandler
    .long   Default_Handler
    .long   Default_Handler
    .long   Default_Handler
    .long   PWRWU_IRQHandler
    .long   Default_Handler
    .long   Default_Handler
    .long   RTC_IRQHandler


 
	.text
	.thumb
	.thumb_func
	.align 2
Reset_Handler: 
	ldr r0, =__rodataStart
	ldr r1, =__dataStart
	ldr r2, =__rodataSize
	cmp r2, #0;					/*if (__rodataSize == 0), then skip copy __rodata to _data*/
	beq Initialize_bss
CopyData:
	ldrb r4, [r0,#1]			/*Load data from address of r0 an move it in r4, also increment value of r0 by one*/
	strb r4, [r1,#1]			/*Store data from r4 in address of r1 and increment r1 by one*/
	subs r2, r2, #1   			/*__rodataSize=__rodataSize-1*/
	bne CopyData;	   			/*check if(__rodataSize!=0), when true continue copying*/
Initialize_bss:
	ldr r0, =__bssStart
	ldr r1, =__bssSize
	cmp r1, #0;					/*if(__bssSize==0), then skip zeroing out bss*/
SetBssToZero:
	strb r4, [r0,#1]
	subs r1, r1, #1
	bne SetBssToZero
InitStack:
	ldr r0, =__StackStart
	mov sp, r0
	bl SystemInit
	bl main
	
	
	
	
	
	
	.align  1
    .thumb_func
    .weak   Default_Handler
    .type   Default_Handler, % function

Default_Handler:
    b   .
    .size   Default_Handler, . - Default_Handler

    /*    Macro to define default handlers. Default handler
     *    will be weak symbol and just dead loops. They can be
     *    overwritten by other handlers */

    .macro  def_irq_handler handler_name
    .weak   \handler_name
    .set    \handler_name, Default_Handler
    .endm

    def_irq_handler NMI_Handler
    def_irq_handler HardFault_Handler
    def_irq_handler SVC_Handler
    def_irq_handler PendSV_Handler
    def_irq_handler SysTick_Handler


    def_irq_handler BOD_IRQHandler
    def_irq_handler WDT_IRQHandler
    def_irq_handler EINT0_IRQHandler
    def_irq_handler EINT1_IRQHandler
    def_irq_handler GPAB_IRQHandler
    def_irq_handler GPCD_IRQHandler
    def_irq_handler PWMA_IRQHandler
    def_irq_handler TMR0_IRQHandler
    def_irq_handler TMR1_IRQHandler
    def_irq_handler TMR2_IRQHandler
    def_irq_handler TMR3_IRQHandler
    def_irq_handler UART0_IRQHandler
    def_irq_handler UART1_IRQHandler
    def_irq_handler SPI0_IRQHandler
    def_irq_handler SPI1_IRQHandler
    def_irq_handler I2C1_IRQHandler
    def_irq_handler USBD_IRQHandler
    def_irq_handler PS2_IRQHandler
    def_irq_handler PWRWU_IRQHandler
    def_irq_handler RTC_IRQHandler


    .end