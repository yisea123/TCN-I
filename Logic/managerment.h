#ifndef _MANAGERMENT_H_
#define _MANAGERMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "project.h"
#include "../UILogic/pageCommon.h"
#include "../HMI/cmd_queue.h"
#include "../HMI/cmd_process.h"
#include "../HMI/hmi_user_uart.h"
#include "../PID/PID.h"
	
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define SOFTWARETIMER_COUNT 2

//����EEPROM��ַ����
#define RESET_DEFAULT			0
#define RESET_DEFAULT_SIZE		4
#define LANGUAGE_BASEADDR 		8
#define LANGUAGE_SIZE 			1
#define CUTOFF1TEMP_BASEADDR	9
#define CUTOFF1TEMP_SIZE 		4
#define CUTOFF2TEMP_BASEADDR	13
#define CUTOFF2TEMP_SIZE 		4
#define FUSINGTEMP_BASEADDR		17
#define FUSINGTEMP_SIZE 		4
#define CUTOFFTIME_BASEADDR		21
#define CUTOFFTIME_SIZE 		4
#define FUSINGTIME_BASEADDR		25
#define FUSINGTIME_SIZE 		4
#define JOINTTIME_BASEADDR		29
#define JOINTTIME_SIZE 			4

//��������Ӧ����
#define CLAMP1_MOTOR		DCMOTOR1	//�н�1
#define CLAMP2_MOTOR		DCMOTOR2	//�н�2
#define CUTOFF1_MOTOR		DCMOTOR3	//�ж�1
#define CUTOFF2_MOTOR		DCMOTOR4	//�ж�2
#define	HEATING_MOTOR		DCMOTOR5	//����Ƭ̧��
#define	SAPERATE_MOTOR		DCMOTOR6	//����
#define	DISPOS_MOTOR		0			//��λ
#define CUTOFF1HEATDCMOTOR1 DCMOTOR9	//�ж�1����Ƭ1
#define CUTOFF1HEATDCMOTOR2 DCMOTOR10	//�ж�1����Ƭ2
#define CUTOFF2HEATDCMOTOR1 DCMOTOR11	//�ж�2����Ƭ1
#define CUTOFF2HEATDCMOTOR2 DCMOTOR12	//�ж�2����Ƭ2

//���崫������Ӧ����
#define CLAMP1_SENSOR_L		PHSENSOR16
#define CLAMP1_SENSOR_R		PHSENSOR15
#define CLAMP2_SENSOR_L		PHSENSOR14
#define CLAMP2_SENSOR_R		PHSENSOR13
#define CUTOFF1_SENSOR_L	PHSENSOR2
#define CUTOFF1_SENSOR_R	PHSENSOR1
#define CUTOFF2_SENSOR_L	PHSENSOR12
#define CUTOFF2_SENSOR_R	PHSENSOR11
#define HEATING_SENSOR_L	PHSENSOR4
#define HEATING_SENSOR_R	PHSENSOR3
#define SAPERATE_SENSOR_L	PHSENSOR8   //PHSENSOR10��ʱ��ⲻ��
#define SAPERATE_SENSOR_R	PHSENSOR9
#define DISPOS_SENSOR_L		PHSENSOR6
#define DISPOS_SENSOR_R		PHSENSOR5

//��Ŀ�������
#define PROJECT_TEST_CLAMP1CW		1
#define PROJECT_TEST_CLAMP1CCW		2
#define PROJECT_TEST_CLAMP2CW		3
#define PROJECT_TEST_CLAMP2CCW		4
#define PROJECT_TEST_CUTOFF1CW		5
#define PROJECT_TEST_CUTOFF1CCW		6
#define PROJECT_TEST_CUTOFF2CW		7
#define PROJECT_TEST_CUTOFF2CCW		8
#define PROJECT_TEST_SEPATATIONCW	9
#define PROJECT_TEST_SEPATATIONCCW	10
#define PROJECT_TEST_DISPOSITIONCW	11
#define PROJECT_TEST_DISPOSITIONCCW	12
#define PROJECT_TEST_HEATINGUP		13
#define PROJECT_TEST_HEATINGDOWN	14
#define PROJECT_PARAMETER_RESET		15
#define PROJECT_PARAMETER_AUTO		16

#define PROJECT_RUNNING				0x80

typedef enum
{
	TIPS_NONE,
	TIPS_CANCEL,
	TIPS_OK,
}TipsButton_TypeDef;

typedef enum
{
	English = 0,
	Chinese,
}Language_TypeDef;

/************************************************************************/
/* ��Ŀ����ṹ��                                                       */
/************************************************************************/
typedef struct
{
	uint8_t projectStatus; //bit7��ʾ���Գ����������У�bit6~bit0��Ӧ��Ŀ�������
	uint8_t projectStopFlag; //ǿ��ֹͣ���Բ���
	
	float cutoff1Temperature;  //�ж�1�¶�
	float cutoff2Temperature;  //�ж�2�¶�
	float fusingTemperature;  //�۽��¶�
	uint32_t cutoffTime;	//�ж�ʱ��
	uint32_t fusingTime;	//�۽�ʱ��
	uint32_t jointTime;		//�Ӻ�ʱ��
	
	uint8_t heating;
	
	uint8_t clamp2Flag;
	uint8_t cutoff2Flag;
	
	TimerHandle_t xTimerUser[SOFTWARETIMER_COUNT];
	uint8_t timerExpireFlag[SOFTWARETIMER_COUNT];

	PID_TypeDef cutoff1PID;
	PID_TypeDef cutoff2PID;
	
	Language_TypeDef lang;
}ProjectMan_TypeDef;

/************************************************************************/
/*                                                                      */
/************************************************************************/

extern ProjectMan_TypeDef *pProjectMan;
extern uint8 cmd_buffer[CMD_MAX_SIZE];

void initProjectMan(ProjectMan_TypeDef *pm);
void initUI(void);
void initSoftwareTimer(void);

void UITask(void);
	
#ifdef __cplusplus
}
#endif

#endif
