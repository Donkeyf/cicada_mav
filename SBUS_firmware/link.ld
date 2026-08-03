ENTRY(_reset);
MEMORY {
  flash(rx)  : ORIGIN = 0x08000000, LENGTH = 2048k
  dtcm_ram(rwx) : ORIGIN = 0x20000000, LENGTH = 128k  /* dtcm RAM  */
  sram_d1(rwx) : ORIGIN = 0x24000000, LENGTH = 512k  /* axi SRAM  */
  sram_d2(rwx) : ORIGIN = 0x30000000, LENGTH = 288k  /* sram 1/2/3  */
  sram_d3(rwx) : ORIGIN = 0x38000000, LENGTH = 64k  /* sram 4  */ 
  sram_d4(rwx) : ORIGIN = 0x38800000, LENGTH = 4k  /* backup sram  */ 
}
_estack     = ORIGIN(dtcm_ram) + LENGTH(dtcm_ram);    /* stack points to end of data tightly coupled memory */

SECTIONS {
  .vectors  : { KEEP(*(.vectors)) }   > flash
  .text     : { *(.text*) }           > flash
  .rodata   : { *(.rodata*) }         > flash

  .data : {
    _sdata = .;   /* .data section start */
    *(.first_data)
    *(.data SORT(.data.*))
    _edata = .;  /* .data section end */
  } > sram_d1 AT > flash  
  _sidata = LOADADDR(.data);

  .bss : {
    _sbss = .;              /* .bss section start */
    *(.bss SORT(.bss.*) COMMON)
    _ebss = .;              /* .bss section end */
  } > sram_d1

  . = ALIGN(8);
  _end = .;     /* for cmsis_gcc.h  */
}
