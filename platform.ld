OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
/*ENTRY(_start)*/
ENTRY(SelfBoot)
/*STARTUP("board/start.o")*/

SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text	   :
	{
		__code_start = . ; /*ycmo*/
		board/start.o	(.text)
		*(.text)
	}
	. = ALIGN(4);
	.rodata : { *(.rodata) *(.rodata.*) }

	__exidx_start = .;
	.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
	__exidx_end = .;

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	.got : { *(.got) }

	. = ALIGN(4);
	__bss_start = .;
	__bss_start__ = .;
	.bss : { *(.bss) }
	_bss_end__ = . ; __bss_end__ = . ; __end__ = . ;
	_end = .;
	PROVIDE (end = .);
}
