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
#define SERVER_ADDR								"52.80.4.27"
#define SERVER_PORT								29100
//#define SERVER_ADDR							"118.190.173.234"
//#define SERVER_PORT							8888

//�����ϴ�����
#define PAIZHAO_COUNT							1 //���մ���
#define PAIZHAO_INTERVAL					5 //����ʱ����

#define VOL_REF										3300 //ADC��ѹ��׼

#define FLASH_TRACE								//ʹ��flash��ΪTrace��¼��,�������������ע�͵�����Ϊ�����Ӵ������ģ�������flash��д����
//#define NO_ALARM								//�رշ��������

#define FW_VERSION								0x0000

//#define OLD_BRD

#endif