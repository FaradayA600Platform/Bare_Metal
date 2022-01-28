
#define DMAC_CH0 0x100
#define DMAC_CH1 0x120
#define DMAC_CH2 0x140
#define DMAC_CH3 0x160
#define DMAC_CH4 0x180
#define DMAC_CH5 0x1a0
#define DMAC_CH6 0x1c0
#define DMAC_CH7 0x1e0

#define DMAC_CTRL_REG 0x00
#define DMAC_CFG_REG  0x04
#define DMAC_SAR_REG  0x08
#define DMAC_DAR_REG  0x0c
#define DMAC_LLP_REG  0x10
#define DMAC_TCNT_REG 0x14
#define DMAC_STRD_REG 0x18

#define DMAC_CTRL_EVENT 0
#define DMAC_CTRL_WSYNC 8
#define DMAC_CTRL_SEVENT 9
#define DMAC_CTRL_SEVNT 12
#define DMAC_CTRL_WEVNT 13
#define DMAC_CTRL_2DEN  14
#define DMAC_CTRL_EXPEN 15
#define DMAC_CTRL_CHEN  16
#define DMAC_CTRL_DSTC  18      //00:INCR  10:FIX
#define DMAC_CTRL_SRCC  20
#define DMAC_CTRL_DSTW  22      //000:8-bit  001:16-bit  010:32-bit  011:64-bit
#define DMAC_CTRL_SRCW  25
#define DMAC_CTRL_TCMSK 28
#define DMAC_CTRL_STCNT 29
#define DMAC_CFG_TCMSK  0
#define DMAC_CFG_ERRMSK 1
#define DMAC_CFG_ABTMSK 2
#define DMAC_CFG_SRCRS  3
#define DMAC_CFG_SRCHE  7
#define DMAC_CFG_DSTRS  9
#define DMAC_CFG_DSTHE  13
#define DMAC_CFG_LLPCNT 16
#define DMAC_CFG_GNTWIN 20
#define DMAC_CFG_CHPRI  28
///////////////////// DMA master controller registers //////////////

/*------------ GlobalRegister -----------*/
#define		INT		0x0
#define		INT_TC		0x04
#define		INT_TC_CLR	0x08
#define		INT_ERR		0x0c
#define		INT_ERR_CLR	0x10
#define		TC		0x14
#define		ERR		0x18
#define		CH_EN		0x1c
#define		CH_BUSY		0x20
#define		CSR		0x24
#define		SYNC		0x28

/*----- AHB bus1 Device Control Register Base Registers -----*/
#define		C0_DevRegBase	0x80
#define		C1_DevRegBase	0x84
#define		C2_DevRegBase	0x88
#define		C3_DevRegBase	0x8c
#define		C4_DevRegBase	0x90
#define		C5_DevRegBase	0x94
#define		C6_DevRegBase	0x98
#define		C7_DevRegBase	0x9c

/*----- AHB bus1 Device Data Base Register Base Registers -----*/
#define		C0_DevDtBase	0x40
#define		C1_DevDtBase	0x44
#define		C2_DevDtBase	0x48
#define		C3_DevDtBase	0x4c
#define		C4_DevDtBase	0x50
#define		C5_DevDtBase	0x54
#define		C6_DevDtBase	0x58
#define		C7_DevDtBase	0x5c

/*----- Channel0 Registers -----*/
#define 	C0_CSR		0x100
#define		C0_CFG		0x104
#define		C0_SrcAddr	0x108
#define		C0_DstAddr	0x10c
#define		C0_LLP		0x110
#define		C0_SIZE		0x114

/*----- Channel1 Regsiters -----*/
#define 	C1_CSR          0x120
#define 	C1_CFG          0x124
#define 	C1_SrcAddr      0x128
#define 	C1_DstAddr      0x12c
#define 	C1_LLP          0x130
#define 	C1_SIZE         0x134

/*----- Channel2 Regsiters -----*/
#define 	C2_CSR          0x140
#define 	C2_CFG          0x144
#define 	C2_SrcAddr      0x148
#define 	C2_DstAddr      0x14c
#define 	C2_LLP          0x150
#define 	C2_SIZE         0x154

