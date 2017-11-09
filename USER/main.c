/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The USART application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-10-30
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "ov2640api.h"

static void takephoto(void);

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
	
	SysTick_Config(SystemCoreClock / 100);
	watch_dog_config();
	init_uart1();
	init_uart2();
	RTC_Init();
	
	//����APP�Ѿ���������
	SET_BOOTLOADER_STATUS(2);
	
	WKUP_Pin_Init();
	watch_dog_config();
	init_utimer();
	init_task();
	mainloop_init();
	
	takephoto();
	
	init_mem();
	
	SET_SYSTEM_COUNTER;
		
	init_uart2_buffer();
	
	for(;;){feed_watchdog();mainloop();}
	
	return 0;
}

static void takephoto(void)
{
	//����ͷ���ղ��ֱ����ٳ���һ��ʼ���У���˽������⴦��
	//SET_SYSTEM_STATUS(SYSTEM_STATUS_TAKEPHOTH);
	switch(GET_SYSTEM_STATUS)
	{
		case SYSTEM_STATUS_TAKEPHOTH:
		{
			int i = 0;
			int initOV = -1;
			int photolen = 0;
			unsigned int takeTime;
			
			#define OV2640_REINIT_COUNT 3
			
			alloc_jpegbuffer();
			ov_poweron();
			_config_mco();
			InitSCCB();
			
			for(i=1;i<=OV2640_REINIT_COUNT;i++)
			{
				ov_poweron();
				initOV = ov2640_init();
				if (initOV >=0 )
					break;
				printf("��������ͷʧ�ܣ����� %d �� \r\n",i);
				ov_poweroff();
				utimer_sleep(1000);
			}
			
			if (initOV != 0)
				goto endtakephoto;
			
			photolen = ov2640_read();
			if (photolen > 0)
			{
				unsigned char *imgbuf;
				
				//�������ͷ����
				mdata.cam_ok = 1;
				
				//��ͼƬд��
				printf("д��flash\r\n");
				__write_img_2_flash(0,JpegBuffer,photolen,0);
				
				//����д���Ƿ���ȷ
				imgbuf = read_imgbuffer(0,&photolen,&takeTime);
				for(i=0;i<photolen;i++)
				{
					uart1_putchar(imgbuf[i]);
					//
				}
			}
			
			endtakephoto:
			
			free_jpegbuffer();
			ov_poweroff();
			break;
		}
		
	}
	//
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

