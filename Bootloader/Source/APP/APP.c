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

unsigned char fputcmod = 0;

#define SERIALPORT		fputcmod = 0;
#define USBPORT				fputcmod = 1;

static void process_atcmd(void);
static void process_usbdata(void);


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

	/*systick init*/
	RCC_ClocksTypeDef rccClk = {0};
	RCC_GetClocksFreq(&rccClk);
	SysTick_Config(rccClk.HCLK_Frequency / 100);
	
	/*Uart1 INIT*/
	BSP_UART1Config(115200);
	
	if (read_usb_status() == 0)
	{
		printf("USB �����Ѿ����� !\r\n");
		//
	}else{
		printf("USB ��δ���� !\r\n");
	}

	/*
			��ȡ USB״̬���ж��Ƿ����bootloaderģʽ

			�жϱ��λ
			���APP������������jump
			���APPδ�������������ָʾ��
	*/

	//printf("USB PIN STATUS %d %d \r\n",BSP_GpioRead(USB_DM_PIN),BSP_GpioRead(USB_DP_PIN));

	//for(;;){};

	USB_Config();

	printf("ϵͳ��ʼ�����\r\n");

	while (1)
	{
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
	unsigned char tmp = (char)ch;
	if (fputcmod == 1)
		USB_TxWrite(&tmp,1);
	else
	{
		USART_SendData(USART1, tmp);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {}
	}
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




