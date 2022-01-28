OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

SECTIONS
{
    . = 0x0;

    .text :
    {
    __code_start = . ;
    board/start_64.o	(.text)
	*(.text)
	*(.rodata) 
    }	
    
    . = ALIGN(16);	
    
    _data_start = .;         
    
    .data : 
    {
        _data_s = .;
	*(.data) 
	*(.data.*) 
        _data_e = .;
    }
    
    . = ALIGN(16);
    
	__bss_start = .;
	__bss_start__ = .;
	
    .bss : 
    {
        _bss_s = .;
	*(.bss) 
	*(.bss.*) 
        _bss_e = .;
    }    
	_bss_end__ = . ; __bss_end__ = . ; __end__ = . ;    
    . = ALIGN(16);    
		
    _end = .;
}