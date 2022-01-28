#include "SoFlexible.h"
#include "serial.h"

#define PCIE_CTR_0_BASE     0x14000000
#define PCIE_PHY_0_BASE     0x14100000
#define PCIE_PHY_1_BASE     0x14400000
#define PCIE_CTR_1_BASE     0x14500000
#define PCIE_0_DBI                0xa800000
#define PCIE_1_DBI                0xac00000

typedef enum {INIT,ASSERT,DEASSERT}flag_status;

//TEST_MSI=0 , TEST_MSI need two A500 EVB to be EP & RC , 
//TEST_MSI==0, PCIE access test on between pcie0 and pcie1 which belong one A500 EVB
#define TEST_MSI  1     

flag_status pcie_irq_status=INIT;
void fLib_PCIEIntHandler()
{
	printf("recv pcie intr\n");
	pcie_irq_status = ASSERT;
	vLib_LeWrite32(0x0a800830	, 0x00000001);
}

void PCIE_QC_Main(void)
{
       int var, src_addr, dst_addr, loop;

	unsigned int pcie0_irq=104;
	pcie_irq_status = INIT;
	   
	printf("==Start the PCIE_QC test==\n");
       //set PCIE0 as RC
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8358, 0x00000008);
       //set PCIE1 as EP
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8364, 0x00000000);

       //enable PCIE aclk/pclk and reset release
/*
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8118, 0x000000C0);
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8124, 0x00000006);
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8128, 0x00000400);
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8130, 0x0000C37F);
*/

       //Set 5G PLL AAC CUR bit[7:2]
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x14));
       var &= 0xffffff03;
       var |= 0x23;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x14, var);

       //Set 8G PLL AAC CUR bit[23:18]
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x14));
       var &= 0xFF03FFFFF;
       var |= 0x00100000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x14, var);

       //5G PLL LP DIV control, bit[24:20] 
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x81c));
       var &= 0xE00FFFFF;
       var |= 0x01900000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x81c, var);

       //5G PLL LP DIV select from REG
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x824));
       var &= 0xFEFFFFFF;
       var |= 0x01000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x824, var);

       //LDO ref voltage from BG and Chopper enabled 
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x804));
       var &= 0xFF9FFFFF;
       var |= 0x00600000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x804, var);

       //5G PLL POST_DIV [5:0]=6d'32 0x0808[25:16] = 10'h20
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x808));
       var &= 0xFC00FFFF;
       var |= 0x00200000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x808, var);

       //8G PLL POST_DIV [5:0]=6d'32 0x0810[9:0] = 10'h20
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x810));
       var &= 0xFFFFFC00;
       var |= 0x00000020;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x810, var);

