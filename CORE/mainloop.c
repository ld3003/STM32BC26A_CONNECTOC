#include "mainloop.h"
#include "modem.h"
#include "serialport.h"
#include "task.h"
#include "at.h"
#include "utimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "udp_proto.h"
#include "dcmi_ov2640.h"
#include "rtc.h"
#include "mem.h"
#include "setting.h"
#include "testpoint.h"
#include "bsp.h"
#include "main.h"

#define MAINLOOP_STATUS_DELAY __time_100ms_cnt[TIMER_100MS_MAINLOOP_DELAY]

#if 0
SYS_INIT = 0,
	SYS_INIT2,
	SYS_INIT3,
	SYS_POWERON,
	SYS_POWEROFF,
	MODEM_POWEROFF,
	MODEM_RESET,
	MODEM_POWERACTIVE,
	MODEM_POWERACTIVE_TESTAT,
	MODEM_GET_IMEI,
	MODEM_ATD,
	MODEM_CHECK_MODEM_TYPE,
	MODEM_POWERON,
	MODEM_CPIN,
	MODEM_CHECK_CSQ,
	MODEM_GPRS_READY,								//GPRS�Ѿ�׼����
	MODEM_GPRS_CGDCONT,
	MODEM_GPRS_CGACT,
	MODEM_GPRS_CGATT,
	MODEM_GPRS_CGREG,
	MODEM_GPRS_MIPCALL_SUCCESS,			//��������ע��
	MODEM_GPRS_MIPOPEN_SUCCESS,			//����SOCKET����
	PROTO_CHECK_1091,
	PROTO_SEND_ALARM,
	PROTO_RECV_2091,
	START_SEND_IMG,
	PROTO_SEND_IMG,
	PROTO_SEND_IMG_SUCCESS,
	PROTO_SEND_IMG_ERROR,
	PROTO_UPLOAD_DONE_GOTO_SLEEP,
	PROTO_ALARM,
	MDATA_STATUS_NULL,
#endif

const struct STATUS_STR_ITEM sstr_item[] = {
	{MDATA_STATUS_NULL,"ģ���״γ�ʼ��\r\n"},
	{SYS_INIT,"��ʼ��\r\n"},
	{SYS_POWERON,"����ģ��\r\n"},
	{MODEM_RESET,"����ģ��\r\n"},
	{MODEM_CHECK_CSQ,"��ȡCSQ\r\n"},
	{MODEM_GPRS_CGDCONT,"����CGDCONT\r\n"},
	{MODEM_GPRS_CGACT,"����GPRS\r\n"},
	{MODEM_GPRS_CGATT,"��ѯGPRS\r\n"},
	{MODEM_GPRS_CGREG,"��ѯ��վ��Ϣ\r\n"},
	{PROTO_SEND_ALARM,"���ͱ���\r\n"},
};

static const char *get_status_str(unsigned char status)
{
	char i=0;
	for(i=0;i<(sizeof(sstr_item)/sizeof(struct STATUS_STR_ITEM));i++)
	{
		if (sstr_item[i].status == status)
		{
			return sstr_item[i].str;
		}
		//
	}
	
	return 0;
}

struct MAINLOOP_DATA mdata;

static int push_1091(void);
static int push_10A0(c_u16 alarmtype);
static int push_img_routing(void);
static void init_push_img(void);
static void set_img_data(void);

static void status_master(unsigned char status , unsigned int time)
{
	static unsigned char __history_status = MDATA_STATUS_NULL;
	static unsigned int current_status_time_ms = 0;
	
	//ι��
	
	if (status != __history_status)
	{
		printf("״̬��ת %d:%s ---> %d:%s \r\n",__history_status,get_status_str(__history_status),status,get_status_str(status));
		__history_status = status;
		mdata.status_running_cnt = 0;
		
		//
	}else{
		mdata.status_running_cnt ++;
		printf("״̬ %d ��ִ�е� %d ��\r\n",status,mdata.status_running_cnt);
	}
	
	
	
}

void mainloop_init(void)
{
	memset(&mdata,0x0,sizeof(mdata));
	mdata.status = SYS_INIT;
}

