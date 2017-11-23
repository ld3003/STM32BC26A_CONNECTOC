#ifndef config_def_h__
#define config_def_h__

#define CONFNIG_RAW_DATA_SIZE 4

enum OTA_FLAG_ENUM {
	OTA_FLAG_INIT,
	OTA_FLAG_DOWNLOAD,
	OTA_FLAG_DOWNLOAD_FINISH,
};

struct CONFIG_DATA {
	char version;
	
	/*
	����ģʽ:
	0,GPRS
	1,GSM
	*/
	char workmode;
	
	
	char ipaddress[32];
	char ipport[8];
	
	char phonenum[13][3];
	
	/*
	�绰ģʽ
	1������
	2���绰
	3������+�绰
	*/
	char phonemode[3];
	
	//
};




#endif



