#ifndef __flash_h__
#define __flash_h__

int program_flash(void); //��flash���б��

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

void FLASH_ProgramStart(u32 addr , u32 size);
u32 FLASH_AppendOneByte(u8 Data);
u32 FLASH_AppendBuffer(u8 *Data , u32 size);
void FLASH_AppendEnd(void);
u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);
void FLASH_ProgramDone(void);

/**
DEMO:
	printf("FLASH DATA A %d\r\n",((unsigned char*)(CONFIG_DATA_ADDR))[0]);
	FLASH_ProgramStart(CONFIG_DATA_ADDR,CONFIG_DATA_SIZE);
	FLASH_AppendOneByte(0xF1);
	FLASH_AppendEnd();
	FLASH_ProgramDone();
	
	printf("FLASH DATA B %d\r\n",((unsigned char*)(CONFIG_DATA_ADDR))[0]);
*/

//FLASH_ZONE
#define FLASH_TOTAL_SIZE (128*1024) //Flash����

#define CONFIG_DATA_SIZE 2048
#define CONFIG_DATA_ADDR (0x8000000 + (FLASH_TOTAL_SIZE - CONFIG_DATA_SIZE))		//4�����ÿռ�

#define REG_SIZE	1024 //�����С����������ģ��ı���ı�ӳ���ַ
#define REG_ADDR	(0x8000000 + (FLASH_TOTAL_SIZE - CONFIG_DATA_SIZE - REG_SIZE))  //1024�ֽ�

#define IMG_CACHE_SIZE (1024*13)
#define IMG_CACHE_TOTAL_SIZE (IMG_CACHE_SIZE*3)
#define IMG_CACHE_ADDR_0	(0x8000000 + (FLASH_TOTAL_SIZE - CONFIG_DATA_SIZE - REG_SIZE - IMG_CACHE_TOTAL_SIZE + 0)) //10K
#define IMG_CACHE_ADDR_1	(0x8000000 + (FLASH_TOTAL_SIZE - CONFIG_DATA_SIZE - REG_SIZE - IMG_CACHE_TOTAL_SIZE + IMG_CACHE_SIZE)) //10K
#define IMG_CACHE_ADDR_2	(0x8000000 + (FLASH_TOTAL_SIZE - CONFIG_DATA_SIZE - REG_SIZE - IMG_CACHE_TOTAL_SIZE + IMG_CACHE_SIZE + IMG_CACHE_SIZE)) //10K

// TOTAL SIZE 1 + 1 + 24 = 26K


/**************

**************/


//0x8000000 + ((128*1024) - 1024 - 128)


#define BOOTLOADER_SIZE (1024*10)
#define APPLICATION_SIZE (1024*32)
#define APPLICATION_ADDRESS (0x8000000 + BOOTLOADER_SIZE)
#define DOWNLOAD_ADDRESS (0x8000000 + BOOTLOADER_SIZE + APPLICATION_SIZE)
#define DOWNLOAD_SIZE (1024*32)

#endif