#ifndef __setting_h__
#define __setting_h__

#define BUILDTIME __DATETIME__

// �汾�Ŷ���
#define MAJOR_VERSION_NUMBER			0
#define MINOR_VERSION_NUMBER			0

#define REVISION_NUMBER						0


#define BUILD_NUMBER							0

//ͼƬ�ְ���С
#define IMG_PKG_SIZE 							(512)

//���ش�����
#define PKG_RETRANS_COUNT					10
#define WAIT_UDP_PKG_TIME					10000

//��������Ϣ 
//#define SERVER_ADDR								"52.80.4.27"
//#define SERVER_PORT								29100
#define SERVER_ADDR							"47.93.103.232"
#define SERVER_PORT							29100

//�����ϴ�����
#define PAIZHAO_COUNT							1 //���մ���
#define PAIZHAO_INTERVAL					5 //����ʱ����

#define VOL_REF										3300 //ADC��ѹ��׼


#define FW_VERSION								0x0000

#define POWERUP_MIN_TIME					0 //������Ĭʱ��
#define ALARM_MIN_TIME					120
#define MOTIONLESS_TIME					120

#define __WAKEUP_DBUG

#endif