static void SYSINIT(void)
{
	switch(GET_SYSTEM_STATUS)
		{
			case SYSTEM_STATUS_INIT:
				IOI2C_Init();
				init_mma845x();
			
				//�����ǰ��RTCС��60��ô�����뱨������
				if (CURRENT_RTC_TIM < POWERUP_MIN_TIME)
				{
					
					printf("��ǰ����ʱ��С��%d��\r\n",POWERUP_MIN_TIME);
					printf("���볤�� %d ����������\r\n",POWERUP_MIN_TIME - CURRENT_RTC_TIM);
					SET_NEXT_WAKEUP_TIME(CURRENT_RTC_TIM + POWERUP_MIN_TIME);
					SET_SYSTEM_STATUS(SYSTEM_STATUS_DEEPSLEEP);
					Sys_Enter_DeepStandby();
					
				}
			
				SET_SYSTEM_STATUS(SYSTEM_STATUS_WAIT_WAKEUP);
				Sys_Enter_Standby();
				
				break;
			case SYSTEM_STATUS_DEEPSLEEP:
				
				if (CURRENT_RTC_TIM > GET_NEXT_WAKEUP_TIME)
				{
					printf("�˳�������ߣ�������ͨ����\r\n");
					
					SET_SYSTEM_STATUS(SYSTEM_STATUS_WAIT_WAKEUP);
					Sys_Enter_Standby();
					
				}else{
					printf("���������...\r\n");
					Sys_Enter_DeepStandby();
				}
				break;
			case SYSTEM_STATUS_WAIT_WAKEUP:
			{
				
				//��������
				
				if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))
				{
					//���ⲿ�жϻ���

					
					//������α��������С�� 120 �򲻽��뱨��
					if ((CURRENT_RTC_TIM - GET_LAST_ALARM_TIME) < ALARM_MIN_TIME)
					{
					
						printf("�������С��%d�룬��ǰʱ�� [%d] ���һ�α���ʱ�� [%d]\r\n",ALARM_MIN_TIME,CURRENT_RTC_TIM,GET_LAST_ALARM_TIME);
						
						//�������Ϊ�����ϴα���ʱ�䲻������ô���¸����ϴα���ʱ��
						
						SET_LAST_ALARM_TIME;
						
						SET_SYSTEM_STATUS(SYSTEM_STATUS_WAIT_WAKEUP);
						Sys_Enter_Standby();
						
						printf("���볤�� %d ����������\r\n",ALARM_MIN_TIME-10); // ��10�����ֵ
						SET_NEXT_WAKEUP_TIME(CURRENT_RTC_TIM + ALARM_MIN_TIME);
						SET_SYSTEM_STATUS(SYSTEM_STATUS_DEEPSLEEP);
						Sys_Enter_DeepStandby();
						
					}else{
						
						printf("���뱨��״̬����վ�ֹ�������λ\r\n");
						
						SET_MOTIONLESS_STATUS(0);
						
						mdata.doing = DOING_10A0;
						mdata._10a0type = 0;
						
						mdata.status = MODEM_POWEROFF;
						
						SET_LAST_ALARM_TIME;
						
						goto exit_standby;
					}
				}else{
					//���Ź����� / ���ڻ���
					
					if (GET_MOTIONLESS_STATUS == 0)
					{
						
						if ((CURRENT_RTC_TIM - GET_LAST_ALARM_TIME) > 120)
						{
							mdata.doing = DOING_10A0;
							mdata._10a0type = 1;
							mdata.status = MODEM_POWEROFF;
							
							SET_MOTIONLESS_STATUS(1);
							
							goto exit_standby;
							
						}else{
							printf("��� %d ��� �豸��Ȼ��ֹ���򴥷���ֹ���� \r\n",(120 - (CURRENT_RTC_TIM - GET_LAST_ALARM_TIME)));
						}
						
					}
					
					printf("��������...\r\n");
					Sys_Enter_Standby();
					
				}
				
				break; 
			}
			
			case SYSTEM_STATUS_RUN:
			
				
				
				break;
		}
		
		exit_standby:
		printf("�˳�����\r\n");
		return;
}