//---------------------------------------------------------------------------
// TXRX_L0

       //Offset 16'h8850[4] =  1'h1   - RX_CDR_DIV64_SEL
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4850));
       var &= 0xFFFFFFEF;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4850, var);

       //SET TX_FFE_POST1 to 4'h6
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4820));
       var &= 0xFFFFFFF0;
       var |= 0x00000006;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4820, var);

       //Set DLPF TC initial value
       //Offset 16'h80F8[17:8] =  9'd511
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f8));
       var &= 0xFFFC00FF;
       var |= 0x0001FF00;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f8, var);

       //Set DLPF FC initial value
       //Offset 16'h80F8[6:0] =  7'd63
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f8));
       var &= 0xFFFFFF80;
       var |= 0x0000003F;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f8, var);

       //Start of new setting for RX_CLK debug
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40F0));
       var &= 0xF800FC00;
       var |= 0x01000080;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40F0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40F4));
       var &= 0xFFFE00C0;
       var |= 0x00010030;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40F4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4854));
       var &= 0xFFFC3FFF;
       var |= 0x00028000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4854, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40E0));
       var &= 0x7FFFFFFF;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40E0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f8));
       var &= 0xFFFC0080;
       var |= 0x00020040;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40d0));
       var &= 0xFFFFFFFF;
       var |= 0x00020000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40d0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4028));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4028, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x402c));
       var &= 0xFFFFDFDF;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x402c, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4020));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4020, var);

       //OFC_EOM setting
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4010));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4010, var);

       //OFC_ERR setting
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4018));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4018, var);

       //AGC setting
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a0));
       var &= 0xBFF7FFFF;
       var |= 0x40080000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a0, var);

       //CTLE setting
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40ac));
       var &= 0xFF3FFFFF;
       var |= 0x00C00000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40ac, var);

       //DFE setting
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a4));
       var &= 0xF83FFFFF;
       var |= 0x07C00000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a4, var);

       //IIR setting
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a4));
       var &= 0xF7FFFFFF;
       var |= 0x08000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a4, var);

       //REFP and AGC
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a0));
       var &= 0xFFFFFC00;
       var |= 0x0000030A;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a0, var);

       //CTLE
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a8));
       var &= 0xF03FFFFF;
       var |= 0x09000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a8, var);

       //PK and VGA
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x402c));
       var &= 0xFFFFE0E0;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x402c, var);

       //Select TXRX_L1 CSR for APB write
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE));
       var &= 0xFFFFFFCF;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4, var);

       //TXRX_L1
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4850));
       var &= 0xFFFFFFEF;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4850, var);

       //SET TX_FFE_POST1 to 4'h6
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4820));
       var &= 0xFFFFFFF0;
       var |= 0x00000006;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4820, var);

       //Set DLPF TC initial value
       //Offset 16'h80F8[17:8] =  9'd511
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f8));
       var &= 0xFFFC00FF;
       var |= 0x0001FF00;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f8, var);

       //Set DLPF FC initial value
       //Offset 16'h80F8[6:0] =  7'd63
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f8));
       var &= 0xFFFFFF80;
       var |= 0x0000003F;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f8, var);

       //Start of new setting for RX_CLK debug
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f0));
       var &= 0xF800FC00;
       var |= 0x01000080;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f4));
       var &= 0xFFFE00C0;
       var |= 0x00010030;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4854));
       var &= 0xFFFC3FFF;
       var |= 0x00028000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4854, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40e0));
       var &= 0x7FFFFFFF;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40e0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40f8));
       var &= 0xFFFC0080;
       var |= 0x00020040;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40D0));
       var &= 0xFFFFFFFF;
       var |= 0x00020000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40D0, var);

       //OFC_DATA setting start
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4028));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4028, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x402c));
       var &= 0xFFFFDFDF;
       var |= 0x00002020;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x402c, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4020));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4020, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4010));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4010, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x4018));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x4018, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a0));
       var &= 0xBFF7FFFF;
       var |= 0x40080000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40ac));
       var &= 0xFF3FFFFF;
       var |= 0x00C00000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40ac, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a4));
       var &= 0xF83FFFFF;
       var |= 0x07C00000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a4));
       var &= 0xF7FFFFFF;
       var |= 0x08000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a0));
       var &= 0xFFFFFC00;
       var |= 0x0000030A;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a8));
       var &= 0xF03FFFFF;
       var |= 0x09000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x402C));
       var &= 0xFFFFE0E0;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x402C, var);

//---------------------------------------------------------------------------
       //DPMA FPGA
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x8000));
       var &= 0xFFFFFFEE;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x8000, var);

//---------------------------------------------------------------------------
       //Prog L0 Control Register
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0xc110));
       var |= 0x00000540;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0xc110, var);

//---------------------------------------------------------------------------
       //Prog L1 Control Register
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0xc120));
       var |= 0x00000540;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0xc120, var);

printf("==Start Programming PCIE_PHY1==\n");

