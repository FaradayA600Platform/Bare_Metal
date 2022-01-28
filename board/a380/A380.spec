Specification A380
IRQs	64	
FIQs	64	

#-----------------------------------------------------------------------------------------------------------------
# IP     Name        Ver    IRQ		E T   FIQ    E T Physical Address      Virtual Address   C B D
#-----------------------------------------------------------------------------------------------------------------
 # AXI
 CPU     CA9MP       -      -		- -   -      - - 0x40000000-0x7FFFFFFF  -                N N N
# CPU     FA626TE     -      -		- -   -      - - 0x40000000-0x7FFFFFFF  -                N N N
 ERAM    FTEMC030    -      -		L H   -      L H 0x88000000-0x880FFFFF  -                N N N
 INTC    FTINTC030   -      -		- -   -      - - 0x96800000-0x968FFFFF  -                N N N
 LCD	 FTLCDC210   -      35		L H   -      L H 0x92900000-0x929FFFFF  -                N N N
 DMAC    FTDMAC030   -      51,52,53    L H  -   L H 0x92D00000-0x92DFFFFF  -                N N N
# DMAC    FTDMAC030   -      116,115,114 L H  -   L H 0x92E00000-0x92EFFFFF  -                N N N
 DMAC    FTDMAC030   -      0,0,0   L H   -      L H 0x92E00000-0x92EFFFFF  -                N N N
 RAM     FTDMAC030   -      -       - -   -      - - 0x96000000-0x967FFFFF  -                N N N
 RAM     FTDMAC030   -      -       - -   -      - - 0x00000000-0x0000FFFF  -                N N N
 RAM     FTDMAC030   -      -       - -   -      - - 0xF8000000-0xF800FFFF  -                N N N
 DDRC    FTDDR3030   -      -		- -   -      - - 0x92F00000-0x92FFFFFF  -                N N N
 MCP	 SCALER300   -      40		L H   -      L H 0x98000000-0x980FFFFF  -                N N N
 MCP     VCAP300     -      41		L H   -      L H 0x98100000-0x981FFFFF  -                N N N
 # AHB
 DMAC    FTDMAC020   -      54		L H   -      L H 0xC0300000-0xC03FFFFF  -                N N N
 SDC	 FTSDC021    -      26		L H   -      L H 0xC0400000-0xC04FFFFF  -                N N N
 NAND	 FTNANDC024  -      23		L H   -      L H 0xC0600000-0xC06FFFFF  -                N N N
 AES	 FTAES020    -      25		L H   -      L H 0xC0700000-0xC07FFFFF  -                N N N
 USB	 FOTG210     -      29		L H   -      L H 0xC0800000-0xC08FFFFF  -                N N N
 USB	 FOTG210     -      30		L H   -      L H 0xC0900000-0xC09FFFFF  -                N N N
 SPI	 FTSPI020    -      22		L H   -      L H 0xC0A00000-0xC0AFFFFF  -                N N N
 MCP	 FTMCP100    -      42		L H   -      L H 0xC0B00000-0xC0BFFFFF  -                N N N
 MCP	 FTMCP280    -      27		L H   -      L H 0xC0D00000-0xC0DFFFFF  -                N N N
 MCP	 FTMCP300    -      28		L H   -      L H 0xC0E00000-0xC0EFFFFF  -                N N N
 # APB
 GPIO    FTGPIO010   -      16		L H   -      L H 0x90100000-0x901FFFFF  -                N N N
 IIC     FTIIC010    -      17		L H   -      L H 0x90200000-0x902FFFFF  -                N N N
 IIC     FTIIC010    -      18		L H   -      L H 0x90300000-0x903FFFFF  -                N N N
 UART    FTUART010   -      10		L H   -      L H 0x90400000-0x904FFFFF  -                N N N
 UART    FTUART010   -      11		L H   -      L H 0x90500000-0x905FFFFF  -                N N N
 UART    FTUART010   -      12		L H   -      L H 0x90600000-0x906FFFFF  -                N N N
 SSP     FTSSP010    -      19		L H   -      L H 0x90700000-0x907FFFFF  -                N N N
 SSP     FTSSP010    -      20		L H   -      L H 0x90800000-0x908FFFFF  -                N N N
 SSP     FTSSP010    -      21		L H   -      L H 0x90900000-0x909FFFFF  -                N N N
 SCU     FTSCU100    -      9		L H   -      L H 0x90A00000-0x90AFFFFF  -                N N N
 TIMER   FTTMR010    -      13,14,15 E H  -      L H 0x90B00000-0x90BFFFFF  -                N N N
# TIMER   FTTMR010    -      60,60,60 E H  -      L H 0x90B00000-0x90BFFFFF  -                N N N
 WDT     FTWDT010    -      24		L H   -      L H 0x90C00000-0x90CFFFFF  -                N N N
 KBC     FTKBC010    -      31		L H   -      L H 0x90D00000-0x90DFFFFF  -                N N N
 PMON    PMON010     -      56		L H   -      L H 0x91100000-0x911FFFFF  -                N N N
 CCU     TMSENSOR    -      -		L H   -      L H 0x91400000-0x914FFFFF  -                N N N
 ADC     WRAPPER     -      -       L H   -      L H 0x93400000-0x934FFFFF  -                N N N
 DAC     WRAPPER     -      -       L H   -      L H 0x93500000-0x935FFFFF  -                N N N
 SERDES  VSEMI       -      -		L H   -      L H 0x92600000-0x926FFFFF  -                N N N
 SERDES  VSEMI       -      -		L H   -      L H 0x92700000-0x927FFFFF  -                N N N
 SERDES  VSEMI       -      -		L H   -      L H 0x92800000-0x928FFFFF  -                N N N


