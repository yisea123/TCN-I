#ifdef __cplusplus	  
extern "C" {
#endif

#include "project.h"
#include "managerment.h"
#include "CPrintf.h"
#include "stdio.h"
#include "stdint.h"
	
#include "../HARDWARE/DCMotor/DCMotor.h"
#include "../HARDWARE/StepMotor/StepMotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../HARDWARE/NTCResistor/NTCResistor.h"
#include "../HARDWARE/RelayMOS/RelayMOS.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HARDWARE/LED/led.h"
	
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

//�н�1
//action : 0,�н��ɿ�; !=0,�н�
//stopFlag : 0,��������; !=0,�����˳�����
void Clamp1(uint8_t action, uint8_t *pStopFlag)
{
	PhSensorEnum_TypeDef sensor;
	Direction_TypeDef dir;
	
	if(action)
	{
		dir = CW;
		sensor = CLAMP1_SENSOR_L;
	}
	else
	{
		dir = CCW;
		sensor = CLAMP1_SENSOR_R;
	}
	
	PhSensor_SingleScan(sensor);
	if(!PhSensor_SingleCheck(sensor) || *pStopFlag)
		return;
	
	DCMotor_Run(CLAMP1_MOTOR, dir, 100);
	while(1)
	{
		PhSensor_SingleScan(sensor);
		if((!PhSensor_SingleCheck(sensor)) || *pStopFlag)
		{
			if(action && !*pStopFlag)
				vTaskDelay(pProjectMan->clamp1Delay);//80
			DCMotor_Stop(CLAMP1_MOTOR);
			break;
		}
		
		vTaskDelay(5);
	}
}

//�н�2
//action : 0,�н��ɿ�; !=0,�н�
//stopFlag : 0,��������; !=0,�����˳�����
void Clamp2(uint8_t action, uint8_t *pStopFlag)
{
	PhSensorEnum_TypeDef sensor;
	Direction_TypeDef dir;
	
	if(action)
	{
		dir = CW;
		sensor = CLAMP2_SENSOR_L;
	}
	else
	{
		dir = CCW;
		sensor = CLAMP2_SENSOR_R;
	}
	
	PhSensor_SingleScan(sensor);
	if(!PhSensor_SingleCheck(sensor) || *pStopFlag)
		return;
	
	DCMotor_Run(CLAMP2_MOTOR, dir, 100);
	while(1)
	{
		PhSensor_SingleScan(sensor);
		if((!PhSensor_SingleCheck(sensor)) || *pStopFlag)
		{
			if(action && !*pStopFlag)
				vTaskDelay(pProjectMan->clamp2Delay);//80
			DCMotor_Stop(CLAMP2_MOTOR);
			break;
		}
		
		vTaskDelay(5);
	}
}

//�ж�1
//action : 0,�ж��ɿ�; !=0,�ж�
//stopFlag : 0,��������; !=0,�����˳�����
void Cutoff1(uint8_t action, uint8_t *pStopFlag)
{
	PhSensorEnum_TypeDef sensor;
	Direction_TypeDef dir;
	uint8_t sensorFlag;
	
	if(action)
	{
		dir = CW;
		sensor = CUTOFF1_SENSOR_L;
		
		PhSensor_SingleScan(sensor);
		if(PhSensor_SingleCheck(sensor) || *pStopFlag)
			return;
	}
	else
	{
		dir = CCW;
		sensor = CUTOFF1_SENSOR_R;
		
		PhSensor_SingleScan(sensor);
		if(!PhSensor_SingleCheck(sensor) || *pStopFlag)
			return;
	}
	
	DCMotor_Run(CUTOFF1_MOTOR, dir, 100);
	while(1)
	{
		PhSensor_SingleScan(sensor);
		if(action)
			sensorFlag = PhSensor_SingleCheck(sensor);
		else
		{
			//sensorFlag = !PhSensor_SingleCheck(sensor);
			
			pProjectMan->timerExpireFlag[0] = 0;
			xTimerChangePeriod( pProjectMan->xTimerUser[0], pProjectMan->cutoffReturnDelay, 0);//4000/portTICK_PERIOD_MS
		
			while(!*pStopFlag && !pProjectMan->timerExpireFlag[0])
			{
				vTaskDelay(5);
			}
			if(*pStopFlag)
				xTimerStop( pProjectMan->xTimerUser[0], 0);
			
			sensorFlag = 1;
		}
		
		if(sensorFlag || *pStopFlag)//����������ر�һ��
		{
			if(action && !*pStopFlag)//��Ƭ���е��ɣ��ж�ʱҪ�ӳ�һ����ͣ
			{
				vTaskDelay(pProjectMan->cutoff1Delay);//330
			}
			
			DCMotor_Stop(CUTOFF1_MOTOR);
			break;
		}
		
		vTaskDelay(5);
	}
}

//�ж�2
//action : 0,�ж��ɿ�; !=0,�ж�
//stopFlag : 0,��������; !=0,�����˳�����
void Cutoff2(uint8_t action, uint8_t *pStopFlag)
{
	PhSensorEnum_TypeDef sensor;
	Direction_TypeDef dir;
	uint8_t sensorFlag;

	if(action)
	{
		dir = CW;
		sensor = CUTOFF2_SENSOR_L;
	}
	else
	{
		dir = CCW;
		sensor = CUTOFF2_SENSOR_R;
	}
	
	PhSensor_SingleScan(sensor);
	if(!PhSensor_SingleCheck(sensor) || *pStopFlag)
		return;
	
	DCMotor_Run(CUTOFF2_MOTOR, dir, 100);
	while(1)
	{
		PhSensor_SingleScan(sensor);
		if(action)
			sensorFlag = !PhSensor_SingleCheck(sensor);
		else
		{
			//sensorFlag = !PhSensor_SingleCheck(sensor);
			
			pProjectMan->timerExpireFlag[1] = 0;
			xTimerChangePeriod( pProjectMan->xTimerUser[1], pProjectMan->cutoffReturnDelay, 0);//4000/portTICK_PERIOD_MS
		
			while(!*pStopFlag && !pProjectMan->timerExpireFlag[1])
			{
				vTaskDelay(5);
			}
			if(*pStopFlag)
				xTimerStop( pProjectMan->xTimerUser[1], 0);
			
			sensorFlag = 1;
		}
		
		if(sensorFlag || *pStopFlag)
		{
			if(action && !*pStopFlag)//��Ƭ���е��ɣ��ж�ʱҪ�ӳ�һ����ͣ
			{
				vTaskDelay(pProjectMan->cutoff2Delay);//420
			}
			DCMotor_Stop(CUTOFF2_MOTOR);
			break;
		}
		
		vTaskDelay(5);
	}
}

#if 0
//����Ƭ����
//stopFlag : 0,��������; !=0,�����˳�����
void HeatingCutoff(uint8_t *pStopFlag)
{
	uint8_t flag1, flag2;
	flag1 = 0;
	flag2 = 0;
	
	DCMotor_Run(CUTOFF1HEATDCMOTOR1, CW, 100);
	DCMotor_Run(CUTOFF1HEATDCMOTOR2, CW, 100);
	DCMotor_Run(CUTOFF2HEATDCMOTOR1, CW, 100);
	DCMotor_Run(CUTOFF2HEATDCMOTOR2, CW, 100);
	flag1 = 0;
	flag2 = 0;
	while(1)
	{
		if((!flag1 && (adcTemp[0].temperature >= pProjectMan->cutoff1Temperature)) || *pStopFlag)
		{
			DCMotor_Stop(CUTOFF1HEATDCMOTOR1);
			DCMotor_Stop(CUTOFF1HEATDCMOTOR2);
			flag1 = 1;
		}
		if((!flag2 && (adcTemp[1].temperature >= pProjectMan->cutoff2Temperature)) || *pStopFlag)
		{
			DCMotor_Stop(CUTOFF2HEATDCMOTOR1);
			DCMotor_Stop(CUTOFF2HEATDCMOTOR2);
			flag2 = 1;
		}
		if(flag1 && flag2)
			break;
		
		if(flag1 && !flag2)
		{
			if(adcTemp[0].temperature <= pProjectMan->cutoff1Temperature)
			{
				DCMotor_Run(CUTOFF1HEATDCMOTOR1, CW, 100);
				DCMotor_Run(CUTOFF1HEATDCMOTOR2, CW, 100);
				flag1 = 0;
			}
		}
		else if(!flag1 && flag2)
		{
			if(adcTemp[1].temperature <= pProjectMan->cutoff2Temperature)
			{
				DCMotor_Run(CUTOFF2HEATDCMOTOR1, CW, 100);
				DCMotor_Run(CUTOFF2HEATDCMOTOR2, CW, 100);
				flag2 = 0;
			}
		}
		
		vTaskDelay(5);
	}
}

//����Ƭ����
//stopFlag : 0,��������; !=0,�����˳�����
void HeatingCutoff1(uint8_t *pStopFlag)
{
	DCMotor_Run(CUTOFF1HEATDCMOTOR1, CW, 100);
	DCMotor_Run(CUTOFF1HEATDCMOTOR2, CW, 100);
	while(1)
	{
		if((adcTemp[0].temperature >= pProjectMan->cutoff1Temperature) || *pStopFlag)
		{
			DCMotor_Stop(CUTOFF1HEATDCMOTOR1);
			DCMotor_Stop(CUTOFF1HEATDCMOTOR2);
			break;
		}
		vTaskDelay(5);
	}
}

//����Ƭ����
//stopFlag : 0,��������; !=0,�����˳�����
void HeatingCutoff2(uint8_t *pStopFlag)
{
	DCMotor_Run(CUTOFF2HEATDCMOTOR1, CW, 100);
	DCMotor_Run(CUTOFF2HEATDCMOTOR2, CW, 100);
	while(1)
	{
		if((adcTemp[1].temperature >= pProjectMan->cutoff2Temperature) || *pStopFlag)
		{
			DCMotor_Stop(CUTOFF2HEATDCMOTOR1);
			DCMotor_Stop(CUTOFF2HEATDCMOTOR2);
			break;
		}
		vTaskDelay(5);
	}
}
#endif

//����
//action : 0,����; !=0,����պ�
//stopFlag : 0,��������; !=0,�����˳�����
void Separation(uint8_t action, uint8_t *pStopFlag)
{
	PhSensorEnum_TypeDef sensor;
	Direction_TypeDef dir;
	
	if(action)
	{
		dir = CCW;
		sensor = SAPERATE_SENSOR_L;
	}
	else
	{
		dir = CW;
		sensor = SAPERATE_SENSOR_R;
	}
	
	PhSensor_SingleScan(sensor);
	if(!PhSensor_SingleCheck(sensor))
		return;
	
	DCMotor_Run(SAPERATE_MOTOR, dir, 100);
	while(1)
	{
		PhSensor_SingleScan(sensor);
		if(!PhSensor_SingleCheck(sensor) || *pStopFlag)
		{
			DCMotor_Stop(SAPERATE_MOTOR);
			break;
		}
		vTaskDelay(5);
	}
}

//��λ
//action : 0,��λ��λ; !=0,��λ
//stopFlag : 0,��������; !=0,�����˳�����
void Dislocation(uint8_t action, uint8_t *pStopFlag)
{
	PhSensorEnum_TypeDef sensor;
	Direction_TypeDef dir;
	
	if(action)
	{
		dir = CCW;
		sensor = DISPOS_SENSOR_L;
	}
	else
	{
		dir = CW;
		sensor = DISPOS_SENSOR_R;
	}
	
	PhSensor_SingleScan(sensor);
	if(!PhSensor_SingleCheck(sensor))
		return;
	
	StepMotor_SetDir(DISPOS_MOTOR, dir);
	StepMotor_SetSpeed(DISPOS_MOTOR, 9);
	StepMotor_SetCMD(DISPOS_MOTOR, ENABLE);
	while(1)
	{
		PhSensor_SingleScan(sensor);
		if(!PhSensor_SingleCheck(sensor) || *pStopFlag)
		{
			StepMotor_Stop(DISPOS_MOTOR);
			break;
		}
		vTaskDelay(5);
	}
}

//��λһ������
//action : 0,��λ��λ; !=0,��λ
//stopFlag : 0,��������; !=0,�����˳�����
void DislocationDistance(uint8_t action, uint32_t distance, uint8_t *pStopFlag)
{
	Direction_TypeDef dir;
	
	if(action)
		dir = CCW;
	else
		dir = CW;
	
	StepMotor_SetDir(DISPOS_MOTOR, dir);
	StepMotor_SetSpeed(DISPOS_MOTOR, 9);
	StepMotor_SetPluse(DISPOS_MOTOR, distance);
	StepMotor_SetCMD(DISPOS_MOTOR, ENABLE);
	while(1)
	{
		if(!StepMotor_IsStop(DISPOS_MOTOR) || *pStopFlag)
			break;
		
		vTaskDelay(5);
	}
}

//�۽Ӽ���
//stopFlag : 0,��������; !=0,�����˳�����
void Heating(uint8_t *pStopFlag)
{
	//�۽Ӽ���
	pProjectMan->fusingTempControlFlag = 0;
	pProjectMan->fusingRaisingTempControlFlag = 1;
	RELAY = 1;
	while(1)
	{
		if(adcTemp[2].temperature >= pProjectMan->fusingTemperature || *pStopFlag)
			break;
		vTaskDelay(5);
	}
	RELAY = 0;
	pProjectMan->fusingRaisingTempControlFlag = 0;
	pProjectMan->fusingTempControlFlag = 1;//�˳��¶ȿ��ƣ�׼������
}

//����Ƭ̧��
//action : 0,����Ƭ��λ; !=0,����Ƭ̧��
//stopFlag : 0,��������; !=0,�����˳�����
void HeatingUp(uint8_t action, uint8_t *pStopFlag)
{
	if(action)
	{
		DCMotor_Run(HEATING_MOTOR, CW, pProjectMan->heatingUpSpeed);//30
		
		pProjectMan->timerExpireFlag[2] = 0;
		xTimerChangePeriod( pProjectMan->xTimerUser[2], pProjectMan->heatingUpDelay, 0);
		while(!pProjectMan->timerExpireFlag[2] && !*pStopFlag)
			vTaskDelay(5);
		if(*pStopFlag)
		{
			xTimerStop( pProjectMan->xTimerUser[2], 0 );
		}
		
		DCMotor_Stop(HEATING_MOTOR);
	}
	else
	{
		PhSensor_SingleScan(HEATING_SENSOR_R);
		if(!PhSensor_SingleCheck(HEATING_SENSOR_R))
			return;
		
		DCMotor_Run(HEATING_MOTOR, CCW, pProjectMan->heatingDownSpeed);//30
		while(1)
		{
			PhSensor_SingleScan(HEATING_SENSOR_R);
			if(!PhSensor_SingleCheck(HEATING_SENSOR_R) || *pStopFlag)
			{
				vTaskDelay(pProjectMan->heatingDownDelay);
				
				DCMotor_Stop(HEATING_MOTOR);
				break;
			}
			vTaskDelay(5);
		}
	}
}

//�븴λ�������ڼ�ͣ״̬��
void HalfResetOriginStatus(void)
{
	uint32_t cutoffReturnDelay = pProjectMan->cutoffReturnDelay;
	pProjectMan->cutoffReturnDelay = 500;
	
	//�жϻ�λ
	pProjectMan->cutoff1Action = 0;
	pProjectMan->cutoff2Action = 0;
	vTaskResume( pProjectMan->cutoff1TaskHandle );
	vTaskResume( pProjectMan->cutoff2TaskHandle );
	
	//�н��ɿ�
	pProjectMan->clamp1Action = 0;
	pProjectMan->clamp2Action = 0;
	vTaskResume( pProjectMan->clamp1TaskHandle );
	vTaskResume( pProjectMan->clamp2TaskHandle );
	
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	pProjectMan->cutoffReturnDelay = cutoffReturnDelay;
}

//�ָ�Ϊԭʼ״̬
//stopFlag : 0,��������; !=0,�����˳�����
void ResetOriginStatus(uint8_t *pStopFlag)
{
	pProjectMan->projectCurrentStatus = 10;
	
	//����
	pProjectMan->separationAction = 0;
	vTaskResume( pProjectMan->separationTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION, pdTRUE, pdFALSE, portMAX_DELAY);

	//���¼���Ƭ
	pProjectMan->heatingUpAction = 0;
	vTaskResume( pProjectMan->heatingUpTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_HEATINGUP, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//��λ��λ
	pProjectMan->dislocationAction = 0;
	vTaskResume( pProjectMan->dislocationTaskHandle );

	//�ж�
	pProjectMan->cutoff1Action = 1;
	pProjectMan->cutoff2Action = 1;
	vTaskResume( pProjectMan->cutoff1TaskHandle );
	vTaskResume( pProjectMan->cutoff2TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);

	//�жϻ�λ
	pProjectMan->cutoff1Action = 0;
	pProjectMan->cutoff2Action = 0;
	vTaskResume( pProjectMan->cutoff1TaskHandle );
	vTaskResume( pProjectMan->cutoff2TaskHandle );

	//�н��ɿ�
	pProjectMan->clamp1Action = 0;
	pProjectMan->clamp2Action = 0;
	vTaskResume( pProjectMan->clamp1TaskHandle );
	vTaskResume( pProjectMan->clamp2TaskHandle );
	
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_DISLOCATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//����պ�
	pProjectMan->separationAction = 1;
	vTaskResume( pProjectMan->separationTaskHandle );
	
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	pProjectMan->projectCurrentStatus = 0;
}

//�Զ�����1������
//stopFlag : 0,��������; !=0,�����˳�����
void AutoRun(uint8_t *pStopFlag)
{
	//�۽Ӽ���
	pProjectMan->fusingTempControlFlag = 0;
	pProjectMan->fusingRaisingTempControlFlag = 1;//���ȵ��۽��¶ȵ�

	//�н�
	pProjectMan->projectCurrentStatus = 1;
	pProjectMan->clamp1Action = 1;
	pProjectMan->clamp2Action = 1;
	vTaskResume( pProjectMan->clamp1TaskHandle );
	vTaskResume( pProjectMan->clamp2TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�жϼ���Ƭ�ﵽָ���¶�	
	//�жϣ�����ʱһ��ʱ���Զ���λ
	pProjectMan->projectCurrentStatus = 2;
	pProjectMan->cutoff1Action = 2;
	pProjectMan->cutoff2Action = 2;
	vTaskResume( pProjectMan->cutoff1TaskHandle );
	vTaskResume( pProjectMan->cutoff2TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	vTaskDelay(1000);
	
	//����
	pProjectMan->projectCurrentStatus = 3;
	pProjectMan->separationAction = 0;
	vTaskResume( pProjectMan->separationTaskHandle );
	
	//��λ
	pProjectMan->projectCurrentStatus = 4;
	pProjectMan->dislocationAction = 1;
	vTaskResume( pProjectMan->dislocationTaskHandle );
	
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_DISLOCATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�Ƿ���ȵ�ָ���¶�
	pProjectMan->projectCurrentStatus = 5;
#if !TEMPERATURE_MASK
	while(!pProjectMan->heatingUpFlag && !pProjectMan->projectStopFlag)
		vTaskDelay(5);
#endif
	pProjectMan->fusingTempControlFlag = 0;
	pProjectMan->fusingRaisingTempControlFlag = 0;//����ά���¶ȵ�
	
	//̧�����Ƭ
	pProjectMan->projectCurrentStatus = 6;
	pProjectMan->heatingUpAction = 1;
	vTaskResume( pProjectMan->heatingUpTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_HEATINGUP, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//��ʱ�۽�ʱ��
	pProjectMan->timerExpireFlag[2] = 0;
	xTimerChangePeriod( pProjectMan->xTimerUser[2], pProjectMan->fusingTime, 0);
	while(!pProjectMan->timerExpireFlag[2] && !pProjectMan->projectStopFlag)
		vTaskDelay(5);
	if(pProjectMan->projectStopFlag)
		xTimerStop( pProjectMan->xTimerUser[2], 0 );
	
	//���¼���Ƭ
	pProjectMan->heatingUpAction = 0;
	vTaskResume( pProjectMan->heatingUpTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_HEATINGUP, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//����պ�
	pProjectMan->projectCurrentStatus = 7;
	pProjectMan->separationAction = 1;
	vTaskResume( pProjectMan->separationTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//��ʱ�Ӻ�ʱ��
	pProjectMan->timerExpireFlag[2] = 0;
	xTimerChangePeriod( pProjectMan->xTimerUser[2], pProjectMan->jointTime, 0);
	while(!pProjectMan->timerExpireFlag[2] && !pProjectMan->projectStopFlag)
		vTaskDelay(5);
	if(pProjectMan->projectStopFlag)
		xTimerStop( pProjectMan->xTimerUser[2], 0 );
	
	//�н��ɿ�
	pProjectMan->projectCurrentStatus = 8;
	pProjectMan->clamp1Action = 0;
	pProjectMan->clamp2Action = 0;
	vTaskResume( pProjectMan->clamp1TaskHandle );
	vTaskResume( pProjectMan->clamp2TaskHandle );
	
	//��λһ��
	pProjectMan->dislocationAction = 2;
	vTaskResume( pProjectMan->dislocationTaskHandle );
	
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_DISLOCATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//��ʾȡ����ͨ��
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(TIPS1PAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 �뾡��ȡ����ͨ�ܣ�\r\n2 ��������˫�ֽ�����ͨ��");
	SetScreen(TIPS1PAGE_INDEX);
	SetTextValue(TIPS1PAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 �뾡��ȡ����ͨ�ܣ�\r\n2 ��������˫�ֽ�����ͨ��");
	pProjectMan->tipsBuzzeFlag = 1;
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	//�ȴ��жϻ�λ
	pProjectMan->projectCurrentStatus = 9;
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//��ʾȡ��ʣ��Ĺ�
	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
	SetTextValue(TIPS1PAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 �뾡��ȡ��ʣ��Ĺܣ�\r\n2 ע��:����ȴǰ��Ҫ������");
	SetScreen(TIPS1PAGE_INDEX);
	SetTextValue(TIPS1PAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 �뾡��ȡ��ʣ��Ĺܣ�\r\n2 ע��:����ȴǰ��Ҫ������");
	pProjectMan->tipsBuzzeFlag = 1;
	xSemaphoreGive(pProjectMan->lcdUartSem);
	
	pProjectMan->projectCurrentStatus = 80;
}

void AutoRun_Cutoff1(uint8_t *pStopFlag)
{
	//�н�
	pProjectMan->clamp2Action = 1;
	vTaskResume( pProjectMan->clamp2TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�жϣ�����ʱһ��ʱ���ٻ�λ
	pProjectMan->cutoff1Action = 2;
	vTaskResume( pProjectMan->cutoff1TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	
	vTaskDelay(1000);
	
	//����
	pProjectMan->separationAction = 0;
	vTaskResume( pProjectMan->separationTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�ȴ�������ʱ��ɣ���ʱ�ж��Ѿ���λ
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�н��ɿ�
	pProjectMan->clamp2Action = 0;
	vTaskResume( pProjectMan->clamp2TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2, pdTRUE, pdFALSE, portMAX_DELAY);
}

void AutoRun_Cutoff2(uint8_t *pStopFlag)
{
	//�н�
	pProjectMan->clamp1Action = 1;
	vTaskResume( pProjectMan->clamp1TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�жϣ�����ʱһ��ʱ���ٻ�λ
	pProjectMan->cutoff2Action = 2;
	vTaskResume( pProjectMan->cutoff2TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	vTaskDelay(1000);
	
	//����
	pProjectMan->separationAction = 0;
	vTaskResume( pProjectMan->separationTaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�ȴ�������ʱ��ɣ���ʱ�ж��Ѿ���λ
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2, pdTRUE, pdFALSE, portMAX_DELAY);
	
	//�н��ɿ�
	pProjectMan->clamp1Action = 0;
	vTaskResume( pProjectMan->clamp1TaskHandle );
	xEventGroupWaitBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1, pdTRUE, pdFALSE, portMAX_DELAY);
}

void ProjectTask(void)
{
	uint32_t j;
	static uint8_t systemPreheatingFlag = 0;
	
//	while(!pProjectMan->lcdNotifyResetFlag)
//		vTaskDelay(100);
//	
//	pProjectMan->lcdNotifyResetFlag = 0;
	ResetDevice();//��λ������
	ResetDevice();//��λ������
	
	for(j=0;j<336;j++);
	//for(j=0;j<65536;j++);
	//for(j=0;j<65536;j++);
	
	//������������
	SetHandShake();//������������
	SetHandShake();//�������������һ����������ᶪʧ
	SetHandShake();//������������
	
	while(!pProjectMan->lcdNotifyResetFlag)
		vTaskDelay(100);
	
#ifndef STM32SIM	
	while(1)
	{
		if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0))
		{
			static uint8_t flag = 0;
			if(!flag)
			{
				flag = 1;
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetTextValue(TIPSPAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"���ɿ���ͣ��ť��");
				SetScreen(TIPSPAGE_INDEX);
				xSemaphoreGive(pProjectMan->lcdUartSem);
			}
		}
		else
			break;
		
		vTaskDelay(10);
	}
#endif
	
	//����ϴιػ�ǰ�а���ͣ�����ɿ��н����жϻ�λ
	if(pProjectMan->systemEmergencyFlag)
	{
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetTextValue(TIPS1PAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 �뽫�豸�ϵĹ������ߣ�\r\n2 ����������ֶ���λ��");
		SetScreen(TIPS1PAGE_INDEX);
		xSemaphoreGive(pProjectMan->lcdUartSem);
		
		HalfResetOriginStatus();
		
		pProjectMan->tipsBuzzeFlag = 2;//��������
		
		pProjectMan->systemEmergencyFlag = 0;
		AT24CXX_Write(EMERGENCYFLAG_BASEADDR, (uint8_t*)&pProjectMan->systemEmergencyFlag, 1);
		pProjectMan->autoButtonFlag = 1;
		
		//vTaskDelay(1000);
	}
	else
	{
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳ��λ�С���");
		SetScreen(LOGOPAGE_INDEX);
		xSemaphoreGive(pProjectMan->lcdUartSem);
#ifndef STM32SIM
		ResetOriginStatus(&pProjectMan->projectStopFlag);
#endif
		pProjectMan->autoButtonFlag = 0;
		
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳԤ���С���");
		SetScreen(LOGOPAGE_INDEX);
		xSemaphoreGive(pProjectMan->lcdUartSem);
	}
	
	pProjectMan->systemPowerUpFlag = 2;
	
//	while(1)
//	{
//		if((adcTemp[0].temperature >= pProjectMan->cutoff1Temperature)
//			&& (adcTemp[1].temperature >= pProjectMan->cutoff2Temperature)
//			&& (adcTemp[2].temperature >= pProjectMan->fusingHoldingTemperature))
//		{
//			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
//			SetTextInt32(MAINPAGE_INDEX, MAIN_OUTPUT_EDIT, pProjectMan->totalOutput, 0, 0);
//			SetScreen(MAINPAGE_INDEX);
//			xSemaphoreGive(pProjectMan->lcdUartSem);	
//			break;
//		}
//		else
//			vTaskDelay(10);
//	}
	
	while(1)
	{
#if 1	
		if((pProjectMan->systemPowerUpFlag == 2) && (pProjectMan->tipsBuzzeFlag == 0))
		{
			if(!systemPreheatingFlag)
			{
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳԤ���С���");
				SetScreen(LOGOPAGE_INDEX);
				xSemaphoreGive(pProjectMan->lcdUartSem);
				
				systemPreheatingFlag = 1;
			}
			
#ifndef STM32SIM			
			if((adcTemp[0].temperature >= pProjectMan->cutoff1Temperature)
				&& (adcTemp[1].temperature >= pProjectMan->cutoff2Temperature)
				&& (adcTemp[2].temperature >= pProjectMan->fusingHoldingTemperature))
#endif
			{
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetTextInt32(MAINPAGE_INDEX, MAIN_OUTPUT_EDIT, pProjectMan->totalOutput, 0, 0);
#ifndef STM32SIM				
				SetScreen(MAINPAGE_INDEX);
#else
				SetScreen(SETUPPAGE_INDEX);
#endif
				xSemaphoreGive(pProjectMan->lcdUartSem);

				pProjectMan->systemPowerUpFlag = 1;
			}
		}
		
		if(pProjectMan->projectStatus&PROJECT_RUNNING)
		{
			switch(pProjectMan->projectStatus&0x7F)
			{
				case PROJECT_MANUAL_CLAMP1CW:
#ifndef STM32SIM
					Clamp1(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CLAMP1CW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);				
				break;
				case PROJECT_MANUAL_CLAMP1CCW:
#ifndef STM32SIM
					Clamp1(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CLAMP1CCW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_CLAMP2CW:
#ifndef STM32SIM
					Clamp2(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CLAMP2CW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_CLAMP2CCW:
#ifndef STM32SIM
					Clamp2(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CLAMP2CCW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_CUTOFF1CW:
#ifndef STM32SIM
					Cutoff1(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CUTOFF1CW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_CUTOFF1CCW:
#ifndef STM32SIM
					Cutoff1(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CUTOFF1CCW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_CUTOFF2CW:
#ifndef STM32SIM
					Cutoff2(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CUTOFF2CW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_CUTOFF2CCW:
#ifndef STM32SIM
					Cutoff2(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_CUTOFF2CCW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_HEATINGUP:
#ifndef STM32SIM
					HeatingUp(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_HEATINGUP_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_HEATINGDOWN:
#ifndef STM32SIM
					HeatingUp(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_HEATINGDOWN_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_SEPATATIONCW:
#ifndef STM32SIM
					Separation(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_SEPARATECW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_SEPATATIONCCW:
#ifndef STM32SIM
					Separation(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_SEPARATECCW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_DISLOCATIONCW:
#ifndef STM32SIM
					Dislocation(1, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_DISLOCATECW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_MANUAL_DISLOCATIONCCW:
#ifndef STM32SIM
					Dislocation(0, &(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(MANUALPAGE_INDEX, MANUAL_DISLOCATECCW_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				
				case PROJECT_STATUS_CUTOFF1:
#ifndef STM32SIM
					AutoRun_Cutoff1(&(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(STATUSPAGE_INDEX, STATUS_CUTOFF1_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_STATUS_CUTOFF2:
#ifndef STM32SIM
					AutoRun_Cutoff2(&(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(STATUSPAGE_INDEX, STATUS_CUTOFF2_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				
				case PROJECT_STATUS_RESET:
#ifndef STM32SIM
					ResetOriginStatus(&(pProjectMan->projectStopFlag));
#endif
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(STATUSPAGE_INDEX, STATUS_RESET_BUTTON, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
				case PROJECT_STATUS_AUTO:
					if(pProjectMan->usableTimes > 0)
					{
#ifndef STM32SIM
						AutoRun(&(pProjectMan->projectStopFlag));
#endif
						pProjectMan->totalOutput++;
						if(pProjectMan->totalOutput > 0xFFFFFFFF)
							pProjectMan->totalOutput = 0;
						
//						pProjectMan->usableTimes--;
//						AT24CXX_Write(USABLETIMES_BASEADDR, (uint8_t*)&pProjectMan->usableTimes, 2);

						AT24CXX_Write(TOTALOUTPUT_BASEADDR, (uint8_t*)&pProjectMan->totalOutput, 4);
					}
					
					xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
					SetButtonValue(STATUSPAGE_INDEX, STATUS_AUTO_BUTTON, 0);
					SetTextInt32(MAINPAGE_INDEX, MAIN_OUTPUT_EDIT, pProjectMan->totalOutput, 0, 0);
					xSemaphoreGive(pProjectMan->lcdUartSem);
				break;
			}
			xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
			if((pProjectMan->projectStatus&0x7F) != PROJECT_STATUS_RESET)
				pProjectMan->autoButtonFlag = 1;
			else
				pProjectMan->autoButtonFlag = 0;
			pProjectMan->projectStatus = 0;
			pProjectMan->projectStopFlag = 0;
			pProjectMan->autoNResetFlag = 0;
			xSemaphoreGive(pProjectMan->projectStatusSem);
		}
		else
			vTaskDelay(10);
#endif
		
		LED2Task(); //LED2��˸ָʾPROJECT�߳���������	
	}
}

#ifdef __cplusplus
}
#endif