//---------------------------------------------------------------------------
       //Programming PCIE_PHY1
       //Set 5G PLL AAC CUR bit[7:2]
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x14));
       var &= 0xFFFFFF03;
       var |= 0x00000023;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x14, var);

       //Set 8G PLL AAC CUR bit[23:18]
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x14));
       var &= 0xFF03FFFF;
       var |= 0x00100000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x14, var);

       //5G PLL LP DIV control, bit[24:20] 
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x81c));
       var &= 0xE00FFFFF;
       var |= 0x01900000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x81c, var);

       //5G PLL LP DIV select from REG
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x824));
       var &= 0xFEFFFFFF;
       var |= 0x01000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x824, var);

       //LDO ref voltage from BG and Chopper enabled 
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x804));
       var &= 0xFF9FFFFF;
       var |= 0x00600000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x804, var);

       //5G PLL POST_DIV [5:0]=6d'32 0x0808[25:16] = 10'h20
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x808));
       var &= 0xFC00FFFF;
       var |= 0x00200000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x808, var);

       //8G PLL POST_DIV [5:0]=6d'32 0x0810[9:0] = 10'h20
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x810));
       var &= 0xFFFFFC00;
       var |= 0x00000020;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x810, var);

//---------------------------------------------------------------------------
       //TXRX_L0
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4850));
       var &= 0xFFFFFFEF;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4850, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4820));
       var &= 0xFFFFFFF0;
       var |= 0x00000006;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4820, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f8));
       var &= 0xFFFC00FF;
       var |= 0x0001FF00;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f8));
       var &= 0xFFFFFF80;
       var |= 0x0000003F;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f0));
       var &= 0xF800FC00;
       var |= 0x01000080;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f4));
       var &= 0xFFFE00C0;
       var |= 0x00010030;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4854));
       var &= 0xFFFC3FFF;
       var |= 0x00028000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4854, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40e0));
       var &= 0x7FFFFFFF;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40e0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f8));
       var &= 0xFFFC0080;
       var |= 0x00020040;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40d0));
       var &= 0xFFFFFFFF;
       var |= 0x00020000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40d0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4028));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4028, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x402c));
       var &= 0xFFFFDFDF;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x402c, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4020));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4020, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4010));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4010, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4018));
       var &= 0xFFFFFFFE;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4018, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a0));
       var &= 0xBFF7FFFF;
       var |= 0x40080000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40ac));
       var &= 0xFF3FFFFF;
       var |= 0x00C00000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40ac, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a4));
       var &= 0xF83FFFFF;
       var |= 0x07C00000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a4));
       var &= 0xF7FFFFFF;
       var |= 0x08000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a0));
       var &= 0xFFFFFC00;
       var |= 0x0000030A;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a8));
       var &= 0xF03FFFFF;
       var |= 0x09000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x402c));
       var &= 0xFFFFE0E0;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x402c, var);

//---------------------------------------------------------------------------
       //Select TXRX_L1 CSR for APB write
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE));
       var &= 0xFFFFFFCF;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4, var);
       

//---------------------------------------------------------------------------
       //TXRX_L1
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4850));
       var &= 0xFFFFFFEF;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4850, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4820));
       var &= 0xFFFFFFF0;
       var |= 0x00000006;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4820, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f8));
       var &= 0xFFFC00FF;
       var |= 0x0001FF00;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f8));
       var &= 0xFFFFFF80;
       var |= 0x0000003F;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f8, var);
       
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f0));
       var &= 0xF800FC00;
       var |= 0x01000080;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f0, var);
       
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f4));
       var &= 0xFFFE00C0;
       var |= 0x00010030;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4854));
       var &= 0xFFFC3FFF;
       var |= 0x00028000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4854, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40e0));
       var &= 0x7FFFFFFF;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40e0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40f8));
       var &= 0xFFFC0080;
       var |= 0x00020040;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40f8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40d0));
       var &= 0xFFFFFFFF;
       var |= 0x00020000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40d0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4028));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4028, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x402c));
       var &= 0xFFFFDFDF;
       var |= 0x00002020;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x402c, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4020));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4020, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4010));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4010, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x4018));
       var &= 0xFFFFFFFE;
       var |= 0x00000001;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x4018, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a0));
       var &= 0xBFF7FFFF;
       var |= 0x40080000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40ac));
       var &= 0xFF3FFFFF;
       var |= 0x00C00000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40ac, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a4));
       var &= 0xF83FFFFF;
       var |= 0x07C00000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a4, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a4));
       var &= 0xF7FFFFFF;
       var |= 0x08000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a4, var);
       
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a0));
       var &= 0xFFFFFC00;
       var |= 0x0000030A;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a0, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a8));
       var &= 0xF03FFFFF;
       var |= 0x09000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a8, var);

       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x402c));
       var &= 0xFFFFE0E0;
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x402c, var);

