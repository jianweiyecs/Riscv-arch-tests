#include <platform.h>

OUTPUT_FORMAT(elf64-littleriscv)

ENTRY(_boot)

STACK_SIZE = 1M;

MEMORY {
    RAM (rwx)    : ORIGIN = MEM_BASE, LENGTH = MEM_SIZE    
}

SECTIONS {

	.boot : {
		*(.boot)
	} 

	.rela.dyn 0 : {
		*(.rela)
		*(.rela.*)
	}

	.text :  {
		*(.text)
		*(.text.*)
	} 

	.rodata :  {
		*(.srodata)
		*(.srodata.*)
		*(.rodata)
		*(.rodata.*)
	}

    .test_table :  {
        _test_table = .;
		KEEP(*(.test_table))
		KEEP(*(.test_table.*))
        _test_table_end = .;
	}
    _test_table_size = (_test_table_end - _test_table) / 8;
	
	.data : {
		*(.sdata)
		*(.sdata.*)
		*(.data)
		*(.data.*)
	}

	// HTIF memory needs to be located on its own page
	.tohost ALIGN(0x1000) :  {
		*(.tohost)
		*(.tohost.*)
	}
	ASSERT(SIZEOF(.tohost) <= 0x1000, ".tohost must fit in a single page")

	.bss ALIGN(0x1000) (NOLOAD) :  {
		__bss_start = .;
		*(.sbss)
		*(.sbss.*)
		*(.bss) 
		*(.bss.*)
		*(COMMON)	
		__bss_end = .;
	} 

	. = MEM_BASE + 0x1ff000;
	__stack_bottom = ALIGN(16);
	ASSERT(__bss_end <= __stack_bottom, ".bss overlaps reserved stack region")
    
    . = __stack_bottom + STACK_SIZE;
    __stack_top = .;
    _end = .;

	/DISCARD/ : {
		*(.note.GNU-stack)
	}

	.comment 0 : { *(.comment) *(.comment.*) }
	.riscv.attributes 0 : { *(.riscv.attributes) }
	.debug_info 0 : { *(.debug_info) *(.debug_info.*) }
	.debug_abbrev 0 : { *(.debug_abbrev) *(.debug_abbrev.*) }
	.debug_loclists 0 : { *(.debug_loclists) *(.debug_loclists.*) }
	.debug_aranges 0 : { *(.debug_aranges) *(.debug_aranges.*) }
	.debug_rnglists 0 : { *(.debug_rnglists) *(.debug_rnglists.*) }
	.debug_macro 0 : { *(.debug_macro) *(.debug_macro.*) }
	.debug_line 0 : { *(.debug_line) *(.debug_line.*) }
	.debug_str 0 : { *(.debug_str) *(.debug_str.*) }
	.debug_line_str 0 : { *(.debug_line_str) *(.debug_line_str.*) }
	.debug_frame 0 : { *(.debug_frame) *(.debug_frame.*) }
}
