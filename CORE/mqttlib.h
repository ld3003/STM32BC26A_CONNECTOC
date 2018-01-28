#ifndef __MQTT_H__
#define __MQTT_H__

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef int int32_t;
typedef short int16_t;

#define INVALID_HANDLE_VALUE 0

#define MQTT_MSG_DUP 					(0x08)
#define MQTT_MSG_QOS_MASK				(0x06)
#define MQTT_MSG_QOS2 					(0x04)
#define MQTT_MSG_QOS1 					(0x02)
#define MQTT_MSG_RETAIN 				(0x01)
#define MQTT_CONNECT_FLAG_USER			(0x80)
#define MQTT_CONNECT_FLAG_PASSWD		(0x40)
#define MQTT_CONNECT_FLAG_WILLRETAIN	(0x20)
#define MQTT_CONNECT_FLAG_WILLQOS2		(0x10)
#define MQTT_CONNECT_FLAG_WILLQOS1		(0x08)
#define MQTT_CONNECT_FLAG_WILL			(0x04)
#define MQTT_CONNECT_FLAG_CLEAN			(0x02)
#define MQTT_SUBSCRIBE_QOS2				(0x02)
#define MQTT_SUBSCRIBE_QOS1				(0x01)

enum MQTTENUM
{
	MQTT_CMD_CONNECT = 1,
	MQTT_CMD_CONNACK,
	MQTT_CMD_PUBLISH,
	MQTT_CMD_PUBACK,
	MQTT_CMD_PUBREC,
	MQTT_CMD_PUBREL,
	MQTT_CMD_PUBCOMP,
	MQTT_CMD_SUBSCRIBE,
	MQTT_CMD_SUBACK,
	MQTT_CMD_UNSUBSCRIBE,
	MQTT_CMD_UNSUBACK,
	MQTT_CMD_PINGREQ,
	MQTT_CMD_PINGRESP,
	MQTT_CMD_DISCONNECT,
};

//�����ȿ��ƵĻ���ṹ
typedef struct
{
	uint8_t *Data;		//����ָ��
	uint32_t Pos;		//�������ֽ���
	uint32_t MaxLen;	//�û���������
}Buffer_Struct;

//�����ȿ��Ƶ�����ṹ�����ڶ���/ȡ������
typedef struct
{
	uint8_t *Char;		//�����ַ���
	uint8_t Qos;		//�����QOS��ֻ����0��MQTT_SUBSCRIBE_QOS2��MQTT_SUBSCRIBE_QOS1
}MQTT_SubscribeStruct;

//MQTT��ͷ������
typedef struct
{
	uint8_t *Data;		//��������ָ�룬����ʱֻ����CONNECT���ĵĿɱ䱨ͷ������ʱ�����CONNECT��PUBLISH���ĵĿɱ䱨ͷ
	uint8_t *String;	//��������������ָ�룬����ʱֻ����PUBLISH���ĵĿɱ䱨ͷ���������ڽ���
	uint32_t DataLen;	//�������ݳ��ȣ�CONNECT����10��PUBLISH���ģ����ⳤ��+2
	uint16_t PackID;	//���ı�ʶ����С�˸�ʽ
	uint8_t Cmd;		//MQTT���Ʊ��ĵ�����
	uint8_t Flag;		//����ָ�����Ʊ������͵ı�־λ
}MQTT_HeadStruct;

/**
 * @defgroup iot_mqtt_api MQTTЭ��API
 * @{
 */
/**�ڱ���MQTT���ĵĹ����У����ַ�����MQTTҪ����ӽ����ģ���֧�ִ�'\0'���ַ���������unicode-16��Ӣ��
*@param     Buf:   		�������ַ������ĵĻ���
*@param     String:		�ַ���
*@return    >0:    		�ɹ������ر����ı��ĳ���
            <=0:   		ʧ��
**/
uint32_t MQTT_AddUFT8String(Buffer_Struct *Buf, const uint8_t *String);

/**����MQTT����
*@param     Head:   	��Ҫ�����MQTT��ͷ�������˿ɱ䲿�ֺ͹̶�����
*@param     Payload:	��Ч�غ�����ָ��
*@param     PayloadLen:	��Ч�غɳ���
*@param     TxBuf:		��ű�����ĵĻ���
*@return    >0:    		�ɹ������ر����ı��ĳ���
            <=0:   		ʧ��
**/
uint32_t MQTT_EncodeMsg(MQTT_HeadStruct *Head, uint8_t *Payload, uint32_t PayloadLen, Buffer_Struct *TxBuf);

/**����MQTT����
*@param     Head:   		��Ž�����MQTT��ͷ�������˿ɱ䲿�ֺ͹̶�����
*@param     HeadDataLenMax:	��Ҫ���뱨�ĵı�ͷ��󳤶ȣ�����ʵ��������û���Ҫ���ݶ��ĵ����ⳤ������������
*@param     PayloadLen:		�������Ч�غɳ���
*@param     RxBuf:			��Ҫ����ı���ָ��
*@param     RxLen:			��Ҫ����ı��ĳ���
*@param     DealLen:		���ν����ʵ�ʳ��ȣ�ע�������ճ�������С�ڽ��յĳ���
*@return    >=0:    		�ɹ���������Ч�غ�����ָ�룬�����������Ч�غɣ��򷵻�0
            <0:   			ʧ��
**/
uint8_t* MQTT_DecodeMsg(MQTT_HeadStruct *Head, uint32_t HeadDataLenMax, uint32_t *PayloadLen, uint8_t *RxBuf, uint32_t RxLen, uint32_t *DealLen);