//---------------------------------------------------------------------------
       //DPMA FPGA
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x8000));
       var &= 0xFFFFFFEE;
       var |= 0x00000010;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x8000, var);

//---------------------------------------------------------------------------
       //PCS
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0xc110));
       var |= 0x00000540;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0xc110, var);

//---------------------------------------------------------------------------
       //Prog L1 Control Register
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0xc120));
       var |= 0x00000540;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0xc120, var);

printf("==Start EP DBI Programing==\n");
//-- EP DBI Programing 		-----------------------------
//   1) Write to LINK_CONTROL2_LINK_STATUS2_REG . PCIE_CAP_TARGET_LINK_SPEED in the local device
//        Write 0x0003 to Address 0xa0
       vLib_LeWrite32(PCIE_1_DBI + 0xa0, 0x00000002);   //Gen2


//-- RC DBI Programing 		-----------------------------//
//  Deassert GEN2_CTRL_OFF . DIRECT_SPEED_CHANGE in the local device
//      Write 0x0000 to Address 0x80e
       var = u32Lib_LeRead32((INT8U * )(PCIE_0_DBI + 0x80c));
       var |= 0x00000000;
       vLib_LeWrite32(PCIE_0_DBI + 0x80c, var);  
//  Assert GEN2_CTRL_OFF . DIRECT_SPEED_CHANGE in the local device
//       Write 0x0002 to Address 0x80e
       var = u32Lib_LeRead32((INT8U * )(PCIE_0_DBI + 0x80c));
       var |= 0x00020000;
       vLib_LeWrite32(PCIE_0_DBI + 0x80c, var);  

printf("==Start RC outbound CFG iATU==\n");
//---------------------------------------------------------------------------
//  1.RC outbound CFG iATU (Use Region 0)
//  use RC dbi slave port
       vLib_LeWrite32(PCIE_0_DBI + 0x300008, 0x40000000);  //set the Original Lower Base Address
       vLib_LeWrite32(PCIE_0_DBI + 0x30000c, 0x00000000);  //set the Original Upper Base Address
       vLib_LeWrite32(PCIE_0_DBI + 0x300010, 0x4200ffff);  //set the Limit Address
       vLib_LeWrite32(PCIE_0_DBI + 0x300014, 0x00000000);  //set the Lower Target Address (0x01000000)
       vLib_LeWrite32(PCIE_0_DBI + 0x300018, 0x00000000);  //set the Upper Target Address (0x00000000)
       vLib_LeWrite32(PCIE_0_DBI + 0x300000, 0x00000004);  //define the type of the region to be CFG
       vLib_LeWrite32(PCIE_0_DBI + 0x300004, 0x80000000);  //enable the region

