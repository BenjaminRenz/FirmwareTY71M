MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x10000
	RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x5000
}
/* Please define your heapsize and stacksize here*/
__StackSize = 0x00003000;
__HeapSize = 0x00000800;

SECTIONS
{
	/* '.' is the location counter, it gets incremented by the corresponding segment size when we an a segment */
	. = ORIGIN(FLASH); 		
	/*.vectorTable : {		 vector table that contains the interrupt function pointers 
		KEEP(*(.vectorTable)) 	 '*' is the wildcard for any number of characters, so all files ending on .vectorTable will be copied to this section
	} > FLASH
	*/
	 				/*contains the executable code*/
	.text : {
		KEEP(*(.text))
		KEEP(*(.eh_frame*))
	} > FLASH
	
	.ARM.exidx :	/* For debugging e.g.: to unwind the stack to see which function was called by which other function*/	
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	
	.rodata : 				/*after startup this section is copied into ram. It contains variables that need to be initialized to something else than 0*/
	{
		__RodataStart = .;
		KEEP(*(.rodata))
		__RodataEnd = .;
	} > FLASH
	
	__RodataSize = __RodataEnd - __RodataStart;
	
	/*. = ORIGIN(RAM); 		 set '.' to the start of RAM*/
	
	.data : AT(__RodataEnd)	/*.rodata is beeing copied to this location, */
	{
		__dataStart = .;
		KEEP(*(.data))
	} > RAM
	
	.bss : { 				/*for all variables that are initialized to zero*/
		__BssStart = .;
		*(.bss)
		__BssEnd = .;
	} > RAM
	
	__bssSize = __bssEnd - __bssStart;
	
	_heap_low = .;
	.heap(COPY) : { 				/*for dynamically allocated storage (malloc)*/
		__HeapStart = .;
		KEEP(*(.heap))
		__HeapEnd = __HeapStart + __HeapSize;
		. = __HeapEnd;
	} > RAM
	_heap_top = .;
	__StackStart = ORIGIN(RAM) + LENGTH(RAM);		/*Because the stack starts at the end of ram Start and End are 'inverted'*/
	__StackEnd = __StackStart - __StackSize;
	.stack(COPY) : 				/*for data thats is beeing backuped prior to a function call or temporary variables inside a function*/
	{
		. = __StackEnd;
		KEEP(*(.stack))
		. = __StackStart;
	} > RAM
	
	PROVIDE(_HeapEnd = __HeapEnd);
	
	ASSERT(__StackEnd >= __HeapEnd, "Linker error: Stack overflowed into heap or vice versa!")
	
}