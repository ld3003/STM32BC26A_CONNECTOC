
#include "main.h"
#include "ov2640api.h"
#include "BC95/neul_bc95.h"
#include "BASE64/cbase64.h"
#include "JSON/cjson.h"
#include "ledctl.h"
#include "common.h"

#include <stdio.h>

static char MYDEVICEID[32] = "000000000000000f";

static int make_json_data(char *oustr)
{
	
	char * p = 0;
	cJSON * pJsonRoot = NULL;
	char tmpstr[128];
 

	pJsonRoot = cJSON_CreateObject();
	if(NULL == pJsonRoot){return -1;}
	

	cJSON_AddNumberToObject(pJsonRoot, "VERSION", 1);
	//AT+CIMI
	cJSON_AddStringToObject(pJsonRoot, "DEVID", MYDEVICEID);
	cJSON_AddStringToObject(pJsonRoot, "FIRST", "BC26 BRD");
	cJSON_AddStringToObject(pJsonRoot, "NAME", "bc26");
	cJSON_AddStringToObject(pJsonRoot, "NUM", "100");
	cJSON_AddStringToObject(pJsonRoot, "ONLINE", "online");
	cJSON_AddStringToObject(pJsonRoot, "TIME", "2018.8");
	cJSON_AddStringToObject(pJsonRoot, "STATUS", "ON");
	snprintf(tmpstr,64,"Current Battery %d mV",read_vdd_voltage());
	cJSON_AddStringToObject(pJsonRoot, "BODY", tmpstr );
	
	p = cJSON_Print(pJsonRoot);
	
	if(NULL == p)
	{
		cJSON_Delete(pJsonRoot);
		return -1;
	}
	cJSON_Delete(pJsonRoot);
	
	sprintf(oustr,"%s",p);
	
	printf("JSON:%s\r\n",oustr);
	
	free(p);
	return 0;
}

static int push_data_func(unsigned char * push_data , int push_length)
{
	
	int i = 0;
	int ret;
	int ret1 = -1;
	int length;
	int recvlen;

	
	char *sbuffer; //�洢������������
	char *hexbuffer;
	
	sbuffer = malloc(1024 + 32);
	hexbuffer = malloc(1024 + 32);
	
	conv_hex_2_string((unsigned char*)push_data,push_length,(char*)hexbuffer);

	sprintf((char*)sbuffer,"AT+QLWDATASEND=19,0,0,512,");
	strcat((char*)sbuffer,(char*)hexbuffer);
	
	if(push_length < 512)
	{
		for(i=0;i<(512 - push_length);i++)
		{
			strcat(sbuffer,"00");
		}
	}
	strcat(sbuffer,",0x0000\r\n");
	
	printf("SEND TO BC26 STR: %s ]\r\n",sbuffer);
	
	uart_data_write(sbuffer, strlen(sbuffer), 0);
	
	free(sbuffer);
	free(hexbuffer);
	
	return ret;

}


