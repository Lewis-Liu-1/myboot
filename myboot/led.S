#define GPBCON    0x56000010
#define    GPBDAT    0x56000014
#define    GPBUP    0x56000018

#define GPGCON    0x56000060
#define GPGDAT    0x56000064
#define GPGUP    0x56000068

#define    DELAY_nand 0x1000    /*在nand flash里面用这个延时值*/
;#define DELAY_sdram 0x100000    /*在SDRAM里面用这个*/
#define ENTRY(name) \
  .globl name; \
  name:

ENTRY(DELAY)
DELAY:
    ldr    r3 , =DELAY_nand
DELAY1:
    sub    r3 , r3 , #0x1
    cmp    r3 , #0x0
    bne    DELAY1
    mov    pc , lr   

ENTRY(led_init)
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

ENTRY(all_led_off)
/*下面初始化灯为全灭*/
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]
    ldr    r2 , =(0x0F<<5)
    bic    r1 , r1 , r2
    orr    r1 , r1 , r2
    str    r1 , [r0]        /*此时灯全灭*/
    mov    pc, lr

ENTRY(on_led1)
    mov    r2 , #(0x1<<0)
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    bic    r1 , r1 , r2
    str    r1 , [r0]
    mov    pc , lr   

ENTRY(on_led2)
    mov    r2 , #(0x1<<3)
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    bic    r1 , r1 , r2
    str    r1 , [r0]
    mov    pc , lr   

ENTRY(on_led3)
    mov    r2 , #(0x1<<5)
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    bic    r1 , r1 , r2
    str    r1 , [r0]
    mov    pc , lr   

ENTRY(on_led4)
    mov    r2 , #(0x1<<7)
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    bic    r1 , r1 , r2
    str    r1 , [r0]
    mov    pc , lr   

ENTRY(on_all_led)
/*灯全亮*/
    ldr    r0 , =GPBDAT
    ldr    r1 , [r0]    
    ldr    r2 , =(0x0F<<5)
    bic    r1 , r1 , r2
    str    r1 , [r0]        
    mov    pc , lr   

    