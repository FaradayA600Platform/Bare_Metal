

.global     cp15_write_c1
.global     cp15_write_ACTLR

.text 
.align 0
.code 32 

@//cp15_write_c1(unsigned int value)
cp15_write_c1:
    mcr p15,0,a1,c1,c0,0
    mov pc,lr

@//cp15_write_ACTLR(unsigned int value)
cp15_write_ACTLR:
    mcr p15,0,a1,c1,c0,1
    mov pc,lr





	
.data
.align


.end
