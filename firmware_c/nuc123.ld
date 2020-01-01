MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x10000
	RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x5000
}
/* Please define your heapsize and stacksize here, must be multiple of 4 bytes*/
_StackSize = 0x1000;
_HeapSize = 0x1000;

ENTRY(Reset_Handler)

SECTIONS{
        /* '.' is the location counter, it gets incremented by the corresponding segment size when we an a segment */
	. = ORIGIN(FLASH);
        /* vector table that contains the interrupt and reset function pointers */
	.vectorTab : {
		_VecStart = .;
		KEEP(*(.vectorTable)) 	 /*'*' is the wildcard for any number of characters, so all files ending on .vectorTable will be copied to this section*/
        _VecEnd = .;
	} > FLASH

        /* contains the executable code */
	.text : {
            /* PROVIDE, defines variable only if it has not been defined yet, that way we avoid multiple definition error when used with more than one .o file */
        _TextStart = .;
		KEEP(*(.text))
		_TextEnd = .;
	} > FLASH

        /* contains variables that are constant*/
	. = ALIGN(4);
	.rodata : {
		_RodataStart = .;
		KEEP(*(.rodata))
		. = ALIGN(4);
		_RodataEnd = .;
	} > FLASH

        /* contains variables that are changing and initialized to a specific value (loaded from the .data section) */
	. = ALIGN(4);
	.data : {
        _DataStart = .;
        KEEP(*(.data))
        . = ALIGN(4);
        _DataEnd = .;
	} > FLASH
    _DataSize = SIZEOF(.data);


	. = ORIGIN(RAM); 		/* set '.' to the start of RAM*/
        /* all sections here are just for calculating offsets and arranging them, the resulting image does not
           contain any ram data, therefore the 'placeholder' in the name */
        /* stack is here so if we have an stack overflow, we get an hardwarefault and not overwrite any other sections */
	.stack_placeholder : {              /* for variables in called subroutines */
        _StackPlaceholderStart = .;
		. += _StackSize;
		_StackPlaceholderEnd = .;
	}
	.data_placeholder :	/* .data is being copied to this location, */
	{
		_DataPlaceholderStart = .;
		. += SIZEOF(.data);
        _DataPlaceholderEnd = .;
	}>RAM
    _DataPlaceholderSize = SIZEOF(.data_placeholder);

    . = ALIGN(4);
	.bss_placeholder : { 				/* for all variables that are initialized to zero */
		_BssPlaceholderStart = .;
		KEEP(*(.bss))
		. = ALIGN(4);
		_BssPlaceholderEnd = .;
	}>RAM

	_BssPlaceholderSize = SIZEOF(.bss_placeholder);

	.heap_placeholder : { 				/* for dynamically allocated storage (malloc) */
		. = ALIGN(4);
		_HeapPlaceholderStart = .;
        . += _HeapSize;
        . = ALIGN(4);
		_HeapPlaceholderEnd = .;
	} >RAM
}
