#if 1
#include "global.h"

#include <stdint.h>

#define NFCONF (*(volatile unsigned long *)0x4E000000)
#define NFCONT (*(volatile unsigned long *)0x4E000004)
#define NFCMMD (*(volatile unsigned char *)0x4E000008)
#define NFADDR (*(volatile unsigned char *)0x4E00000C)
#define NFDATA (*(volatile unsigned char *)0x4E000010)
#define NFSTAT (*(volatile unsigned char *)0x4E000020)

#define TACLS 1
#define TWRPH0 2
#define TWRPH1 1

extern int g_page_type;

/**chip select**/
void select_chip()
{
    NFCONT &= ~(1 << 1);
}
/**release chip select**/
void diselect_chip()
{
    NFCONT |= (1 << 1);
}
/**clear RnB signal**/
void clear_RnB()
{
    NFSTAT |= (1 << 2);
}
/**send command**/
void send_cmmd(unsigned cmmd)
{
    NFCMMD = cmmd;
}

/**send address**/
void send_addr(unsigned addr)
{
    NFADDR = addr;
}

/**wait for RnB signal**/
void wait_RnB()
{
    while (!(NFSTAT & (1 << 2)))
        ;
}

/***reset nand flash controler****/
void nand_reset()
{
    /**chip select**/
    select_chip();

    /**clear RnB signal**/
    clear_RnB();

    /**send command 0xff ***/
    send_cmmd(0xff);

    /**wait for RnB signal**/
    wait_RnB();

    /**release chip select**/
    diselect_chip();
}

static inline void delay(void)
{
#if 0
	volatile int i;
	for (i = 0; i < 1000; i++);
#endif
}

#if 0
void nand_read_id()
{
	uint8_t Mid, Did, DontCare, id4th;

	select_chip();

	send_cmmd(0x90);
	send_addr(0x0);

	delay();

	Mid = NFDATA;
	Did = NFDATA;
	DontCare = NFDATA;
	id4th = NFDATA;

	diselect_chip();

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

	//return (U32) ((Mid << 24) | (Did << 16) | (DontCare << 8) | id4th);
}
#endif

/***initialize nand flash controler**/
void nand_init()
{ /**initialize configuration register**/
    NFCONF = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);
    /**initialize control register**/
    NFCONT = 
	    (0 << 13) | (0 << 12) | (0 << 10) | (0 << 9) | (0 << 8) | (0 << 6) |
	    (0 << 5) | (1 << 4) | (1 << 1) | (1 << 0);
    NFSTAT = 0;
    /**reset nand flash controler**/
    nand_reset();

    //nand_read_id();
}

/****read datas by Page****/
void nand_PageRead(unsigned long addr, unsigned char *buff)
{
    int i = 0;
    /**chip select**/
    select_chip();

    /**clear RnB signal**/
    clear_RnB();

    /**send command 0x00 **/
    send_cmmd(0x00);

    /**send column address**/
    send_addr(0x00);
    send_addr(0x00);

    /**send row address**/
    send_addr(addr & 0xff);
    send_addr((addr >> 8) & 0xff);
    send_addr((addr >> 16) & 0xff);

    /**send command 0x30**/
    send_cmmd(0x30);

    /**wait for RnB signal**/
    wait_RnB();

    /**read datas byte by byte**/
    for (i = 0; i < 2048; i++)
    {
        buff[i] = NFDATA;
    }

    /**release chip select**/
    diselect_chip();
}
/***write datas by Page***/
unsigned long nand_PageWrite(unsigned long addr, unsigned char *buff)
{
    int i = 0;
    unsigned long ret = 0;
    /**chip select**/
    select_chip();

    /**clear RnB signal**/
    clear_RnB();

    /**send command Input command ox80**/
    send_cmmd(0x80);

    /**send Column address**/
    send_addr(0x00);
    send_addr(0x00);

    /**send Row address**/
    send_addr(addr & 0xff);
    send_addr((addr >> 8) & 0xff);
    send_addr((addr >> 16) & 0xff);
    /**write datas**/
    for (i = 0; i < 2048; i++)
    {
        NFDATA = buff[i];
    }
    /**send Program command 0x10**/
    send_cmmd(0x10);
    /**wait for RnB signal **/
    wait_RnB();

    /**send read status command 0x70**/
    send_cmmd(0x70);

    /**read program status**/
    ret = NFDATA;

    /**release chip select**/
    diselect_chip();

    return ret;
}
/****write datas to sdram from nand flash****/
void nand_to_ram(unsigned long start_addr, unsigned char *sdram_addr, int size)
{
    int i = 0;
    for (i = (start_addr >> 11); size > 0;)
    {
        /***each circle wirtes 1 page***/
        nand_PageRead(i, sdram_addr);
        size -= 2048;
        sdram_addr += 2048;
        i++;
    }
}
/****erase nand flash****/
unsigned long nand_erase(unsigned long addr)
{
    unsigned long ret = 0;
    /**chip select**/
    select_chip();

    /**clear RnB signal**/
    clear_RnB();

    /***send erase setup command***/
    send_cmmd(60);

    /**send Row address**/
    send_addr(addr & 0xff);
    send_addr((addr >> 8) & 0xff);
    send_addr((addr >> 16) & 0xff);

    /**send erase command**/
    send_cmmd(0xd0);

    /**wait for RnB signal**/
    wait_RnB();

    /**send read status command**/
    send_cmmd(0x70);

    /**read erase status**/
    ret = NFDATA;
    /***release chip select***/
    diselect_chip();

    return ret;
}
#endif