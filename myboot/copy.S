#define GPBCON    0x56000010
#define    GPBDAT    0x56000014
#define    GPBUP    0x56000018

#define GPGCON    0x56000060
#define GPGDAT    0x56000064
#define GPGUP    0x56000068

#define    DELAY_nand 0x1000    /*在nand flash里面用这个延时值*/
;#define DELAY_sdram 0x100000    /*在SDRAM里面用这个*/

.text
.global _start
_start:

/******配置KEY1和KEY6相关管脚为内部上拉输入功能******/
/*把KEY1到KEY6管脚置为输入*/
    ldr    r0 , =GPGCON
    ldr    r1 , [r0]
    ldr    r2 , =(0x3<<22)|(0x3F<<10)|(0x3<<6)|(0x3<<0)
    bic    r1 , r1 , r2
    str    r1 , [r0]

/*使能KEY1到KEY6按键的内部上拉输出功能*/
 ldr    r0 , =GPGUP
    ldr    r1 , [r0]
    ldr    r2 , =(0x1<<11)|(0x7<<5)|(0x1<<3)|(0x1<<0)
    bic    r1 , r1 , r2
    str    r1 , [r0]

/******配置LED1,2相关管脚为内部上拉输出功能******/
/*把LED1-4管脚置为输出*/
    ldr    r0 , =GPBCON
    ldr    r1 , [r0]        /*把GPBCON里的内容加载到r1里*/
    ldr    r2 , =(0xFF<<10)
    bic    r1 , r1 ,r2    /*    操作数取反码或上r1，用于清零工作*/
    ldr    r2 , =(0x55<<10)
    orr    r1 , r1 , r2
    str    r1 , [r0]    

/*使能LED1-4管脚的上拉功能*/
    ldr    r0 , =GPBUP
    ldr    r1 , [r0]
    ldr    r2 , =(0x0F<<5)
    bic    r1 , r1 ,r2
    str    r1 , [r0]

/*下面初始化灯为全灭*/
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]
    ldr    r2 , =(0x0F<<5)
    bic    r1 , r1 , r2
    orr    r1 , r1 , r2
    str    r1 , [r0]        /*此时灯全灭*/

/***************下面开始判断是否有按键按下************/
IS_KEY:
    ldr    r0 , =GPGDAT
    ldr    r1 , [r0]
    ldr    r2 , =(0x1<<11)|(0x7<<5)|(0x1<<3)|(0x1<<0)
    and    r3 , r1 , r2
    cmp    r2 , r3
    beq    IS_KEY
    bl    DELAY        /*此处用于消抖延时*/
    ldr    r1 , [r0]
    and    r3 , r1 , r2
    cmp    r2 , r3
    beq    IS_KEY

/*判断是否为KEY6被按下*/
    ldr    r2 , =(0x1<<11)
    and    r2 , r2 , r1
    cmp    r2 , #0x0
    beq    IS_KEY6

/*判断是否为KEY5被按下*/
    ldr    r2 , =(0x1<<7)
    and    r2 ,r2 ,r1
    cmp    r2 , #0x0
    beq    IS_KEY5

/*判断是否为KEY4被按下*/
    ldr    r2 , =(0x1<<6)
    and    r2 , r2 , r1
    cmp    r2 , #0x0
    mov    r2 , #(0x1<<8)
    beq    IS_KEY1_4    

/*判断是否为KEY3被按下*/
    ldr    r2 , =(0x1<<5)
    and    r2 , r2 , r1
    cmp    r2 , #0x0
    mov    r2 , #(0x1<<7)
    beq    IS_KEY1_4    

/*判断是否为KEY2被按下*/
    ldr    r2 , =(0x1<<3)
    and    r2 , r2 , r1
    cmp    r2 , #0x0
    mov    r2 , #(0x1<<6)
    beq    IS_KEY1_4    

/*判断是否为KEY1被按下*/
    ldr    r2 , =(0x1<<0)
    and    r2 , r2 , r1
    cmp    r2 , #0x0
    mov    r2 , #(0x1<<5)
    beq    IS_KEY1_4    

OTHERS:
    b    IS_KEY
/****************判断结束***************/



/************按键触发事件**************/

IS_KEY6:/*灯全灭*/
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    ldr    r2 , =(0x0F<<5)
    bic    r1 , r1 , r2
    orr    r1 , r1 , r2
    str    r1 , [r0]        
    b    IS_KEY

IS_KEY5:/*灯全亮*/
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    ldr    r2 , =(0x0F<<5)
    bic    r1 , r1 , r2
    str    r1 , [r0]        
    b    IS_KEY

IS_KEY1_4:/*用于KEY1—4按下对应LED灯状态反转*/
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    and    r3 , r1 , r2
    cmp    r3 , #0x0
    beq    D1
    bic    r1 , r1 , r2
    str    r1 , [r0]
    b    IS_KEY
D1:
    orr    r1 , r1 , r2
    str    r1 , [r0]
    b    IS_KEY    

/********按键触发事件END***********/


/************延时函数***********/
DELAY:
    ldr    r3 , =DELAY_nand
DELAY1:
    sub    r3 , r3 , #0x1
    cmp    r3 , #0x0
    bne    DELAY1
    mov    pc , lr                        
/*************END*************/

//原文链接：https://blog.csdn.net/shengnan_wu/article/details/8168795