//  2.RC outbound MEM iATU (Use Region 1)
//  use RC dbi slave port
       vLib_LeWrite32(PCIE_0_DBI + 0x300208, 0x50000000);  //set the Original Lower Base Address
       vLib_LeWrite32(PCIE_0_DBI + 0x30020c, 0x00000000);  //set the Original Upper Base Address
       vLib_LeWrite32(PCIE_0_DBI + 0x300210, 0x500fffff);  //set the Limit Address
       vLib_LeWrite32(PCIE_0_DBI + 0x300214, 0x18000000);  //set the Lower Target Address (0x01000000)
       vLib_LeWrite32(PCIE_0_DBI + 0x300218, 0x00000000);  //set the Upper Target Address (0x00000000)
       vLib_LeWrite32(PCIE_0_DBI + 0x300200, 0x00000000);  //define the type of the region to be MEM
       vLib_LeWrite32(PCIE_0_DBI + 0x300204, 0x80000000);  //enable the region

//  3.RC inbound MEM iATU (Use Region 0)
//  use RC dbi slave port
       vLib_LeWrite32(PCIE_0_DBI + 0x300108, 0x0a100000);  //set the Original Lower Base Address
       vLib_LeWrite32(PCIE_0_DBI + 0x30010c, 0x00000000);  //set the Original Upper Base Address
       vLib_LeWrite32(PCIE_0_DBI + 0x300110, 0x0a1fffff);  //set the Limit Address
       vLib_LeWrite32(PCIE_0_DBI + 0x300114, 0x0a100000);  //set the Lower Target Address (0x01000000)
       vLib_LeWrite32(PCIE_0_DBI + 0x300118, 0x00000000);  //set the Upper Target Address (0x00000000)
       vLib_LeWrite32(PCIE_0_DBI + 0x300100, 0x00000000);  //define the type of the region to be MEM
       vLib_LeWrite32(PCIE_0_DBI + 0x300104, 0x80000000);  //enable the region

//-- set ICAL_START -----------------------------
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8358, 0x00000009);   //set ICAL_START_PCIE0 = 1 
       vLib_LeWrite32(SCU_FTSCU100_PA_BASE + 0x8364, 0x00000001);   //set ICAL_START_PCIE1 = 1

printf("==Start PCIEPHY0==\n");
// PCIEPHY0 
// TXRX Reg0x00A8 bit[27:22] = 6'h3F
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_0_BASE + 0x40a8));
       var &= 0xF03FFFFF;
       var |= 0x0FC00000;
       vLib_LeWrite32(PCIE_PHY_0_BASE + 0x40a8, var);
       
// PCIEPHY1 
// TXRX Reg0x00A8 bit[27:22] = 6'h3F
       var = u32Lib_LeRead32((INT8U * )(PCIE_PHY_1_BASE + 0x40a8));
       var &= 0xF03FFFFF;
       var |= 0x0FC00000;
       vLib_LeWrite32(PCIE_PHY_1_BASE + 0x40a8, var);

//-- Waiting for L0 -----------------------------/
       var = u32Lib_LeRead32((INT8U * )(PCIE_CTR_0_BASE + 0x8));
       var &= 0x110;
       while(var != 0x110)
        {
           var = u32Lib_LeRead32((INT8U * )(PCIE_CTR_0_BASE + 0x8));
           var &= 0x110;
        }

	printf("Link to L0\n");
	printf("Start to Write Slave\n");

//-- Write RC Slave -----------------------------
// Setting the BAR
//    1) Configuration write 0xffff_ffff to the BAR0 Register 
//       Write 0xffff_ffff to 0x0100_0010 to RC AXI Slave
       vLib_LeWrite32(0x41000010, 0xffffffff);
//       Read 0x0100_0010
       vLib_LeWrite32(0x41000010, 0x18000000);
       vLib_LeWrite32(0x41000004, 0x00000006);
       var = u32Lib_LeRead32((INT8U * )(0x41000034));

