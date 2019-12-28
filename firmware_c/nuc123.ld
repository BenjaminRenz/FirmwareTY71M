MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x10000
	RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 0x5000
}
/* Please define your heapsize and stacksize here, must be multiple of 4 bytes*/
_StackSize = 0x400;
_HeapSize = 0x400;

SECTIONS{
        /* '.' is the location counter, it gets incremented by the corresponding segment size when we an a segment */
	. = ORIGIN(FLASH);
        /* vector table that contains the interrupt and reset function pointers */
	.vectorTable : {
		KEEP(*(.vectorTable)) 	 /*'*' is the wildcard for any number of characters, so all files ending on .vectorTable will be copied to this section*/
	}> FLASH

        /* contains the executable code */
	.text : {
            /* PROVIDE, defines variable only if it has not been defined yet, that way we avoid multiple definition error when used with more than one .o file */
        PROVIDE(_TextStart = .);
		KEEP(*(.text))
		PROVIDE(_TextEnd = .);
	}> FLASH

        /* contains variables that are constant*/
	.rodata : {
		PROVIDE(_RodataStart = .);
		KEEP(*(.rodata))
		PROVIDE(_RodataEnd = .);
	}> FLASH

        /* contains variables that are changing and initialized to a specific value (loaded from the .data section) */
	.data : {
        PROVIDE(_DataStart = .);
        KEEP(*(.data))
        PROVIDE(_DataEnd = .);
	} > FLASH
    PROVIDE(_DataSize = SIZEOF(.data));


	. = ORIGIN(RAM); 		/* set '.' to the start of RAM*/
        /* all sections here are just for calculating offsets and arranging them, the resulting image does not
           contain any ram data, therefore the 'placeholder' in the name */
	.data_placeholder :	/* .data is being copied to this location, */
	{
		PROVIDE(_DataPlaceholderStart = .);
		. += SIZEOF(.data);
        PROVIDE(_DataPlaceholderEnd = .);
	}>RAM
    PROVIDE(_DataPlaceholderSize = SIZEOF(.data_placeholder));

	.bss_placeholder : { 				/* for all variables that are initialized to zero */
		PROVIDE(_BssPlaceholderStart = .);
		KEEP(*(.bss))
		PROVIDE(_BssPlaceholderEnd = .);
	}>RAM

	PROVIDE(_BssPlaceholderSize = SIZEOF(.bss_placeholder));

	.heap_placeholder : { 				/* for dynamically allocated storage (malloc) */
		. = ALIGN(4);
		PROVIDE(_HeapPlaceholderStart = .);
        . += _HeapSize;
		PROVIDE(_HeapPlaceholderEnd = .);
	} >RAM

	/* seems to be ineffective? */
    _StackPlaceholderEnd=ORIGIN(RAM)+LENGTH(RAM)-_StackSize;
	.stack_placeholder _StackPlaceholderEnd: {              /* for variables in called subroutines */
        /* note inverted, stack extend downwards */
		. += _StackSize;
		PROVIDE(_StackPlaceholderStart = .);
	}>RAM




	ASSERT(_StackPlaceholderEnd >= _HeapPlaceholderEnd, "Linker error: Stack overflowed into heap or vice versa!")

}
