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

//��������Ϣ 
#define SERVER_ADDR								"easy-iot.cc"
#define SERVER_PORT								29100


#define FW_VERSION								0x0000

#define POWERUP_MIN_TIME					0 //������Ĭʱ��
#define ALARM_MIN_TIME					120
#define MOTIONLESS_TIME					120

#define SEND_IMGERROR_SLEEP_TIME (3600*4)

#define WAIT_UDP_PKG_TIME 5000
#define PKG_RETRANS_COUNT 3

#define __WAKEUP_DBUG

#endif