//-- EP DBI Programing 		-----------------------------
       vLib_LeWrite32(PCIE_1_DBI + 0x300108, 0x18000000);  //set the Original Lower Base Address
       vLib_LeWrite32(PCIE_1_DBI + 0x30010c, 0x00000000);  //set the Original Upper Base Address
       vLib_LeWrite32(PCIE_1_DBI + 0x300110, 0x180fffff);  //set the Limit Address
       vLib_LeWrite32(PCIE_1_DBI + 0x300114, 0x18000000);  //set the Lower Target Address (0x01000000)
       vLib_LeWrite32(PCIE_1_DBI + 0x300118, 0x00000000);  //set the Upper Target Address (0x00000000)
       vLib_LeWrite32(PCIE_1_DBI + 0x300100, 0x00000000);  //define the type of the region to be MEM
       vLib_LeWrite32(PCIE_1_DBI + 0x300104, 0x80000000);  //enable the region
//  1.EP outbound MEM iATU (Use Region 1)
       vLib_LeWrite32(PCIE_1_DBI + 0x300208, 0x70000000);  //set the Lower Base Address
       vLib_LeWrite32(PCIE_1_DBI + 0x30020c, 0x00000000);  //set the Upper Base Address
       vLib_LeWrite32(PCIE_1_DBI + 0x300210, 0x700fffff);  //set the Limit Address
       vLib_LeWrite32(PCIE_1_DBI + 0x300214, 0x0a100000);  //set the Lower Target Address 
       vLib_LeWrite32(PCIE_1_DBI + 0x300218, 0x00000000);  //set the Upper Target Address 
       vLib_LeWrite32(PCIE_1_DBI + 0x300200, 0x00000000);  //define the type of the region to be MEM
       vLib_LeWrite32(PCIE_1_DBI + 0x300204, 0x80000000);  //enable the region
//-- RC DBI Reading 		-----------------------------
       var = u32Lib_LeRead32((INT8U * )(PCIE_1_DBI + 0x78));
       var = u32Lib_LeRead32((INT8U * )(PCIE_1_DBI + 0x80));

#ifdef TEST_MSI
		printf("Start MSI test.\n");
		vLib_LeWrite32(0x40000054	, 0x0a100000);		
		vLib_LeWrite32(0x40000058	, 0x00000000);		
		vLib_LeWrite32(0x4000005c	, 0x0000dd00);		
		vLib_LeWrite32(0x40000050	, 0x01817005);				
		vLib_LeWrite32(0x0a800054	, 0x01817005);		
		vLib_LeWrite32(0x0a800820	, 0x0a100100);		
		vLib_LeWrite32(0x0a800824	, 0x00000000);				
		vLib_LeWrite32(0x0a800828	, 0x00000001);	
		
		fLib_CloseIRQ(pcie0_irq);
	   	if (!fLib_ConnectIRQ(pcie0_irq, fLib_PCIEIntHandler))
			return ;

        	fLib_SetIRQmode(pcie0_irq, LEVEL);

	    	fLib_EnableIRQ(pcie0_irq);

		printf("Wait for MSI...\n");
		while(pcie_irq_status!=ASSERT);	
		printf("MSI test pass\n");

#else
//---------------------------------------------------------------------------
// Start write pattern from RC to EP
printf("========================================\n");
printf("Start write pattern from RC to EP\n");
       src_addr = 0x50000000;
       dst_addr = 0x18000000;
       for(loop = 0; loop < 0x1000; loop++)
        {
            vLib_LeWrite32(src_addr, src_addr);
            src_addr = src_addr + 4;
            dst_addr = dst_addr + 4;
        }
printf("========================================\n");
printf("Compare the destination address value\n");
       src_addr = 0x50000000;
       dst_addr = 0x18000000;
       for(loop = 0; loop < 0x1000; loop++)
        {
            var = u32Lib_LeRead32((INT8U * )(dst_addr));
            if(src_addr != var)
            {
                printf("PCIE_QC test failed, dst address value = 0x%x, should be 0x%x\n", var, src_addr);
                break;
            }
            src_addr = src_addr + 4;
            dst_addr = dst_addr + 4;
        }
       if(loop == 0x1000)
            printf("PASS \n");
	printf("End the PCIE_QC test\n");
#endif
}