/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH,(0x8000000+4));
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
	init_led();
	SysTick_Config(SystemCoreClock / 100);
	
	init_uart1();
	init_uart2();
	RTC_Init();
	SET_BOOTLOADER_STATUS(2);
	WKUP_Pin_Init();
	
	init_utimer();
	init_task();
	init_mem();
	init_uart2_buffer();
	
	LED_NETWORK_REGISTER_STATUS;
	
	modem_poweron();
	
	vdd_5v_out(1);
	vdd_3v3_out(1);
	
	
	while(neul_bc26_get_netstat()<0){};										//�ȴ�����������
	
	{
		
		/*
		 * �����ڴ�
		 */
		#define RECV_BUF_LEN 1024
		char *recvbuf = malloc(RECV_BUF_LEN);
		char *atbuf = malloc(1024);
		char *jsonbuf = malloc(512);
		
		
		
		/*
		 * ����ATָ��
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT\r\n", strlen("AT\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * ��PSM
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+CPSMS=1\r\n", strlen("AT+CPSMS=1\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * �رջ���
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("ATE0\r\n", strlen("ATE0\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);

		/*
		 * ��ȡ�ź�ֵ
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+CSQ\r\n", strlen("AT+CSQ\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * ��ȡ�豸ID IMEI
		 */
		
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+CGSN=1\r\n", strlen("AT+CGSN=1\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		if (strstr(recvbuf,"+CGSN:"))
		{
			MYDEVICEID[15]='f';
			MYDEVICEID[16]=0;
			memcpy(MYDEVICEID,recvbuf+9,15);
			printf("IMEI: %s\r\n",MYDEVICEID);
		}
		
		/*
		 * ���ӵ����Ʒ�����
		 */
		{
			char *tmpstr;
			tmpstr = (char*)malloc(128);
			snprintf(tmpstr,128,"%s","AT+QLWSERV=180.101.147.115,5683\r\n");
			memset(recvbuf,0x0,RECV_BUF_LEN);
			uart_data_write(tmpstr,strlen(tmpstr),0);
			uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
			free(tmpstr);
		}
		
		{
			char *tmpstr;
			tmpstr = (char*)malloc(128);
			snprintf(tmpstr,128,"AT+QLWCONF=\"%s\"\r\n",MYDEVICEID);
			memset(recvbuf,0x0,RECV_BUF_LEN);
			uart_data_write(tmpstr,strlen(tmpstr),0);
			uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
			free(tmpstr);
		}
		
		{
			//AT+QLWCONF?\r\n
			char *tmpstr;
			tmpstr = (char*)malloc(128);
			snprintf(tmpstr,128,"%s","AT+QLWCONF?\r\n");
			memset(recvbuf,0x0,RECV_BUF_LEN);
			uart_data_write(tmpstr,strlen(tmpstr),0);
			uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
			free(tmpstr);
		}
		
		{
			char *tmpstr;
			tmpstr = (char*)malloc(128);
			snprintf(tmpstr,128,"%s","AT+QLWADDOBJ=19,0,1,\"0\"\r\n");
			memset(recvbuf,0x0,RECV_BUF_LEN);
			uart_data_write(tmpstr,strlen(tmpstr),0);
			uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
			free(tmpstr);
		}
		
		{
			char *tmpstr;
			tmpstr = (char*)malloc(128);
			snprintf(tmpstr,128,"%s","AT+QLWOPEN=0\r\n");
			memset(recvbuf,0x0,RECV_BUF_LEN);
			uart_data_write(tmpstr,strlen(tmpstr),0);
			uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
			free(tmpstr);
		}
		
		{
			char *tmpstr;
			tmpstr = (char*)malloc(128);
			snprintf(tmpstr,128,"%s","AT+QLWCFG=\"dataformat\",1,1\r\n");
			memset(recvbuf,0x0,RECV_BUF_LEN);
			uart_data_write(tmpstr,strlen(tmpstr),0);
			uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
			free(tmpstr);
		}
		
		{
			char *tmp = malloc(512);
			make_json_data(tmp);
			memset(recvbuf,0x0,RECV_BUF_LEN);
			push_data_func((unsigned char*)tmp,strlen(tmp));
			
			for(;;)
			{
				uart_data_read(recvbuf, RECV_BUF_LEN, 0, 2000);
			}
			free(tmp);
		}
		
		printf("���ӽ�����ϣ���ʼ�����ƽ̨�������ݣ�waiting 4 second................\r\n");
		utimer_sleep(4000);
		
		
		
		
		
		/*
		�ͷ��ڴ�
		*/
		free(recvbuf);
		free(atbuf);
		free(jsonbuf);
	}
	
	printf("Sys_Enter_Standby CurrentTim %d\r\n",RTC_GetCounter());
	
	/*
	 * ����1Сʱ֮���ٴ�����������PSMģʽ
	 */
	RTC_SetAlarm(RTC_GetCounter() +  3600);

	//��������
	utimer_sleep(2000);
	Sys_Enter_Standby();

	return 0;
}





#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
	
  USART_SendData(USART1, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

