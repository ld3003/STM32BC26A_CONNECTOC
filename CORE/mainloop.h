#ifndef __mainloop_h__
#define __mainloop_h__

#include "setting.h"

typedef enum {
	SYS_INIT,
	SYS_INIT2,
	SYS_INIT3,
	SYS_POWERON,
	SYS_POWEROFF,
	MODEM_POWEROFF,
	MODEM_RESET,
	MODEM_POWERACTIVE,
	MODEM_POWERACTIVE_TESTAT,
	
	MODEM_GET_IMEI,
	MODEM_CHECK_MODEM_TYPE,
	
	MODEM_POWERON,
	
	MODEM_CHECK_CSQ,
	
	MODEM_GPRS_READY,								//GPRS�Ѿ�׼����
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
	
} SYSTEM_STATUS;

enum {
	MODEM_TYPE_G510 = 0,
	MODEM_TYPE_M6312 = 1,
};

struct MAINLOOP_DATA {
	SYSTEM_STATUS status;								//״̬��
	unsigned char modemtype;
	
	//������
	unsigned char send_test_at_cnt;			//���Ͳ���AT�Ĵ���
	unsigned char check_1091_cnt;				//���1091���Ƿ�ظ��Ĵ���
	unsigned char modem_reset_cnt;			//ģ������������
	unsigned char test_at_cnt;					
	
	unsigned char pictype;							//ͼƬ���ͣ����ԣ�ʵ��?
	
	unsigned int time1;
	unsigned int time2;
	
	unsigned int paizhao_time;
	
	unsigned char upload_index;					//�ϴ�����
	unsigned char need_uload_cnt;				//��Ҫ�ϴ����ţ�
	
	unsigned short voltage;
	
};


extern struct MAINLOOP_DATA mdata;
void mainloop_init(void);							//�����ʼ��
void mainloop(void);									//����ѭ��

int push_data(unsigned char *data , int length , unsigned char *outdata , int timeout);
int push_data_m6312(unsigned char *data , int length , unsigned char *outdata , int timeout);
int push_data_A6(unsigned char *data , int length , unsigned char *outdata , int timeout);

#endif
