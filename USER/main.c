
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
	char tmpstr[32];
 

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
	cJSON_AddStringToObject(pJsonRoot, "BODY", "Current temperature 26'C");
	
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

static int make_send_data_str(char *outstr , unsigned char *data , int length)
{
	//AT+QSOSEND=0,5,3132333435\r\n
	char *tmp = malloc(1024);
	conv_hex_2_string((unsigned char*)data,length,tmp);
	sprintf(outstr,"AT+QSOSEND=0,%d,%s\r\n",length,tmp);
	free(tmp);
	printf("SEND : %s \r\n",outstr);
	return 0;
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
	
	
	while(neul_bc26_get_netstat()<0){};										//等待连接上网络
	
	{
		
		/*
		 * 分配内存
		 */
		#define RECV_BUF_LEN 1024
		char *recvbuf = malloc(RECV_BUF_LEN);
		char *atbuf = malloc(1024);
		char *jsonbuf = malloc(512);
		
		
		
		/*
		 * 发送AT指令
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT\r\n", strlen("AT\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * 打开PSM
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+CPSMS=1\r\n", strlen("AT+CPSMS=1\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * 关闭回显
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("ATE0\r\n", strlen("ATE0\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);

		/*
		 * 获取信号值
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+CSQ\r\n", strlen("AT+CSQ\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * 获取设备ID IMEI
		 */
		
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+CGSN=1\r\n", strlen("AT+CGSN=1\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		if (strstr(recvbuf,"+CGSN:"))
		{
			MYDEVICEID[15]=0;
			memcpy(MYDEVICEID,recvbuf+9,15);
			printf("IMEI: %s\r\n",MYDEVICEID);
		}
		
		/*
		 * 创建Socket
		 */
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+QSOC=1,2,1\r\n", strlen("AT+QSOC=1,2,1\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+QSOCON=0,39002,\"47.93.103.232\"\r\n", strlen("AT+QSOCON=0,39002,\"47.93.103.232\"\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		make_json_data(jsonbuf);
		make_send_data_str(atbuf,(unsigned char*)jsonbuf,strlen(jsonbuf));
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write(atbuf,strlen(atbuf),0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+QSODIS=0\r\n", strlen("AT+QSODIS=0\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		memset(recvbuf,0x0,RECV_BUF_LEN);
		uart_data_write("AT+QSOCL=0\r\n", strlen("AT+QSOCL=0\r\n"), 0);
		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		

		
		/*
		释放内存
		*/
		free(recvbuf);
		free(atbuf);
		free(jsonbuf);
	}
	
	printf("Sys_Enter_Standby CurrentTim %d\r\n",RTC_GetCounter());
	
	/*
	 * 设置1小时之后再次启动并进入PSM模式
	 */
	RTC_SetAlarm(RTC_GetCounter() +  3600);

	//进入休眠
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

