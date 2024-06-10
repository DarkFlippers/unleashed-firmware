ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_stack_end = 0x20030000;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_stack_size = 0x1000; /* required amount of stack */

MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM1  (xrw) : ORIGIN = 0x20000008, LENGTH = 0x2FFF8
    RAM2A (xrw) : ORIGIN = 0x20030000, LENGTH = 10K
    RAM2B (xrw) : ORIGIN = 0x20038000, LENGTH = 10K
}

SECTIONS {
    /* The startup code goes first into FLASH */
    .isr_vector : {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Startup code */
        . = ALIGN(4);
    } >FLASH

    /* The program code and other data goes into FLASH */
    .text : {
        . = ALIGN(4);
        *lib*.a:*(.text .text.*) /* code from libraries before apps */
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
        _etext = .;        /* define a global symbols at end of code */
    } >FLASH

    /* Constant data goes into FLASH */
    .rodata : {
        . = ALIGN(4);
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
    } >FLASH

    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    .preinit_array : {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH
    .init_array : {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH
    .fini_array : {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    /* used by the startup to initialize data */
    _sidata = LOADADDR(.data);

    /* Initialized data sections goes into RAM, load LMA copy after code */
    .data : {
        . = ALIGN(4);
        _sdata = .;        /* create a global symbol at data start */
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        *(*_DRIVER_CONTEXT)

        . = ALIGN(4);
        _edata = .;        /* define a global symbol at data end */
    } >RAM1 AT> FLASH

    /* Uninitialized data section */
    . = ALIGN(4);
    .bss : {
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;         /* define a global symbol at bss start */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
        __bss_end__ = _ebss;
    } >RAM1

    /* User_heap_stack section, used to check that there is enough RAM left */
    /* DSECT: https://software-dl.ti.com/ccs/esd/documents/sdto_cgt_linker_special_section_types.html */
    ._user_heap_stack(DSECT) : {
        . = ALIGN(8);
        __heap_start__ = .;
        . = ORIGIN(RAM1) + LENGTH(RAM1) - _stack_size;
        __heap_end__ = .;
        . = . + _stack_size;
        . = ALIGN(8);
    } >RAM1

    /* Free Flash space, that can be used for internal storage */
    .free_flash(DSECT) : {
        __free_flash_start__ = .;
        . = ORIGIN(FLASH) + LENGTH(FLASH);
    } >FLASH

    /* Remove information from the standard libraries */
    /DISCARD/ : {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0       : { *(.ARM.attributes) }
    ._sram2a_start          : { . = ALIGN(4); __sram2a_start__ = .; } >RAM2A
    MAPPING_TABLE (NOLOAD)  : { *(MAPPING_TABLE) } >RAM2A
    MB_MEM1 (NOLOAD)        : { *(MB_MEM1) } >RAM2A
    MB_MEM2 (NOLOAD)        : { _sMB_MEM2 = . ; *(MB_MEM2) ; _eMB_MEM2 = . ; } >RAM2A
    ._sram2a_free           : { . = ALIGN(4); __sram2a_free__ = .; } >RAM2A
    ._sram2b_start          : { . = ALIGN(4); __sram2b_start__ = .; } >RAM2B
}