void mainloop(void)
{
	
	#define SYNC_STATUS_2_REG //__WRITE_STANDBY_STATUS(mdata.status);
	#define SET_STATUS(X) //__WRITE_STANDBY_STATUS(X);
	
	switch(mdata.status)
	{
		case SYS_INIT:
			status_master(mdata.status,60*1000);
		
			#if 1
			mdata.doing = DOING_10A0;
			mdata._10a0type = 1;
			mdata.status = MODEM_POWEROFF;
			#else
			SYSINIT();
			#endif
			
		
			break;
		
		case MODEM_RESET:
			status_master(mdata.status,60*1000);
		
			printf("����GPRSģ��,��ǰ�� %d ������\r\n",mdata.modem_reset_cnt);
		
			//��һЩ�ؼ�����������
			mdata.modem_reset_cnt ++;
			mdata.test_at_cnt = 0;
			
			mdata.status = MODEM_POWEROFF;
			break;
		
		case MODEM_POWEROFF:
			
			/*
					
			*/
		
		
			status_master(mdata.status,60*1000);
		
			printf("����GPRSģ��\r\n");
		
			gprs_modem_power_off();
			utimer_sleep(1000);
			gprs_modem_power_on();
			utimer_sleep(3000);
		
			mdata.status = MODEM_POWERACTIVE_TESTAT; //�������ATָ��Ļ���
		
			break;

		case MODEM_POWERACTIVE:
			status_master(mdata.status,60*1000);
		
			if (MAINLOOP_STATUS_DELAY >= 50) //5��
			{
				mdata.status = MODEM_POWERACTIVE_TESTAT;
				
				printf("mdata.status = MODEM_POWERACTIVE_TESTAT;\r\n");
			}
			break;
		case MODEM_POWERACTIVE_TESTAT:
		{
			int ret;
			
			status_master(mdata.status,60*1000);
			
			//ͨ����ģ�鷢��ATָ���жϷ��ؽ���Ƿ���ȷ���ж�ģ���Ƿ��Ѿ�����������
			
			ret = at_cmd_wait("ATE0\r\n",AT_AT,0,AT_WAIT_LEVEL_2);
			ret = at_cmd_wait("AT\r\n",AT_AT,0,AT_WAIT_LEVEL_2);
			
			mdata.test_at_cnt ++;
			
			if (ret == AT_RESP_OK)
			{
				//ģ��ATָ�����ȷ����Ϊģ���Ѿ�����������������һ��״̬
				printf("ģ�鿪���ɹ�,����10����ģ���ʼ�����\r\n");
				utimer_sleep(10000);
				mdata.status = MODEM_GET_IMEI;
				
			}else{
				
				//���ATָ��ز���ȷ�������·���ATָ����N�ζ�û���յ���������ģ��
				if (mdata.test_at_cnt > 3)
				{
					mdata.test_at_cnt = 0;

					//ģ��������3�ζ�û��ATָ��ͳɹ���������Ϊ�Ǵ�������ˣ������յ���Ϣ�ˣ���������ϵͳ
					printf("******************************************************\r\n");
					printf("*               ģ�鿪��ʧ��                         *\r\n");
					printf("******************************************************\r\n");
					//30���������
					//__SET_NEXT_WAKEUP_TIM(26);
					sys_shutdown();			
					
				}
			}
			break;
		}

		case MODEM_ATD:
			status_master(mdata.status,60*1000);
			break;
		case MODEM_GET_IMEI:
		{
			status_master(mdata.status,60*1000);
			at_cmd_wait_str("AT+GSN\r\n",AT_GSN,"OK",100);
			mdata.status = MODEM_CHECK_MODEM_TYPE;
			
			break;
		}
		case MODEM_CHECK_MODEM_TYPE:
		{
			
			status_master(mdata.status,60*1000);
			mdata.status = MODEM_POWERON;
			
			break;
		}
		
		case MODEM_POWERON:
		{
			int ret;
			
			status_master(mdata.status,60*1000);
			
			ret = at_cmd_wait_str("AT+CPIN?\r\n",AT_AT,"+CPIN: READY",300);
			
			if (ret == AT_RESP_OK)
			{
				//�����Ѿ�ע��ɹ��������ѯ�ź�����
				printf("���SIM���ɹ�\r\n");
				mdata.status = MODEM_CHECK_CSQ;
				
			}else{
				printf("���SIM��ʧ��,����ģ��\r\n");
				
				if (mdata.status_running_cnt > 5)
					mdata.status = MODEM_RESET;
				else
					utimer_sleep(1000); //�ȴ�1s���ٴ�ִ��
			}
			break;
		}
		
		case MODEM_CHECK_CSQ:
		{
			
			status_master(mdata.status,60*1000);
			at_cmd_wait("AT+CSQ\r\n",AT_CSQ,0,500);
			printf("��ǰ�ź����� : %d\r\n",gsm_signal);
			mdata.status = MODEM_GPRS_READY;
			
			break;
			//
		}
		
		case MODEM_GPRS_READY:
		{
			status_master(mdata.status,60*1000);
			mdata.status = MODEM_GPRS_CGDCONT;
			break;
		}
		
		case MODEM_GPRS_CGDCONT:
		{
			int ret;
			status_master(mdata.status,60*1000);
			ret = at_cmd_wait("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n",AT_CGDCONT,AT_WAIT,5000);
			if (ret == 0)
			{
				mdata.status = MODEM_GPRS_CGACT;
			}
			
			break;
		}
		case MODEM_GPRS_CGACT:
		{
			int ret;
			status_master(mdata.status,60*1000);
			ret = at_cmd_wait("AT+CGACT=1,1\r\n",AT_CGACT,AT_WAIT,5000);
			if (ret == 0)
			{
				mdata.status = MODEM_GPRS_CGATT;
			}else{
				printf("����GPRS CGACT ʧ��\r\n");
				if (mdata.status_running_cnt > 5)
					mdata.status = MODEM_RESET;
				else
					utimer_sleep(1000); //�ȴ�1s���ٴ�ִ��
			}
			break;
		}
		
		case MODEM_GPRS_CGATT:
		{
			int ret;
			status_master(mdata.status,60*1000);
			ret = at_cmd_wait_str_str("AT+CGATT=1\r\n","OK","OK",5000);
			if (ret == 0)
			{
				mdata.status = MODEM_GPRS_CGREG;
			}else{
				printf("��ѯ����GPRS CGATT ʧ��\r\n");
				if (mdata.status_running_cnt > 5)
					mdata.status = MODEM_RESET;
				else
					utimer_sleep(1000); //�ȴ�1s���ٴ�ִ��
			}
			break;
		}
		
		case MODEM_GPRS_CGREG:
		{
			int ret;
			status_master(mdata.status,60*1000);
			at_cmd_wait_str_str("AT+CGREG=2\r\n","OK","OK",5000);
			at_cmd_wait_str("AT+CGREG?\r\n",AT_CGREG,"OK",5000);
			mdata.status = MODEM_GPRS_MIPCALL_SUCCESS;
			break;
			
		}
		
		case MODEM_GPRS_MIPCALL_SUCCESS:
		{
			
			int ret;
			char *tmpstr;
			status_master(mdata.status,60*1000);
			
			tmpstr = (char*)alloc_mem(__FILE__,__LINE__,1024); 
			tmpstr[0] = 0x0;
			
			ret = at_cmd_wait_str_str("AT+CIPHEAD=1\r\n","OK","OK",100);
			snprintf(tmpstr,1024,"AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",SERVER_ADDR,SERVER_PORT);
			//AT+CIPHEAD=
			ret = at_cmd_wait(tmpstr,AT_IPSTART,AT_WAIT,10000);
			free_mem(__FILE__,__LINE__,(unsigned char *)tmpstr);

			if(ret == 0)
			{
				mdata.status = MODEM_GPRS_MIPOPEN_SUCCESS;
				printf("SERVER���ӳɹ�\r\n");
			}else{
				printf("SERVER ����ʧ��\r\n");
				if (mdata.status_running_cnt > 2)
					mdata.status = MODEM_RESET;
				else
					utimer_sleep(1000); //�ȴ�1s���ٴ�ִ��
			}
			
			break;
		}
		case MODEM_GPRS_MIPOPEN_SUCCESS:
		{
			status_master(mdata.status,60*1000);
			mdata.status = PROTO_SEND_ALARM;
			break;
		}
		
		case PROTO_SEND_ALARM:
			
			status_master(mdata.status,10*60*1000);
		
			if (push_10A0(mdata._10a0type) < 0)
			{
				printf("����10A0 ʧ��\r\n");
				if (mdata.status_running_cnt > 5)
				{
					mdata.status = MODEM_RESET;
				}
			}else{
				
				//
				printf("����10a0���ݳɹ�\r\n");
				SET_SYSTEM_STATUS(SYSTEM_STATUS_WAIT_WAKEUP);
				Sys_Enter_Standby();
				
				
			}
			
			break;
		case PROTO_CHECK_1091:
		{
			status_master(mdata.status,60*1000);
			break;
		}
		
		case START_SEND_IMG:
		{
			status_master(mdata.status,60*1000);
			init_push_img();
			set_img_data();
			update_message_id();
			mdata.status = PROTO_SEND_IMG;
			
			break;
			
		}
			
		case PROTO_SEND_IMG:
		{
			int ret;
			
			status_master(mdata.status,10*60*1000);
			
			ret = push_img_routing();

			switch(ret)
			{
				case 1:
					mdata.status = PROTO_SEND_IMG_SUCCESS;
					
					break;
				case 0:
					break;
				case -1:
					mdata.status = PROTO_SEND_IMG_ERROR;
					
					//
					break;
			}
			break;
		}
			//
		
		case PROTO_SEND_IMG_ERROR:
			
			status_master(mdata.status,60*1000);
			
			//���ͼƬ����ʧ�ܣ����ϴ�PIC
			printf("******************************************************\r\n");
			printf("           UPLOAD PHOTO ERROR!                       *\r\n");
			printf("******************************************************\r\n");
		
			mdata.status = MODEM_RESET;
			
		
			//���ģ��������3����Ȼû�������ϴ�����ô��������
			if (mdata.modem_reset_cnt > 3)
			{
				mdata.status = PROTO_UPLOAD_DONE_GOTO_SLEEP;
				
			}
			
			break;
		case PROTO_SEND_IMG_SUCCESS:
			
			status_master(mdata.status,60*1000);
		
			printf("******************************************************\r\n");
			printf("           UPLOAD PHOTO SUCCESS                       \r\n");
			printf("******************************************************\r\n");
		
			DEBUG_VALUE(mdata.upload_index);
			DEBUG_VALUE(mdata.need_uload_cnt);
		
			mdata.upload_index ++; //�ϴ��ɹ����ϴ�������+1

			printf("set prev photo time :\r\n");
		
			DEBUG_VALUE(mdata.paizhao_time);
			//__SET_LAST_PAIZHAOTIM(mdata.paizhao_time);
		
			if (mdata.upload_index < mdata.need_uload_cnt)
			{
				//����ϴ�����������������ϴ�
				mdata.status = START_SEND_IMG;
			}
			else
			{
				mdata.status = PROTO_UPLOAD_DONE_GOTO_SLEEP;
			}  
			break;
		case PROTO_UPLOAD_DONE_GOTO_SLEEP:
			
			status_master(mdata.status,60*1000);
			
			/**

			//���㲢������һ�ε�����ʱ��
			DEBUG_VALUE(RTC_GetCounter());
			DEBUG_VALUE(mdata.paizhao_time);
			DEBUG_VALUE(__GET_TIM1);
			
			if ((__GET_TIM1 >= (RTC_GetCounter() - mdata.paizhao_time)) && (RTC_GetCounter() >= mdata.paizhao_time))
			{
				unsigned int __wakeup_time = __GET_TIM1 - (RTC_GetCounter() - mdata.paizhao_time);	
				
				//��ȥһ��ϵͳ�������
				if (__wakeup_time > 3)
				{
					__wakeup_time -= 3;
				}
				DEBUG_VALUE(__wakeup_time);
				__SET_NEXT_WAKEUP_TIM(__wakeup_time);
			}
			else
			{
				__SET_NEXT_WAKEUP_TIM(0);
			}
			*/

			
			
			//��λϵͳ��������
			sys_shutdown();		
		
			break;
		case PROTO_ALARM:
			status_master(mdata.status,60*1000);
			break;
		default:
			status_master(mdata.status,60*1000);
			break;
	}
	


	//
}