/*----- Channel3 Regsiters -----*/
#define 	C3_CSR          0x160
#define 	C3_CFG          0x164
#define 	C3_SrcAddr      0x168
#define 	C3_DstAddr      0x16c
#define 	C3_LLP          0x170
#define 	C3_SIZE         0x174

/*----- Channel4 Regsiters -----*/
#define 	C4_CSR          0x180
#define 	C4_CFG          0x184
#define 	C4_SrcAddr      0x188
#define 	C4_DstAddr      0x18c
#define 	C4_LLP          0x190
#define 	C4_SIZE         0x194

/*----- Channel5 Regsiters -----*/
#define 	C5_CSR          0x1a0
#define 	C5_CFG          0x1a4
#define 	C5_SrcAddr      0x1a8
#define 	C5_DstAddr      0x1ac
#define 	C5_LLP          0x1b0
#define 	C5_SIZE         0x1b4

/*----- Channel6 Regsiters -----*/
#define 	C6_CSR          0x1c0
#define 	C6_CFG          0x1c4
#define 	C6_SrcAddr      0x1c8
#define 	C6_DstAddr      0x1cc
#define 	C6_LLP          0x1d0
#define 	C6_SIZE         0x1d4

/*----- Channel7 Regsiters -----*/
#define 	C7_CSR          0x1e0
#define 	C7_CFG          0x1e4
#define 	C7_SrcAddr      0x1e8
#define 	C7_DstAddr      0x1ec
#define 	C7_LLP          0x1f0
#define 	C7_SIZE         0x1f4

/*----- Channel Control Register(Cn_CSR) -----*/
#define 	TC_MSK		0x80000000
        	
#define		CHPRIHI		0x00c00000
#define		CHPRI2ND	0x00800000
#define		CHPRI3ND	0x00400000		
#define 	CHPRILO		0x00000000
        	
#define		PROT3		0x00200000
#define		PROT2		0x00100000
#define		PROT1		0x00080000
        	
#define		SRC_SIZE1	0x00
#define		SRC_SIZE4	0x00010000
#define		SRC_SIZE8	0x00020000
#define		SRC_SIZE16	0x00030000
#define		SRC_SIZE32	0x00040000
#define		SRC_SIZE64	0x00050000
#define		SRC_SIZE128	0x00060000
#define		SRC_SIZE256	0x00070000

#define		ABT		0x00008000
        	
#define		SRC_WIDTH8	0x00000000
#define		SRC_WIDTH16	0x00000800
#define		SRC_WIDTH32	0x00001000
        	
#define		DST_WIDTH8	0x00000000
#define		DST_WIDTH16	0x00000100
#define		DST_WIDTH32	0x00000200
        	
#define		MODE_NORMAL	0x00000000
#define		MODE_HARD	0x00000080
        	
#define		SRCAD_CTL_INC	0x00000000
#define 	SRCAD_CTL_DEC	0x00000020
#define		SRCAD_CTL_FIX	0x00000040
        	
#define		DSTAD_CTL_INC	0x00000000
#define		DSTAD_CTL_DEC	0x00000008
#define		DSTAD_CTL_FIX	0x00000010
        	
#define		SRC_SEL_M0	0x00000000
#define		SRC_SEL_M1	0x00000004
        	
#define		DST_SEL_M0	0x00000000
#define 	DST_SEL_M1	0x00000002
        	
#define		CHAN_DIS	0x00000000
#define		CHAN_EN		0x00000001

#define WRITE( addr, data ) *( (volatile unsigned int *) ( addr ) ) = data
#define READ( addr )  *( (volatile unsigned int *) ( addr ) )

#define DATA0 0x04030201
#define DATA1 0x08070605
#define	DATA2 0x0c0b0a09
#define DATA3 0x100f0e0d
#define DATA4 0x14131211
#define	DATA5 0x18171615
#define DATA6 0x1c1b1a19
#define DATA7 0x201f1e1d
#define	DATA8 0x24232221
#define	DATA9 0x28272625
#define	DATA10 0x2c2b2a29
#define DATA11 0x302f2e2d
#define	DATA12 0x34333231
#define	DATA13 0x38373635
#define	DATA14 0x3c3b3a39
#define	DATA15 0x403f3e3d
#define	DATA16 0x44434241

#define D_CACHE_BIT  2
#define I_CACHE_BIT  12
