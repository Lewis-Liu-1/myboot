#include "global.h"
#include <stdint.h>

#define __REG(x)	(*(volatile unsigned long *)(x))
#define __REGl(x)	(*(volatile unsigned long *)(x))
#define __REGw(x)	(*(volatile unsigned short *)(x))
#define __REGb(x)	(*(volatile unsigned char *)(x))

#define bNAND_CTL(Nb)   __REG(0x4e000000 + (Nb))
#define NFCONF          bNAND_CTL(0x00)
#define NFCONT          bNAND_CTL(0x04)
#define NFCMD       bNAND_CTL(0x08)
#define NFADDR      bNAND_CTL(0x0c)
#define NFDATA      __REGb(0x4e000000 + (0x10))
#define NFSTAT      bNAND_CTL(0x20)
#define NFECC       bNAND_CTL(0x2c)

#define NF_MECC_UnLock()    {NFCONT&=~(1<<5);}
#define NF_MECC_Lock()      {NFCONT|=(1<<5);}

#define NF_CMD(cmd)			{NFCMD=cmd;}
#define NF_ADDR(addr)		{NFADDR=addr;}
#define NF_nFCE_L()			{NFCONT&=~(1<<1);}
#define NF_nFCE_H()			{NFCONT|=(1<<1);}
#define NF_RSTECC()			{NFCONT|=(1<<4);}
#define NF_RDDATA8() 		(NFDATA)

#define NF_WAITRB()    		{while(!(NFSTAT&(1<<1)));}
							 //wait tWB and check F_RNB pin.
// RnB Signal
#define NF_CLEAR_RB()    	{NFSTAT |= (1<<2);}	// Have write '1' to clear this bit.
#define NF_DETECT_RB()    	{while(!(NFSTAT&(1<<2)));}

#define TACLS		0	// 1-clk(0ns)
#define TWRPH0		6	// 3-clk(25ns)
#define TWRPH1		2	// 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns


#define ECC_VERIFY	0

int g_page_type = PAGE_UNKNOWN;

extern void ReadPage512(uint8_t* buf, volatile unsigned char *nf_data_ptr);

static inline void NandReset(void)
{
	NF_nFCE_L();
	NF_CLEAR_RB();
	NF_CMD(0xFF);		//reset command
	NF_DETECT_RB();
	NF_nFCE_H();

}

static inline void delay(void)
{
#if 0
	volatile int i;
	for (i = 0; i < 1000; i++);
#endif
}

static inline uint32_t NandCheckId(void)
{
	uint8_t Mid, Did, DontCare, id4th;

	NF_nFCE_L();

	NF_CMD(0x90);
	NF_ADDR(0x0);

	delay();

	Mid = NF_RDDATA8();
	Did = NF_RDDATA8();
	DontCare = NF_RDDATA8();
	id4th = NF_RDDATA8();

	NF_nFCE_H();

	switch(Did) {
	case 0x76:
		g_page_type = PAGE512;
		break;
	case 0xF1:
	case 0xD3:
	case 0xDA:
	case 0xDC:
		g_page_type = PAGE2048;
		break;
	default:
		;
	}

	return (uint32_t) ((Mid << 24) | (Did << 16) | (DontCare << 8) | id4th);
}

void NandInit(void)
{
	NFCONF = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4) | (0 << 0);
	NFCONT =
	    (0 << 13) | (0 << 12) | (0 << 10) | (0 << 9) | (0 << 8) | (0 << 6) |
	    (0 << 5) | (1 << 4) | (1 << 1) | (1 << 0);
	NFSTAT = 0;
	NandReset();
	NandCheckId();
}

