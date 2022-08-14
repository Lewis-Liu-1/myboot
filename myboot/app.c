#include <stdint.h>

#include "global.h"

/********** SDRAM and Nand Space Allocation ****************/
#define SDRAM_TOTAL_SIZE 0x04000000   // 64M SDRAM
#define SDRAM_ADDR_START 0x30000000   //起始地址是0x3000，0000
#define SDRAM_LBOOT_START 0x33000000  // LBOOT的存放地址
#define SDRAM_TAGS_START 0x30000100   // tag列表的存放地址
#define SDRAM_KERNEL_START 0x33800000 // kernel的存放地址

#define NAND_KERNEL_START 0x00060000
#define NAND_KERNEL_SZIE 0x00500000

/*************** Linux Kernel Boot Parameters ***********/
#define S3C2440_MATHINE_TYPE 1999

void (*theKernel)(int, int, unsigned int);
static struct Atag *pCurTag; /* used to point at the current ulTag */

const char *CmdLine = "root=/dev/mtdblock3 console=ttySAC0,115200 mem=64M init=/linuxrc";

extern int g_page_type;
void printf(char *str);

void *memset(void *dst, int src, unsigned int len)
{
	char *p = dst;
	while (len--)
		*p++ = src;
	return dst;
}
void *memcpy(void *dst, const void *src, unsigned int len)
{
	const char *s = src;
	      char *d = dst;
	while (len --) {
		*d++ = *s++;
	}
	return dst;
}
int strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}
char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}

#define OS_LINUX 0x02
#define OS_WINCE 0x04
struct zboot_first_sector {
	unsigned char  dont_care[0x20];
	unsigned int   magic;
	unsigned char  os_type;
	unsigned char  has_nand_bios;
	unsigned short logo_pos;
	unsigned int   os_start;
	unsigned int   os_length;
	unsigned int   os_ram_start;
	unsigned char  linux_cmd[512 - 0x34];
}  __attribute__((packed)) first_sector;

#define g_magic			(first_sector.magic)
#define g_os_type		(first_sector.os_type)
#define g_has_nand_bios		(first_sector.has_nand_bios)
#define g_logo_pos		(first_sector.logo_pos)
#define g_os_start		(first_sector.os_start)
#define g_os_length		(first_sector.os_length)
#define g_os_ram_start		(first_sector.os_ram_start)
#define g_linux_cmd_line	(first_sector.linux_cmd)

#if 0
static inline void GetParameters(void)
{
	uint32_t Buf[2048];
	g_os_type = OS_LINUX;
	g_os_start = 0x60000;
	g_os_length = 0x500000;

	g_os_ram_start = 0x30008000;

	// vivi LINUX CMD LINE
	NandReadOneSector((uint8_t *)Buf, 0x48000);
	if (Buf[0] == 0x49564956 && Buf[1] == 0x4C444D43) {
		memcpy(g_linux_cmd_line, (char *)&(Buf[2]), sizeof g_linux_cmd_line);
	}
}

static void CallLinux(void)
{
	struct param_struct {
		union {
			struct {
				unsigned long page_size;	/*  0 */
				unsigned long nr_pages;	/*  4 */
				unsigned long ramdisk_size;	/*  8 */
				unsigned long flags;	/* 12 */
				unsigned long rootdev;	/* 16 */
				unsigned long video_num_cols;	/* 20 */
				unsigned long video_num_rows;	/* 24 */
				unsigned long video_x;	/* 28 */
				unsigned long video_y;	/* 32 */
				unsigned long memc_control_reg;	/* 36 */
				unsigned char sounddefault;	/* 40 */
				unsigned char adfsdrives;	/* 41 */
				unsigned char bytes_per_char_h;	/* 42 */
				unsigned char bytes_per_char_v;	/* 43 */
				unsigned long pages_in_bank[4];	/* 44 */
				unsigned long pages_in_vram;	/* 60 */
				unsigned long initrd_start;	/* 64 */
				unsigned long initrd_size;	/* 68 */
				unsigned long rd_start;	/* 72 */
				unsigned long system_rev;	/* 76 */
				unsigned long system_serial_low;	/* 80 */
				unsigned long system_serial_high;	/* 84 */
				unsigned long mem_fclk_21285;	/* 88 */
			} s;
			char unused[256];
		} u1;
		union {
			char paths[8][128];
			struct {
				unsigned long magic;
				char n[1024 - sizeof(unsigned long)];
			} s;
		} u2;
		char commandline[1024];
	};

	struct param_struct *p = (struct param_struct *)0x30000100;
	memset(p, 0, sizeof(*p));
	memcpy(p->commandline, g_linux_cmd_line, sizeof(g_linux_cmd_line));
	p->u1.s.page_size = 4 * 1024;
	p->u1.s.nr_pages = 64 * 1024 * 1024 / (4 * 1024);

	{
		unsigned int *pp = (unsigned int *)(0x30008024);
		if (pp[0] == 0x016f2818) {  // Magic number of zImage
			//printf("\n\rOk\n\r");
		} else {
			printf("\n\rWrong Linux Kernel\n\r");
			for (;;) ;
		}

	}
 	asm (
		"mov	r5, %2\n"
		"mov	r0, %0\n"
		"mov	r1, %1\n"
		"mov	ip, #0\n"
		"mov	pc, r5\n"
		"nop\n" "nop\n":	/* no outpus */
		:"r"(0), "r"(1999), "r"(g_os_ram_start)
	);
}

void ReadImageFromNand(void)
{
    #if 0
	unsigned int Length;
	uint8_t *RAM;
	unsigned BlockNum;
	unsigned pos;

	Length = g_os_length;
	Length = (Length + BLOCK_SIZE - 1) >> (BYTE_SECTOR_SHIFT + SECTOR_BLOCK_SHIFT) << (BYTE_SECTOR_SHIFT + SECTOR_BLOCK_SHIFT); // align to Block Size

	BlockNum = g_os_start >> (BYTE_SECTOR_SHIFT + SECTOR_BLOCK_SHIFT);
	RAM = (uint8_t *) g_os_ram_start;
	for (pos = 0; pos < Length; pos += BLOCK_SIZE) {
		unsigned int i;
		// skip badblock
		for (;;) {
			if (NandIsGoodBlock
			    (BlockNum <<
			     (BYTE_SECTOR_SHIFT + SECTOR_BLOCK_SHIFT))) {
				break;
			}
			BlockNum++;	//try next
		}
		for (i = 0; i < BLOCK_SIZE; i += SECTOR_SIZE) {
			int ret =
			    NandReadOneSector(RAM,
					      (BlockNum <<
					       (BYTE_SECTOR_SHIFT +
						SECTOR_BLOCK_SHIFT)) + i);
			RAM += SECTOR_SIZE;
			ret = 0;

		}

		BlockNum++;
	}

	CallLinux();
    #endif
}


int boot_linux()
{
    GetParameters();
	printf("load Image of Linux...\n\r");
    return 0;
}
#endif

void arm_main()
{
    uart_init();

    // display_menul();
    printf("prepare to boot\r\n");

    #if 0
    NandInit();
    if (g_page_type == PAGE_UNKNOWN) {
		printf("\r\nunsupport NAND\r\n");
		for(;;);
	}

    boot_linux();
    #endif
    
    while (1)
        ;
}
