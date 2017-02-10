/* This will work with STM32 type of microcontrollers.    *
 * The sizes of RAM and flash are specified smaller than  *
 * what most of the STM32 provide to ensure that the demo *
 * program will run on ANY STM32.                         */
MEMORY
{
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
}

SECTIONS
{
    .  = 0x0;         /* From 0x00000000 */

    .text :
    {
        _stext = .;
        *(vectors)    /* Vector table */
        *(.text)      /* Program code */
        *(.text.*)      /*  */
    } >FLASH

    .rodata :
    {
        *(.rodata)    /* Read only data */
        *(.rodata.*)    /* Read only data */
    } >FLASH

    _etext = .;

    .  = 0x20000000;  /* From 0x20000000 */
    
    .data :
    {
        _sdata = .;
        *(.data)	   /* Data memory */
        *(.data*)      /* .data* sections */

        . = ALIGN(4);
        _smeas_meta = .;
        *(.meas_meta)
        _emeas_meta = .;

        . = ALIGN(4);
        _smeas_hdr = .;
        *(.meas_hdr)
        _emeas_hdr = .;

        . = ALIGN(4);
        _smeas = .;
        *(.meas_4B)

        . = ALIGN(1);
        *(.meas)
        _emeas = .;

        _sparam = .;
        *(.param)
        _eparam = .;
        
        . = ALIGN(4);
        _sparamptr = .;
        *(.paramptr)
        _eparamptr = .;
        LONG((_eparamptr - _sparamptr) / 4)
        _nbrparam = . - 4;
    } >RAM AT > FLASH
    _edata = .;
    
    .bss :
    {
        _sbss = .;
        *(.bss)       /* Zero-filled run time allocate data memory */
    } >RAM AT > FLASH
    _ebss = .;

    .stack_seg :
    {
        _stack_bottom = .;
        KEEP(*(.stack*))
    } > RAM

    _stack_top = .;
}