static int push_1091(void)
{
	
	struct UDP_PROTO_2091_DATA *data2091p;
	struct UDP_PROTO_HDR *hdr;
	
	unsigned char __dd[] = {0x00,0x03,0x00,0x05,0x01};
	
	int ret;
	int ret1 = -1;
	int length,recvlen;
	
	
	unsigned char *sbuffer,*rbuffer;
	char *hexbuffer;
	char *atbuffer;
	
	sbuffer = alloc_mem(__FILE__,__LINE__,512);
	rbuffer = sbuffer;//alloc_mem(__FILE__,__LINE__,1024);
	
	
	length = make_0x1091(sbuffer,__dd,sizeof(__dd));
	
	
	recvlen = push_data_A6(sbuffer,length,rbuffer,WAIT_UDP_PKG_TIME);
	
	if (recvlen >= sizeof(struct UDP_PROTO_HDR))
	{
		char *hexbuffer = 0;
		
		data2091p = (struct UDP_PROTO_2091_DATA *)rbuffer;
	  hdr = (struct UDP_PROTO_HDR *)rbuffer;
		

		switch(hdr->cmdcode)
		{
			case 0x9120:
			{
				ret1 = 0;
				break;
			}
			case 0x00:
				break;
			default:
				break;
		}
		//
	}else{
		printf("RECV 2091 ERROR \r\n");
	}
	
	free_mem(__FILE__,__LINE__,(unsigned char*)sbuffer);
	//free_mem((unsigned char*)rbuffer);
	
	return ret1;
	//
}


