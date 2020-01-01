	.syntax unified
	.arch armv6-m

	.cpu cortex-m0
	.section .vectorTable, "a" /* The "a" indicates that the section is allocatable and therfore needs storage space */
	         /* will be moved into the vector section of flash */
	.align	2
	.long	_StackPlaceholderEnd  /* End of Stack */
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

    .long   BOD_IRQHandler        /* Brown-out low voltage */
    .long   WDT_IRQHandler        /* Watchdog */
    .long   EINT0_IRQHandler      /* PB14 external int */
    .long   EINT1_IRQHandler      /* PB15 or PD11 external int */
    .long   GPAB_IRQHandler       /* PA or PB int */
    .long   GPCDF_IRQHandler      /* Ext PC, PD or PF int*/
    .long   PWMA_IRQHandler       /* PWM int */
    .long   Default_Handler
    .long   TMR0_IRQHandler
    .long   TMR1_IRQHandler
    .long   TMR2_IRQHandler
    .long   TMR3_IRQHandler
    .long   UART0_IRQHandler
    .long   UART1_IRQHandler
    .long   SPI0_IRQHandler
    .long   SPI1_IRQHandler
    .long   SPI2_IRQHandler
    .long   Default_Handler
    .long   I2C0_IRQHandler
    .long   I2C1_IRQHandler
    .long   Default_Handler
    .long   Default_Handler
    .long   Default_Handler
    .long   USBD_IRQHandler
    .long   PS2_IRQHandler
    .long   Default_Handler
    .long   PDMA_IRQHandler
    .long   I2S_IRQHandler
    .long   PWRWU_IRQHandler
    .long   Default_Handler
    .long   Default_Handler



	.section .text
	.thumb
	.thumb_func
	.align 2
	.global Reset_Handler
Reset_Handler:
	ldr r0, =_DataStart
	ldr r1, =_DataPlaceholderStart
	ldr r2, =_DataPlaceholderSize
	cmp r2, #0;					/*if (_DataSize == 0), then skip copy .data to .data_placeholder*/
	beq Initialize_bss
CopyData:                       /*Data from flash needs to be copied into ram*/
	subs r2, r2, #4
	ldr r4, [r0,r2]			/*Load data from address of r0 an move it in r4, also increment value of r0 by one*/
	str r4, [r1,r2]			/*Store data from r4 in address of r1 and increment r1 by one*/
	bgt CopyData	   			/*check if(_rodataSize!=0), when true continue copying*/
Initialize_bss:
	ldr r1, =_BssPlaceholderStart
	ldr r2, =_BssPlaceholderSize
	cmp r2, #0					/*if(_BssPlaceholderSize==0), then skip zeroing out bss*/
	beq InitStack
SetBssToZero:
    movs r4, #0
    subs r2, r2, #4
	str r4, [r1, r2]
	bgt SetBssToZero
InitStack:
	ldr r0, = _StackPlaceholderEnd  /* Stack extends downward, so end of section stack */
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
    def_irq_handler GPCDF_IRQHandler
    def_irq_handler PWMA_IRQHandler
    def_irq_handler TMR0_IRQHandler
    def_irq_handler TMR1_IRQHandler
    def_irq_handler TMR2_IRQHandler
    def_irq_handler TMR3_IRQHandler
    def_irq_handler UART0_IRQHandler
    def_irq_handler UART1_IRQHandler
    def_irq_handler SPI0_IRQHandler
    def_irq_handler SPI1_IRQHandler
    def_irq_handler SPI2_IRQHandler
    def_irq_handler I2C0_IRQHandler
    def_irq_handler I2C1_IRQHandler
    def_irq_handler USBD_IRQHandler
    def_irq_handler PS2_IRQHandler
    def_irq_handler PWRWU_IRQHandler
    def_irq_handler PDMA_IRQHandler
    def_irq_handler I2S_IRQHandler

    .end