static inline int NandIsGoodBlockP512(uint32_t addr)
{
	uint32_t sector = addr >> 9;
	uint8_t bad_value;

	NandReset();
	NF_nFCE_L();
	NF_CLEAR_RB();
	NF_CMD(0x50);
	NF_ADDR(5);
	NF_ADDR(sector & 0xff);
	NF_ADDR((sector >> 8) & 0xff);
	NF_ADDR((sector >> 16) & 0xff);

	delay();
	NF_DETECT_RB();

	bad_value = NF_RDDATA8();

	NF_nFCE_H();

	if (bad_value == 0xFF) {
		return 1;
	}

	return 0;
}

static inline int NandIsGoodBlockP2048(uint32_t addr)
{
	uint32_t sector = addr >> 11;
	uint8_t bad_value;

	NandReset();
	NF_nFCE_L();
	NF_CLEAR_RB();
	NF_CMD(0x0);
	NF_ADDR(2048 & 0xFF);
	NF_ADDR((2048 >>8) & 0xff);
	NF_ADDR(sector & 0xff);
	NF_ADDR((sector >> 8) & 0xff);
	NF_ADDR((sector >> 16) & 0xff);
	NF_CMD(0x30);
	delay();
	NF_DETECT_RB();

	bad_value = NF_RDDATA8();

	NF_nFCE_H();

	if (bad_value == 0xFF) {
		return 1;
	}

	return 0;
}

int NandIsGoodBlock(uint32_t addr)
{
	int ret;
	
	switch(g_page_type) {
	case PAGE512:
		{
			unsigned int i;
			// for ugly method of Chen Yongqiang
			for (i = 0; i < 128 * 1024; i+= 16 * 1024) {
				ret = NandIsGoodBlockP512(addr / (128 * 1024) * (128 * 1024) + i );
				if (!ret) {
					break;
				}
			}
		}
		break;
	case PAGE2048:
		ret = NandIsGoodBlockP2048(addr);
		break;
	default:
		for(;;);
	}
	return ret;
}

static inline int NandReadOneSectorP512(uint8_t * buffer, uint32_t addr)
{
	uint32_t sector;
	sector = addr >> 9;

	NandReset();
#if 0
	NF_RSTECC();
	NF_MECC_UnLock();
#endif
	NF_nFCE_L();

	NF_CLEAR_RB();
	NF_CMD(0x00);

	NF_ADDR(0x00);
	NF_ADDR(sector & 0xff);
	NF_ADDR((sector >> 8) & 0xff);
	NF_ADDR((sector >> 16) & 0xff);

	delay();
	NF_DETECT_RB();

	ReadPage512(buffer, &NFDATA);
#if 0
	NF_MECC_Lock();
#endif
	NF_nFCE_H();

	return 1;
}


static inline int NandReadOneSectorP2048(uint8_t * buffer, uint32_t addr)
{
	uint32_t sector;
	sector = addr >> 11;

	delay();
	NandReset();
#if 0
	NF_RSTECC();
	NF_MECC_UnLock();
#endif
	NF_nFCE_L();

	NF_CLEAR_RB();
	NF_CMD(0x00);

	NF_ADDR(0x00);
	NF_ADDR(0x00);
	NF_ADDR(sector & 0xff);
	NF_ADDR((sector >> 8) & 0xff);
	NF_ADDR((sector >> 16) & 0xff);
	NF_CMD(0x30);

	delay();
	NF_DETECT_RB();

	ReadPage512(buffer + 0 * 512, &NFDATA);
	ReadPage512(buffer + 1 * 512, &NFDATA);
	ReadPage512(buffer + 2 * 512, &NFDATA);
	ReadPage512(buffer + 3 * 512, &NFDATA);

#if 0
	NF_MECC_Lock();
#endif
	NF_nFCE_H();

	return 1;
}


int NandReadOneSector(uint8_t * buffer, uint32_t addr)
{
	int ret;
	
	switch(g_page_type) {
	case PAGE512:
		ret = NandReadOneSectorP512(buffer, addr);
		break;
	case PAGE2048:
		ret = NandReadOneSectorP2048(buffer, addr);
		break;
	default:
		for(;;);
	}
	return ret;
}