static int push_10A0(c_u16 alarmtype)
{
	
	struct UDP_PROTO_HDR *hdr;
	
	
	int ret;
	int ret1 = -1;
	int length,recvlen;
	
	
	unsigned char *sbuffer,*rbuffer;
	char *hexbuffer;
	char *atbuffer;
	
	sbuffer = alloc_mem(__FILE__,__LINE__,512);
	rbuffer = sbuffer;//alloc_mem(__FILE__,__LINE__,1024);
	
	length = make_0x10A0(sbuffer,alarmtype,1,0);
	
	recvlen = push_data_A6(sbuffer,length,rbuffer,WAIT_UDP_PKG_TIME);
	
	if (recvlen >= sizeof(struct UDP_PROTO_HDR))
	{
		
	  hdr = (struct UDP_PROTO_HDR *)rbuffer;

		switch(hdr->cmdcode)
		{
			case 0xA020:
				printf("�������ɹ��յ� 10A0\r\n");
				ret1 = 0;
				break;
			case 0x00:
				break;
			default:
				break;
			
		}
		//
	}else{
		printf("RECV 2091 ERROR \r\n");
	}
	
	free_mem(__FILE__,__LINE__,(unsigned char*)sbuffer);
	//free_mem((unsigned char*)rbuffer);
	
	return ret1;
	//
}

