
#define GPHCON (*((volatile unsigned long *)0x56000070))

#define INTMSK (*((volatile unsigned long *)0x4a000008))
#define INTSUBMSK (*((volatile unsigned long *)0x4a00001c))
#define SUBSRCPND (*((volatile unsigned long *)0x4a000018))

#define ULCON0 (*((volatile unsigned char *)0x50000000))
#define UCON0 (*((volatile unsigned long *)0x50000004))
#define UTRSTAT0 (*((volatile unsigned char *)0x50000010))
#define UBRDIV0 (*((volatile unsigned char *)0x50000028))
#define UTXH0 (*((volatile unsigned char *)0x50000020))
#define URXH0 (*((volatile unsigned char *)0x50000024))

void port_init()
{
    GPHCON &= ~(0xf << 4);
    GPHCON |= (0xa << 4);
}

void uart_con_init()
{
    /**8-bits word length ;1 stop bit ; none parity **/
    ULCON0 = (3 << 0) | (0 << 2) | (0 << 3) | (0 << 6);

    /**Tx Rx Mode: polling mode; Clock for the baud rate is PCLK**/
    UCON0 = 0b1001;

    /**Baud rate is 115200 bps PCLK = 50M***/
    UBRDIV0 = (int)(50000000 / (115200 * 16) - 1);
}

void uart_init()
{
    port_init();
    uart_con_init();
}

void putc(unsigned char ch)
{
    while (!(UTRSTAT0 & (1 << 2)))
        ;
    UTXH0 = ch;
}

void printf(char *str)
{
    int i = 0;
    while (str[i])
    {
        putc(str[i++]);
    }
}

void display_menul()
{
    printf("\n******************************************\n\r");
    printf("\n********** now boot kernel from NAND *************\n\r");
}