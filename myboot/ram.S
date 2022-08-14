/*  SDRAM初始化 */
.text
.global  memory_init
#define  BWSCON         0x48000000                /* 总线宽度和等待控制寄存器 0x00*/
#define  BANKCON0       0x48000004                /* Bank0 控制寄存器 0x0700 */
#define  BANKCON1       0x48000008                /* Bank1 控制寄存器 0x0700 */
#define  BANKCON2       0x4800000C                /* Bank2 控制寄存器 0x0700 */
#define  BANKCON3       0x48000010                /* Bank3 控制寄存器 0x0700 */
#define  BANKCON4       0x48000014                   /* Bank4 控制寄存器 0x0700 */
#define  BANKCON5       0x48000018                  /* Bank5 控制寄存器 0x0700 */
#define  BANKCON6       0x4800001C                  /* Bank6 控制寄存器 0x0700 */
#define  BANKCON7       0x48000020                  /* Bank7 控制寄存器 0x0700 */
#define  REFRESH        0x48000024                /* SDRAM 刷新控制寄存器  0xAC0000 */
#define  BANKSIZE       0x48000028                /* 可变Bank大小寄存器   0x00 */
#define  MRSRB6         0x4800002C                /* 模式寄存器组寄存器Bank6 */
#define  MRSRB7         0x48000030                /* 模式寄存器组寄存器Bank7 */

memory_init:
    /* 初始化BWSCON */
    ldr r0,=BWSCON
    ldr r1,=0x02000000
    str r1,[r0]
    
    /* 初始化BANKCON0 */
    ldr r1,=0x00000700
    str r1,[r0,#0x04]

    /* 初始化BANKCON1 */
    ldr r1,=0x00000700
    str r1,[r0,#0x08]
    
    /* 初始化BANKCON2 */
    ldr r1,=0x00000700
    str r1,[r0,#0x0C]
    
    /* 初始化BANKCON3 */
    ldr r1,=0x00000700
    str r1,[r0,#0x10]
    
    /* 初始化BANKCON4 */
    ldr r1,=0x00000700
    str r1,[r0,#0x14]
    
    /* 初始化BANKCON5 */
    ldr r1,=0x00000700
    str r1,[r0,#0x18]
    
    /* 初始化BANKCON6 */
    ldr r1,=0x00018005
    str r1,[r0,#0x1C]
    
    /* 初始化BANKCON7 */
    ldr r1,=0x00018005
    str r1,[r0,#0x20]
    
    /* 初始化REFRESH */
    ldr r1,=0x008C04F5
    str r1,[r0,#0x24]
    
    /* 初始化BANKSIZE */
    ldr r1,=0x000000B1
    str r1,[r0,#0x28]
    
    /* 初始化MRSRB6 */
    ldr r1,=0x00000030
    str r1,[r0,#0x2C]
    
    /* 初始化MRSRB7 */
    ldr r1,=0x00000030
    str r1,[r0,#0x30]
    mov  pc,lr                 /* bl指令将下一条指令地址复制到了lr,子程序返回  */