static int push_10A1(void)
{
	
	struct UDP_PROTO_HDR *hdr;
	
	
	int ret;
	int ret1 = -1;
	int length,recvlen;
	
	
	unsigned char *sbuffer,*rbuffer;
	char *hexbuffer;
	char *atbuffer;
	
	sbuffer = alloc_mem(__FILE__,__LINE__,512);
	rbuffer = sbuffer;//alloc_mem(__FILE__,__LINE__,1024);
	
	length = make_0x10A0(sbuffer,1,1,0);
	
	recvlen = push_data_A6(sbuffer,length,rbuffer,WAIT_UDP_PKG_TIME);
	
	if (recvlen >= sizeof(struct UDP_PROTO_HDR))
	{
		
	  hdr = (struct UDP_PROTO_HDR *)rbuffer;

		switch(hdr->cmdcode)
		{
			case 0xA020:
				printf("�������ɹ��յ� 10A0\r\n");
				ret1 = 0;
				break;
			case 0x00:
				break;
			default:
				break;
			
		}
		//
	}else{
		printf("RECV 2091 ERROR \r\n");
	}
	
	free_mem(__FILE__,__LINE__,(unsigned char*)sbuffer);
	//free_mem((unsigned char*)rbuffer);
	
	return ret1;
	//
}

