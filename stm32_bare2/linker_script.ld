/*program entry point:*/
ENTRY(reset_handler)

/* Define the end of RAM and limit of stack memory */
/* (RAM starts at address 0x20000000) */
_estack = 0x20002000;		/*end of stack*/


MEMORY
{
	FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
	SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 8K
}

SECTIONS
{
	/* vector table at beginning of flash */
	.vector_table :
	{
		. = ALIGN(4);
		KEEP( *(.vector_table) )
		. = ALIGN(4);
	} >FLASH

	/* main program code */
	.text :
	{
	  . = ALIGN(4);
	  *(.text)
	  *(.text*)
	  . = ALIGN(4);
	} >FLASH

	/* The 'rodata' section contains read-only data,
 	 * constants, strings, information that won't change. */
	.rodata :
	{
	  . = ALIGN(4);
	  *(.rodata)
	  *(.rodata*)
	  . = ALIGN(4);
	} >FLASH

	_sidata = .;
	.data : AT(_sidata)
	{
	  . = ALIGN(4);
	  /* Mark start/end locations for the 'data' section. */
	  _sdata = .;
	  *(.data)
	  *(.data*)
	  _edata = .;
	  . = ALIGN(4);
	} >RAM

	/* The 'bss' section is similar to the 'data' section,
   * but its space is initialized to all 0s at the
   * start of the program. */
  	.bss :
  	{
	    . = ALIGN(4);
	    /* Also mark the start/end of the BSS section. */
	    _sbss = .;
	    *(.bss)
	    *(.bss*)
	    *(COMMON)
	    . = ALIGN(4);
	    _ebss = .;
  	} >RAM

  	/* THIS IS JUST A PLACEHOLDER to make sure we have enough free ram
  	.dynamic_allocations :
	{
	  . = ALIGN(4);
	  _ssystem_ram = .;
	  . = . + _Min_Leftover_RAM;
	  . = ALIGN(4);
	  _esystem_ram = .;
	} >RAM*/


}