/**����CONNECT����
*@param     TxBuf:   		��ű�����ĵĻ���
*@param     PayloadBuf:		��ʱ�����Ч�غ����ݵĻ��棬�ⲿ�ṩ�������ڲ�ʹ��
*@param     Flag:			���ӱ�ʶ��0�������º궨������
							MQTT_CONNECT_FLAG_USER
							MQTT_CONNECT_FLAG_PASSWD
							MQTT_CONNECT_FLAG_WILLRETAIN
							MQTT_CONNECT_FLAG_WILLQOS2
							MQTT_CONNECT_FLAG_WILLQOS1
							MQTT_CONNECT_FLAG_WILL
							MQTT_CONNECT_FLAG_CLEAN
*@param     KeepTime:		��������ʱ��
*@param     ClientID:		�ͻ��˱�ʶ���ַ���������Ϊ��
*@param     WillTopic:		���������ַ���
*@param     User:			�û����ַ���
*@param     Passwd:			�����ַ���
*@param     WillMsgData:	������Ϣ����ָ��
*@param     WillMsgLen:		������Ϣ����
*@return    >0:    			�ɹ������ر����ı��ĳ���
            <=0:   			ʧ��
**/
uint32_t MQTT_ConnectMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint8_t Flag, uint16_t KeepTime,
		const uint8_t *ClientID,
		const uint8_t *WillTopic,
		const uint8_t *User,
		const uint8_t *Passwd,
		uint8_t *WillMsgData, uint16_t WillMsgLen);

/**����PUBLISH����
*@param     TxBuf:   		��ű�����ĵĻ���
*@param     Flag:			���ı�ʶλ��0�������º궨������
							MQTT_MSG_DUP
							MQTT_MSG_QOS_MASK
							MQTT_MSG_QOS2
							MQTT_MSG_QOS1
							MQTT_MSG_RETAIN
*@param     PackID:			���ı�ʶ����С�˸�ʽ
*@param     Topic:			�����ַ���
*@param     Payload:	 	��Ч�غ�
*@param     PayloadLen:		��Ч�غɳ���
*@return    >0:    			�ɹ������ر����ı��ĳ���
            <=0:   			ʧ��
**/
uint32_t MQTT_PublishMsg(Buffer_Struct *TxBuf, uint8_t Flag, uint16_t PackID, const int8_t *Topic,
		uint8_t *Payload, uint32_t PayloadLen);

/**����PUBLISH������QOS�������ͱ��ģ�����PUBACK��PUBREC��PUBREL��PUBCOMP
*@param     TxBuf:   		��ű�����ĵĻ���
*@param     Cmd:			��������
*@param     PackID:			���ı�ʶ����С�˸�ʽ
*@return    >0:    			�ɹ������ر����ı��ĳ���
            <=0:   			ʧ��
**/
uint32_t MQTT_PublishCtrlMsg(Buffer_Struct *TxBuf, uint8_t Cmd, uint16_t PackID);

/**����SUBSCRIBE����
*@param     TxBuf:   		��ű�����ĵĻ���
*@param     PayloadBuf:		��ʱ�����Ч�غ����ݵĻ��棬�ⲿ�ṩ�������ڲ�ʹ��
*@param     PackID:			���ı�ʶ����С�˸�ʽ
*@param     Topic:			��Ҫ���ĵ����⻺��ָ�룬�����ж������
*@param     TopicNum:		��������
*@return    >0:    			�ɹ������ر����ı��ĳ���
            <=0:   			ʧ��
**/
uint32_t MQTT_SubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum);

/**����UNSUBSCRIBE����
*@param     TxBuf:   		��ű�����ĵĻ���
*@param     PayloadBuf:		��ʱ�����Ч�غ����ݵĻ��棬�ⲿ�ṩ�������ڲ�ʹ��
*@param     PackID:			���ı�ʶ����С�˸�ʽ
*@param     Topic:			��Ҫȡ�����ĵ����⻺��ָ�룬�����ж������
*@param     TopicNum:		��������
*@return    >0:    			�ɹ������ر����ı��ĳ���
            <=0:   			ʧ��
**/
uint32_t MQTT_UnSubscribeMsg(Buffer_Struct *TxBuf, Buffer_Struct *PayloadBuf, uint16_t PackID, MQTT_SubscribeStruct *Topic, uint32_t TopicNum);

/**���������򵥱��ģ�ֻ����PINGREQ��DISCONNECT
*@param     TxBuf:   		��ű�����ĵĻ���
*@param     PayloadBuf:		��ʱ�����Ч�غ����ݵĻ��棬�ⲿ�ṩ�������ڲ�ʹ��
*@param     Cmd:			��������
*@return    >0:    			�ɹ������ر����ı��ĳ���
            <=0:   			ʧ��
**/
uint32_t MQTT_SingleMsg(Buffer_Struct *TxBuf, uint8_t Cmd);

/** @}*/

#endif