static int push_10B0(char *json)
{
	//int make_0x10B0(unsigned char *data , char *json)
	
	struct UDP_PROTO_HDR *hdr;
	
	
	int ret;
	int ret1 = -1;
	int length,recvlen;
	
	
	unsigned char *sbuffer,*rbuffer;
	char *hexbuffer;
	char *atbuffer;
	
	sbuffer = alloc_mem(__FILE__,__LINE__,512);
	rbuffer = sbuffer;//alloc_mem(__FILE__,__LINE__,1024);
	
	length = make_0x10B0(sbuffer,json);
	
	recvlen = push_data_A6(sbuffer,length,rbuffer,WAIT_UDP_PKG_TIME);
	
	if (recvlen >= sizeof(struct UDP_PROTO_HDR))
	{
		
	  hdr = (struct UDP_PROTO_HDR *)rbuffer;

		switch(hdr->cmdcode)
		{
			case 0xB020:
				printf("RECV 20A0 SUCCESS \r\n");
				ret1 = 0;
				break;
			case 0x00:
				break;
			default:
				break;
			
		}
		//
	}else{
		printf("RECV 20B0 ERROR \r\n");
	}
	
	free_mem(__FILE__,__LINE__,(unsigned char*)sbuffer);
	//free_mem((unsigned char*)rbuffer);
	
	return ret1;
	//
}

//-----------------------------------

struct PUSH_IMG_DATA {
	unsigned char status;
	unsigned char *imgdata;
	int img_total_len;
	int img_send_index;
	int seq;
	unsigned short seq_cnt;
	unsigned char send_err_cnt;
};

struct PUSH_IMG_DATA push_img_dat;

static void set_img_data(void)
{
	//���ݵ�ǰ�ϴ�����д�������
	
	extern unsigned int ___paizhaoshijian;
	
	//push_img_dat.imgdata = JpegBuffer;
	//push_img_dat.img_total_len = JpegDataCnt;
	unsigned int paizhaotime;
	//push_img_dat.imgdata = read_imgbuffer(mdata.upload_index,&push_img_dat.img_total_len,&paizhaotime);
	
	mdata.paizhao_time = paizhaotime;
	
	DEBUG_VALUE(mdata.upload_index);
	DEBUG_VALUE(push_img_dat.img_total_len);
	
	
	if((push_img_dat.img_total_len % IMG_PKG_SIZE) == 0)
	{
		push_img_dat.seq_cnt = (push_img_dat.img_total_len / IMG_PKG_SIZE);
	}else{
		push_img_dat.seq_cnt = (push_img_dat.img_total_len / IMG_PKG_SIZE) + 1;
	}
	printf("push_img_dat.seq_cnt %d %d %d %d\r\n",push_img_dat.seq_cnt , ((int)push_img_dat.img_total_len / IMG_PKG_SIZE) , push_img_dat.img_total_len , IMG_PKG_SIZE);
}

static void init_push_img(void)
{
	memset(&push_img_dat,0x0,sizeof(struct PUSH_IMG_DATA));
}

static int push_1092(unsigned short imgl , unsigned char *imgdata ,unsigned short frq_count , unsigned char imgtype ,  unsigned short img_frq)
{
	
	struct UDP_PROTO_2092_DATA *data2092p;
	struct UDP_PROTO_HDR *hdr;
	
	int ret;
	int ret1 = -1;
	int length;
	int recvlen;
	
	unsigned char *sbuffer; //�洢������������
	unsigned char *rbuffer;
	
	sbuffer = alloc_mem(__FILE__,__LINE__,sizeof(struct UDP_PROTO_2092_DATA) + imgl + 16);
	rbuffer = sbuffer;//alloc_mem(__FILE__,__LINE__,512);
	
	
	length = make_0x1092(imgl,imgdata,frq_count,imgtype,img_frq,sbuffer);
	
	recvlen = push_data_A6(sbuffer,length,rbuffer,WAIT_UDP_PKG_TIME);
	
	if (recvlen > 0)
	{
		//debug_buf("recv",data,length);
		
		hdr = (struct UDP_PROTO_HDR*)rbuffer;
		data2092p = (struct UDP_PROTO_2092_DATA*)rbuffer;
		
		if (recvlen >= sizeof(struct UDP_PROTO_HDR))
		{
			switch(hdr->cmdcode)
			{
				case 0x9220:
					transfer16(&data2092p->hdr.messageSeq);
					printf("RECV 2092 SUCCESS \r\n");
					printf("RECV SEQ  %d \r\n",data2092p->hdr.messageSeq);
					ret1 = 0;
					break;
				default:
					printf("RECV 2092 FORMAT ERROR \r\n");
					break;
				
			}
			//
		}else{
			printf("RECV 2092 ERROR \r\n");
		}
		
	}else{
		printf("wait 2092 error !\r\n");
	}
	
	free_mem(__FILE__,__LINE__,(unsigned char*)sbuffer);
	//free_mem((unsigned char*)rbuffer);
	
	return ret1;
	//
}

