.text
.global _start
/******************1. set exception vectors*******************/
_start:
       b    reset
       ldr  pc, _undefined_instructions
       ldr  pc, _software_interrupt
       ldr  pc, _prefetch_abort
       ldr  pc, _data_abort
       ldr  pc, _not_used
       ldr  pc, _irq
       ldr  pc, _fiq


_undefined_instructions: .word undefined_instructions
_software_interrupt:     .word software_interrupt
_prefetch_abort:         .word prefetch_abort
_data_abort:             .word data_abort
_not_used:               .word not_used
_irq:                    .word irq
_fiq:                    .word fiq    

undefined_instructions:
        nop

software_interrupt:
        nop

prefetch_abort:
        nop

data_abort:
        nop
      
not_used:
        nop

irq:
        sub lr, lr, #4   /**save environment**/
        stmfd sp!, {r0-r12,lr}       
        @bl handle_int
        ldmfd sp!, {r0-r12,pc}^ /**restore environment**/
                                /*** ^ indicates restoring cpsr from spsr**/
fiq:
        nop


/*************************************/
/*********start code on reset*********/
/*************************************/

reset:
    bl set_cpu_mode         /****** 2. set to svc32 mode            ******/
    bl turn_off_watchdog    /****** 3. turn off watchdog            ******/
    bl mask_irqs            /****** 4. mask all irqs                ******/
    bl set_clock            /****** 5. set clock                    ******/     
    bl disable_id_caches    /****** 6. disable I/Dcaches and mmu    ******/
    bl init_memory          /****** 7. initialize memory controler  ******/
    bl clean_bss            /****** 8. clear bss section            ******/
    bl init_stack           /****** 9. initialize stack             ******/
    bl nand_init            /****** 10. initialize nand flash controler *****/
    bl copy_to_ram          /****** 11. copy datas to sdram             *****/

    bl led_init
    bl on_all_led
    bl DELAY

//loop:
//   b loop

    ldr pc, =arm_main      /****12. jump to c function******/


set_cpu_mode:      
    /****** set to svc32 mode ******/
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0xd3 /*设置为SVC模式，M[4:0] 10011, 并关闭fiq,irq中断*/
    msr cpsr, r0
    mov pc, lr

turn_off_watchdog:
     /*******turn off watchdog********/  
#define WTCON 0x53000000
    ldr r0, =WTCON
    mov r1, #0x0
    str r1, [r0] /*  把[WTCON]内存单元清零  */
    mov pc, lr    

mask_irqs:
     /*********mask all irqs**********/
      mvn r0, #0                /*   取反赋值 r1 = 0xffffffff   */
      ldr r1, =0x4a000008
      str r0, [r1]              /*  INTMASK每一位均写1  屏蔽中断 */
      mov pc,lr

/*  初始化系统时钟 FCLK = 400MHz,HCLK = 100MHz, PCLK = 50MHz, UPLL=48MHz  */
.EQU  LOCKTIME,    0x4c000000           
.EQU  MPLLCON,     0x4c000004    
.EQU  UPLLCON,     0x4c000008    
.EQU  CLKDIVN,     0x4c000014    
.EQU  M_MDIV,      92       /* Fin=12M  UPLL=400M  */
.EQU  M_PDIV,      1
.EQU  M_SDIV,      1
.EQU  U_MDIV,      56        /* Fin=12M  UPLL=48M  */
.EQU  U_PDIV,      2
.EQU  U_SDIV,      2
.EQU  DIVN_UPLL,   0         /* FCLK:HCLK:PCLK=1:4:8 */ 
.EQU  HDIVN,       2         
.EQU  PDIVN,       1  
#define CLKDIVN 0x4c000014
#define MPLLCON 0x4C000004
#define MPLL_405M ((0x7f<<12)|(2<<4)|(1<<0))
set_clock:
     /*****FCLK:HCLK:PCLK = 1:4:8*********/
      /* 设置分频系数 */
    ldr r0, =CLKDIVN
    mov r1, #0x5  //((DIVN_UPLL<<3) | HDIVN <<1 | PDIVN)
    str r1, [r0]

    /* CPU改为异步总线模式 */
    mrc p15, 0, r0,c1,c0,0
    orr r0,r0,#0xc0000000
    mcr p15,0,r0,c1,c0,0
    
    /*******FCLK = 405MHZ************/
      ldr r0, =MPLLCON
      ldr r1, =MPLL_405M
      str r1, [r0]

   /* 设置UPLL */
    ldr r0,=UPLLCON  
    ldr r1,=((U_MDIV<<12) | (U_PDIV<<4) | U_SDIV)    
    str r1, [r0]  

    mov pc, lr

disable_id_caches:
      /*******flush I/Dcaches and mmu*****/
      mcr p15, 0, r0, c7, c7,0
      mrc p15, 0, r0, c1, c0,0
      bic r0, r0,#0x00000007 
      mcr p15, 0, r0, c1,c0, 0
      mov pc, lr
 
init_memory:
      /*******initialize memory controler********/
      ldr r0, =0X48000000
      add r3, r0, #4*13
      adrl r1, mem_data
      
      
0:
      ldr r2, [r1],#4
      str r2, [r0],#4
      cmp r0, r3
      bne 0b
      mov pc, lr
mem_data:
          .long 0x22000000
          .long 0x00000700
          .long 0x00000700
          .long 0x00000700
          .long 0x00000700
          .long 0x00000700
          .long 0x00000700
          .long 0x00018001
          .long 0x00018001
          .long 0x008c04f5
          .long 0x000000b1
          .long 0x00000030
          .long 0x00000030

copy_to_ram:
      /*******copy datas to sdram from stepping stone*****/
      mov r0, #0
      ldr r1, =_start
      ldr r2, =bss_end
      sub r2, r2,r1
      mov ip, lr
      //bl nand_to_ram
      mov lr,ip
      mov pc, lr

init_stack:
      /********initializing stack*********/
      msr cpsr_c, #0xd2
      ldr sp, =0x33000000 /**R13_irq**/
      msr cpsr_c, #0xd3
      ldr sp, =0x34000000 /**R13_svc**/
      mov pc, lr

clean_bss:
      /********clearing bss section**********/
	ldr r0, =bss_start
	ldr r1, =bss_end
	cmp r0, r1
	moveq pc, lr


	.globl ReadPage512

ReadPage512:
	stmfd	sp!, {r2-r7}
	mov	r2, #0x200
1:
	ldr	r4, [r1]
	ldr	r5, [r1]
	ldr	r6, [r1]
	ldr	r7, [r1]
	stmia	r0!, {r4-r7}
	ldr	r4, [r1]
	ldr	r5, [r1]
	ldr	r6, [r1]
	ldr	r7, [r1]
	stmia	r0!, {r4-r7}
	ldr	r4, [r1]
	ldr	r5, [r1]
	ldr	r6, [r1]
	ldr	r7, [r1]
	stmia	r0!, {r4-r7}
	ldr	r4, [r1]
	ldr	r5, [r1]
	ldr	r6, [r1]
	ldr	r7, [r1]
	stmia	r0!, {r4-r7}
	subs	r2, r2, #64
	bne	1b;
	ldmfd	sp!, {r2-r7}
	mov	pc,lr
