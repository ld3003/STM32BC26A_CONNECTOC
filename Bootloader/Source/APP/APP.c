/*
********************************************************************************
*
*                                 APP.c
*
* File          : APP.c
* Version       : V1.0
* Author        : whq
* Mode          : Thumb2
* Toolchain     : 
* Description   : ���������
*                
* History       :
* Date          : 2013.07.21
*******************************************************************************/


#include <string.h>
#include <stdio.h>

#include "APP.h"

#include "BSP_tim.h"
#include "BSP_Uart.h"

#include "HW_Config.h"

#include "atcmd.h"
#include "bsp.h"
#include "mem.h"
#include "flash.h"
#include "rtc.h"
#include "bkpreg.h"
#include "common.h"
#include <stdlib.h>

#define SERIALPORT		fputcmod = 0;
#define USBPORT				fputcmod = 1;

extern unsigned char enable_led_shanshuo;
extern unsigned char led_shanshuo_pinlv;

static void process_atcmd(void);
static void process_usbdata(void);
static void bootloader(void);


/*******************************************************************************
* Function Name : int main(void)
* Description   : ���������
* Input         : 
* Output        : 
* Other         : 
* Date          : 2013.07.21
*******************************************************************************/
int main(void)
{
	
	RCC_ClocksTypeDef rccClk = {0};
	unsigned char runapp_status = 0; 
	
	
	//�ر� SWD ����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
	watch_dog_config();
	
	led0_off();
	enable_led_shanshuo = 1;
	led_shanshuo_pinlv = 100;
	
	/*RCC SYSTICK RTC ��ʼ��*/
	RCC_GetClocksFreq(&rccClk);
	SysTick_Config(rccClk.HCLK_Frequency / 100);
	//
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������  
	
	#ifdef ENABLE_WAKEUP_DBG
	BSP_UART1Config(115200);
	printf("Wakeup Bootloader ״̬ %d \r\n",GET_BOOTLOADER_STATUS);
	#endif
	
	switch(GET_BOOTLOADER_STATUS)
	{
		case 0:
			//�̼�����ʧ��ֱ�ӽ���bootloader״̬
			bootloader();
			break;
		default:
			gotoApp();
	}
	
	
}

static void bootloader(void)
{
	
	//led0_on();
	extern unsigned int SysTickCnt;
	unsigned int SysTickCntRecord;
	unsigned int tmpCnt;
	int i=0;
	
	
	
	init_mem();
	
	/*Uart1 INIT*/
	BSP_UART1Config(115200);
	
	
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\n*");
	printf("\r\nBootloader [%s %s]\r\n",__DATE__,__TIME__);
	printf("\r\n");
	printf("\r\n");
	
	printf("Bootloader STATUS %d \r\n",GET_BOOTLOADER_STATUS);
	
	printf("CheckUSB @ 10 Second...\r\n");
	SysTickCntRecord = SysTickCnt;
	for(;;)
	{
		if ((SysTickCnt - SysTickCntRecord) > (100*10))
		{
			printf("USB ��δ���� !\r\n");
			gotoApp();
		}
		if (read_usb_status() == 1)
		{
			printf("USB �����Ѿ����� , ����Bootloader����ģʽ !\r\n");
			break;
		}
	}
	
	led_shanshuo_pinlv = 30;
	ENABLE_USB;

	USB_Config();
	
	//�����´�ֱ�ӽ�����USB��״̬
	SET_BOOTLOADER_STATUS(2);

	while (1)
	{
		//ι������
		feed_watchdog();
		//����USB ����
		process_usbdata();
		//������AT����
		process_atcmd();	
	}
}

static void process_atcmd(void)
{
	//����ATָ��
	switch(status_recv)
	{
		case STATUS_RECV_FINISH:
			break;
		case STATUS_RECV_ERROR:
			atcmd_recv_reset(); //����������Ҫ��λ
			break;
		default:
			break;
	}
	//
}

static void process_usbdata(void)
{
	uint32_t len = 0;
	static uint8_t buf[256] = {0};
	
	//������1���͹����Ķ����ݣ�ͨ��USBת����ȥ
	if (QueueIsEmpty(&UartRingQueue) == 0)
	{
		uint8_t tmp = (uint8_t)DeQueue(&UartRingQueue);
		USB_TxWrite(&tmp, 1);
	}
	
	len = USB_RxRead(buf, sizeof(buf));
	
	if (len > 0)
	{
		unsigned short i=0;
		
		UART1_Write_buffer(buf,len);
		
		for(i=0;i<len;i++)
		{
			uart_recv_handle(buf[i]); //�������
		}
			
	}
	
}

int fputc(int ch, FILE *f)
{
	
	USART_SendData(USART1, ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}
	return (ch);
}

////////

void write_usb_buffer(unsigned char*buf , int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		EnQueue(&UartRingQueue,buf[i]);
	}
}


typedef  void (*pFunction)(void);
static void JumpToApp(u32 appAddr)				
{
	pFunction JumpToApplication;
	u32 JumpAddress;
	
	//appAddr = CPU2_FW_ADDRESS;
	
	JumpAddress = *(u32*) (appAddr + 4);
	JumpToApplication = (pFunction)JumpAddress;
  /* Initialize user application's Stack Pointer */
  __set_MSP(*(vu32*) appAddr);
  JumpToApplication();
}
void gotoApp(void)
{
	//printf("����Ϊ����APP״̬\r\n");
	SET_BOOTLOADER_STATUS(1);
	JumpToApp(APPLICATION_ADDRESS);
	//
}