static int push_img_routing(void)
{
	int ret;
	int send_len;
	
	//��ȡͼƬ����
	
	if (push_img_dat.send_err_cnt > PKG_RETRANS_COUNT)
	{
		return -1; //����ʧ��
	}

	//��Ҫ����
	if (push_img_dat.img_total_len > push_img_dat.img_send_index)
	{
		if ((push_img_dat.img_total_len - push_img_dat.img_send_index) >= IMG_PKG_SIZE)
		{
			send_len = IMG_PKG_SIZE;
		}else{
			send_len = push_img_dat.img_total_len - push_img_dat.img_send_index;
		}
		
		ret = push_1092(send_len,push_img_dat.imgdata+push_img_dat.img_send_index,push_img_dat.seq_cnt,mdata.pictype,push_img_dat.seq + 1);
		if (ret == 0)
		{
			push_img_dat.send_err_cnt = 0;
			push_img_dat.seq ++;
			push_img_dat.img_send_index += send_len;
		}else{
			push_img_dat.send_err_cnt ++;
		}

		//
	}else{
		return 1;
	}
	
	
	return 0;
	//
}

int push_data_A6(unsigned char *data , int length , unsigned char *outdata , int timeout)
{
	extern unsigned char __debug_uart_flag;
	
	int ret = 0;
//	int recv_length = 0;
	
	char *tmpbuf = (char*)alloc_mem(__FILE__,__LINE__,32);

	snprintf(tmpbuf,32,"AT+CIPSEND=%d\r\n",length);
	send_at(tmpbuf);
	
	free_mem(__FILE__,__LINE__,(unsigned char*)tmpbuf);
	
	ret = at_cmd_wait_str_str(0,">",">",500);
	
	if (ret != 0)
	{
		printf("�ȴ����ͱ�־��ʱ \r\n");
		return -1;
	}
	
	send_data(data,length);
	
	ret = at_cmd_wait_str_str(0,"SEND","OK",timeout);
	
	if (ret != 0)
	{
		printf("����UDP����ʧ��\r\n");
		return -1;
	}else{
		printf("����UDP���ݳɹ�\r\n");
	}
	
	//������ջ������������ڴ�һ������
	if ((outdata > 0) && (ret == 0))
	{
		start_uart_debug();
		ret = at_cmd_wait_str_str(0,"+IPD,","+IPD,",5000);
		utimer_sleep(1000);
		stop_uart_debug();
		
		if (ret == 0)
		{
			
			char *colon,*dot;
			char tmpbuf[8];
			char x;
			int rlen;
			
			printf("�յ�����������\r\n");
			
			DEBUG_VALUE(0);
			
			colon = strstr(uart2_rx_buffer,":");
			dot = strstr(uart2_rx_buffer,",");
			
			printf("colon %02x dot %02x \r\n",(unsigned int)colon,(unsigned int)dot);
			
			if ((colon <= dot) || ((colon-dot) > 8))
			{
				ret = 0;
			}
			else
			{
			
				DEBUG_VALUE(0);
				
				for(x=0;x<(colon-dot-1);x++)
				{
					tmpbuf[x] = dot[x+1];
				}
				tmpbuf[x] = 0x0;
				
				DEBUG_VALUE(0);
				
				sscanf(tmpbuf,"%d",&rlen);
				memcpy(outdata,colon+1,rlen);
				
				DEBUG_VALUE(0);
				
				ret = rlen;
			}

		}else{
			printf("������û�����ݷ���\r\n");
			ret = 0;
		}
		
	}else{
		ret = -1;
	}
	
	return ret;
	
}



