/*  This linker script tells the linker (ld) how to create and structure an ELF
    executable from the object code files (.o files) that are used as input.
    It specifies the memory locations, sizes, and characteristics of code, 
    rodata, data, and bss memory regions, as well as which sections from object
    code are put into these regions.
*/



/*  The MEMORY command tells the linker how memory is organized, defining the
    address where a region starts and its length. We define a code region
    that is readable and executable (but not writeable), in which we'll put
    the .text sections containing executable code. The rodata region is
    intended, as the name suggests, for data that is read only, usually for
    a program's constants. The data region is both readable and writeable, and
    holds a program's global variables. The bss region is similar, but is 
    initialized to all zero values when a program starts (see the start.s file
    for how this is done).
    
    Note that each region is currently defined to be 65,536 bytes long, which
    should be adequate for short embedded programs running on the Raspberry Pi.
    The code segment can thus hold 16,384 instructions, since each instruction
    is 4 bytes long. If necessary, one can adjust the lengths of sections, but
    one must make sure that the origin addresses are also adjusted so that 
    sections don't overlap.
    
    Also note that the start.s file assumes that the top of the stack is at
    0x80000 (where the code segment begins), and it "grows backwards" towards
    address 0).
*/

MEMORY 
{
	code_region (rx)  : ORIGIN =  0x80000, LENGTH = 0x10000
	rodata_region (r) : ORIGIN =  0x90000, LENGTH = 0x10000
	data_region (rw)  : ORIGIN = 0x100000, LENGTH = 0x10000
	bss_region (rw)   : ORIGIN = 0x110000, LENGTH = 0x10000
}



/*  The SECTIONS command tells the linker what sections should be created in
    the output executable (.elf) file.  */
SECTIONS
{
    /*  Create a .text section in the executable, using all the .text sections
        in the object files. These will be put into the code_region defined
        above.  This section is quadword aligned, since the beginning of the
        .text section is also the top of the stack, and the SP register (which
        is initialized in the start.s code) can only hold addresses evenly
        divisible by 16.  */
    .text : {
    	. = ALIGN(16);
    	KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*)
    } > code_region


    /*  Create a .rodata (read-only data) section in the executable, using all
        the .rodata sections in the object files, and put these into the
        rodata_region defined above.  */
    .rodata : {
    	*(.rodata .rodata.* .gnu.linkonce.r*)
    } > rodata_region
    

    /*  Create a .data section in the executable, using all the .data sections
        in the object files. These will be put into the data_region defined
        above.  */
    .data : {
    	*(.data .data.* .gnu.linkonce.d*)
    } > data_region


    /*  Create a .bss section in the executable, using all the .bss sections in
        the object files. These will be put into the bss_region defined above.
        No data or machine code is loaded into this section since it will be
        zeroed out when the program starts (in the start.s file). The
        __bss_start and __bss_end symbols record the start and end addresses
        of this section. The section is aligned on an address evenly
        divisible by 8 (doubleword aligned), so that the section can be zeroed
        out using doubleword load instruction in the start.s file.  */
    .bss (NOLOAD) : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    } > bss_region


    /*  The following sections are not included in the executable  */
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}


/*  We calculate the size (in doublewords) of the .bss section and record it in
    the __bss_size symbol.  This is used in the start.s code to zero out the
    appropriate amount of memory  */
    __bss_size = (__bss_end - __bss_start) >> 3;
    