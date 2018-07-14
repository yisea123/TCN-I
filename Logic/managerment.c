#include "managerment.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "CPrintf.h"
#include "../HARDWARE/DCMotor/dcmotor.h"
#include "../HARDWARE/StepMotor/stepmotor.h"
#include "../HARDWARE/Beep/beep.h"
#include "../HARDWARE/24CXX/24cxx.h"
#include "../HARDWARE/WDG/wdg.h"
#include "../HMI/hmi_driver.h"
#include "../HMI/hmi_user_uart.h"
#include "NTCResistor/NTCResistor.h"
#include "LED/led.h"
#include "RelayMOS/RelayMOS.h"
#include "../PID/PID.h"
#include "../Logic/misc.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif
	
/************************************************************************/
/* ������Ŀ����ṹ��                                                   */
/************************************************************************/
static ProjectMan_TypeDef ProjectMan;
ProjectMan_TypeDef *pProjectMan = &ProjectMan;

/************************************************************************/
/* ���ڽ��ջ�����                                                       */
/************************************************************************/
uint8 cmd_buffer[CMD_MAX_SIZE];
	
#define EEPROM_DEFAULT 0x11223345

//��ʱ���ص�����
void vTimerCallback( TimerHandle_t xTimer )
{
	// do something no block
	// ��ȡ��ʱ��ID
	uint32_t ulID = ( uint32_t ) pvTimerGetTimerID( xTimer );

	pProjectMan->timerExpireFlag[ulID] = 1;
	
	//ֹͣ��ʱ��
	xTimerStop( xTimer, 0 );
}

//��ʼ�������ʱ��
void initSoftwareTimer(void)
{
	uint32_t i;
	for(i=0;i<SOFTWARETIMER_COUNT;i++)
	{
		//���붨ʱ��
		pProjectMan->xTimerUser[i] = xTimerCreate
					   //��ʱ�����֣�ֻ�ڵ���ʱ����
					   ("Timer's name",
					   //������ڣ��������ó�10ms
					   10 / portTICK_PERIOD_MS,   
					   //�Ƿ��Զ����أ���������Ϊ�Զ�����
					   pdTRUE,
					   //����ʶ�����ĸ���ʱ�����ûص�����
					  ( void * ) i,
					   //�ص�����
					  vTimerCallback);

		 if( pProjectMan->xTimerUser[i] != NULL ) {
			//������ʱ����0��ʾ������
			xTimerStart( pProjectMan->xTimerUser[i], 0 );
		}
		else
		{
			while(1);
		}
	}
}

void initUI(void)
{
	//��ʼ����Ŀ�ṹ��
	//int16 i;
	uint32 j;
	uint32_t dat;

    UartInit(38400);

	PID_Init(&(pProjectMan->cutoff1PID), 8.0, 0.0, 0.0);
	PID_Init(&(pProjectMan->cutoff2PID), 8.0, 0.0, 0.0);
	
	GetChipID(pProjectMan->chipID);
	pProjectMan->serial = pProjectMan->chipID[0]+pProjectMan->chipID[1]+pProjectMan->chipID[2];
	srand(SysTick->VAL);
	pProjectMan->randomCode = rand();
	
#if 1    
	AT24CXX_Read(POWERONTEST_BASEADDR, (uint8_t*)(&dat), sizeof(uint32_t));//�Ƿ��һ�ο�������3��
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(POWERONTEST_BASEADDR, (uint8_t*)(&dat), sizeof(uint32_t));
	if(dat != EEPROM_DEFAULT)
		AT24CXX_Read(POWERONTEST_BASEADDR, (uint8_t*)(&dat), sizeof(uint32_t));

	if(dat != EEPROM_DEFAULT) //�ǣ���ʼ��EEPROM�е�����
	{
		cDebug("RESET_DEFAULT\n");

		pProjectMan->lang = English;
		AT24CXX_Write(LANGUAGE_BASEADDR, (uint8_t*)(&(pProjectMan->lang)), 1);

//		SetScreen(TIPS0PAGE_INDEX);//��ת����ʾ0ҳ��
//		//if(pProjectMan->lang == 0)
//			SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, (uint8_t*)"Restore factory settings����");
//		//else
//		//	SetTextValue(TIPS0PAGE_INDEX, TIPS0_TIPS_EDIT, "���ڻָ��������á���");
		
		//Ĭ������
		pProjectMan->cutoff1PID.Proportion = 8.0;
		pProjectMan->cutoff1PID.Integral = 0.0;
		pProjectMan->cutoff1PID.Derivative = 0.0;
		pProjectMan->cutoff2PID.Proportion = 8.0;
		pProjectMan->cutoff2PID.Integral = 0.0;
		pProjectMan->cutoff2PID.Derivative = 0.0;
		pProjectMan->cutoff1Temperature = 110.0;
		pProjectMan->cutoff2Temperature = 105.0;
		pProjectMan->fusingTemperature = 160.0;
		pProjectMan->fusingHoldingTemperature = 120;
		
		pProjectMan->clamp1Delay = 80;
		pProjectMan->clamp2Delay = 80;
		pProjectMan->cutoff1Delay = 150;
		pProjectMan->cutoff2Delay = 280;
		pProjectMan->cutoffReturnDelay = 4000;
		pProjectMan->cutoffTime = 30000;
		pProjectMan->fusingTime = 4000;
		pProjectMan->jointTime = 4000;
		pProjectMan->heatingUpDelay = 400;
		pProjectMan->heatingUpDelay = 100;
		
		pProjectMan->dislocateReturn = 11700;
		pProjectMan->dislocateSpeed = 9;
		pProjectMan->heatingUpSpeed = 30;
		pProjectMan->heatingDownSpeed = 30;
		pProjectMan->cutoff1HeatingVoltage = 24;
		pProjectMan->cutoff2HeatingVoltage = 24;
		pProjectMan->fusingHeatingVoltage = 24;
		
		pProjectMan->totalOutput = 0;
		
		pProjectMan->usableTimes = 1;
		
		pProjectMan->systemEmergencyFlag = 0;

#ifndef STM32SIM
		//��������
		AT24CXX_Write(CUTOFF1KP_BASEADDR, (uint8_t*)&pProjectMan->cutoff1PID.Proportion, 4);
		AT24CXX_Write(CUTOFF1KI_BASEADDR, (uint8_t*)&pProjectMan->cutoff1PID.Integral, 4);
		AT24CXX_Write(CUTOFF1KD_BASEADDR, (uint8_t*)&pProjectMan->cutoff1PID.Derivative, 4);
		AT24CXX_Write(CUTOFF2KP_BASEADDR, (uint8_t*)&pProjectMan->cutoff2PID.Proportion, 4);
		AT24CXX_Write(CUTOFF2KI_BASEADDR, (uint8_t*)&pProjectMan->cutoff2PID.Integral, 4);
		AT24CXX_Write(CUTOFF2KD_BASEADDR, (uint8_t*)&pProjectMan->cutoff2PID.Derivative, 4);
		AT24CXX_Write(CUTOFF1TEMP_BASEADDR, (uint8_t*)&pProjectMan->cutoff1Temperature, 4);
		AT24CXX_Write(CUTOFF2TEMP_BASEADDR, (uint8_t*)&pProjectMan->cutoff2Temperature, 4);
		AT24CXX_Write(FUSINGTEMP_BASEADDR, (uint8_t*)&pProjectMan->fusingTemperature, 4);
		AT24CXX_Write(FUSINGHOLDINGTEMP_BASEADDR, (uint8_t*)&pProjectMan->fusingHoldingTemperature, 4);
		
		AT24CXX_Write(CLAMP1DELAY_BASEADDR, (uint8_t*)&pProjectMan->clamp1Delay, 4);
		AT24CXX_Write(CLAMP2DELAY_BASEADDR, (uint8_t*)&pProjectMan->clamp2Delay, 4);
		AT24CXX_Write(CUTOFF1DELAY_BASEADDR, (uint8_t*)&pProjectMan->cutoff1Delay, 4);
		AT24CXX_Write(CUTOFF2DELAY_BASEADDR, (uint8_t*)&pProjectMan->cutoff2Delay, 4);
		AT24CXX_Write(CUTOFFRETURN_BASEADDR, (uint8_t*)&pProjectMan->cutoffReturnDelay, 4);		
		AT24CXX_Write(CUTOFFTIME_BASEADDR, (uint8_t*)&pProjectMan->cutoffTime, 4);
		AT24CXX_Write(FUSINGTIME_BASEADDR, (uint8_t*)&pProjectMan->fusingTime, 4);
		AT24CXX_Write(JOINTTIME_BASEADDR, (uint8_t*)&pProjectMan->jointTime, 4);
		AT24CXX_Write(HEATINGUPDELAY_BASEADDR, (uint8_t*)&pProjectMan->heatingUpDelay, 4);
		AT24CXX_Write(HEATINGDOWNDELAY_BASEADDR, (uint8_t*)&pProjectMan->heatingDownDelay, 4);
		
		AT24CXX_Write(DISLOCATERETURN_BASEADDR, (uint8_t*)&pProjectMan->dislocateReturn, 4);
		AT24CXX_Write(DISLOCATESPEED_BASEADDR, (uint8_t*)&pProjectMan->dislocateSpeed, 1);
		AT24CXX_Write(HEATINGUPSPEED_BASEADDR, (uint8_t*)&pProjectMan->heatingUpSpeed, 1);
		AT24CXX_Write(HEATINGDOWNSPEED_BASEADDR, (uint8_t*)&pProjectMan->heatingDownSpeed, 1);
		AT24CXX_Write(CUTOFF1HEATINGVOL_BASEADDR, (uint8_t*)&pProjectMan->cutoff1HeatingVoltage, 1);
		AT24CXX_Write(CUTOFF2HEATINGVOL_BASEADDR, (uint8_t*)&pProjectMan->cutoff2HeatingVoltage, 1);
		AT24CXX_Write(FUSINGHEATINGVOL_BASEADDR, (uint8_t*)&pProjectMan->fusingHeatingVoltage, 1);
		
		AT24CXX_Write(TOTALOUTPUT_BASEADDR, (uint8_t*)&pProjectMan->totalOutput, 4);
		
		AT24CXX_Write(USABLETIMES_BASEADDR, (uint8_t*)&pProjectMan->usableTimes, 2);
		
		AT24CXX_Write(EMERGENCYFLAG_BASEADDR, (uint8_t*)&pProjectMan->systemEmergencyFlag, 1);
		
		dat = EEPROM_DEFAULT;
		AT24CXX_Write(POWERONTEST_BASEADDR, (uint8_t*)&dat, sizeof(uint32_t));
#endif

		SetScreen(LOGOPAGE_INDEX);//��ת��LOGOҳ��
	}
	else //�񣬴�EEPROM�ж�ȡ����
	{
		cDebug("read data from EEPROM\n");

		AT24CXX_Read(LANGUAGE_BASEADDR, (uint8_t*)(&pProjectMan->lang), 1); //�������Բ���

//		if(pProjectMan->lang == 0)
//		{
//			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"System initializing����");
//		}
//		else
//			SetTextValue(LOGOPAGE_INDEX, LOGO_STATUS_EDIT, (uint8_t*)"ϵͳ��ʼ���С���");

		//��ȡ����
		AT24CXX_Read(CUTOFF1KP_BASEADDR, (uint8_t*)&pProjectMan->cutoff1PID.Proportion, 4);
		AT24CXX_Read(CUTOFF1KI_BASEADDR, (uint8_t*)&pProjectMan->cutoff1PID.Integral, 4);
		AT24CXX_Read(CUTOFF1KD_BASEADDR, (uint8_t*)&pProjectMan->cutoff1PID.Derivative, 4);
		AT24CXX_Read(CUTOFF2KP_BASEADDR, (uint8_t*)&pProjectMan->cutoff2PID.Proportion, 4);
		AT24CXX_Read(CUTOFF2KI_BASEADDR, (uint8_t*)&pProjectMan->cutoff2PID.Integral, 4);
		AT24CXX_Read(CUTOFF2KD_BASEADDR, (uint8_t*)&pProjectMan->cutoff2PID.Derivative, 4);
		AT24CXX_Read(CUTOFF1TEMP_BASEADDR, (uint8_t*)&pProjectMan->cutoff1Temperature, 4);
		AT24CXX_Read(CUTOFF2TEMP_BASEADDR, (uint8_t*)&pProjectMan->cutoff2Temperature, 4);
		AT24CXX_Read(FUSINGTEMP_BASEADDR, (uint8_t*)&pProjectMan->fusingTemperature, 4);
		AT24CXX_Read(FUSINGHOLDINGTEMP_BASEADDR, (uint8_t*)&pProjectMan->fusingHoldingTemperature, 4);
		
		AT24CXX_Read(CLAMP1DELAY_BASEADDR, (uint8_t*)&pProjectMan->clamp1Delay, 4);
		AT24CXX_Read(CLAMP2DELAY_BASEADDR, (uint8_t*)&pProjectMan->clamp2Delay, 4);
		AT24CXX_Read(CUTOFF1DELAY_BASEADDR, (uint8_t*)&pProjectMan->cutoff1Delay, 4);
		AT24CXX_Read(CUTOFF2DELAY_BASEADDR, (uint8_t*)&pProjectMan->cutoff2Delay, 4);
		AT24CXX_Read(CUTOFFRETURN_BASEADDR, (uint8_t*)&pProjectMan->cutoffReturnDelay, 4);		
		AT24CXX_Read(CUTOFFTIME_BASEADDR, (uint8_t*)&pProjectMan->cutoffTime, 4);
		AT24CXX_Read(FUSINGTIME_BASEADDR, (uint8_t*)&pProjectMan->fusingTime, 4);
		AT24CXX_Read(JOINTTIME_BASEADDR, (uint8_t*)&pProjectMan->jointTime, 4);
		AT24CXX_Read(HEATINGUPDELAY_BASEADDR, (uint8_t*)&pProjectMan->heatingUpDelay, 4);
		AT24CXX_Read(HEATINGDOWNDELAY_BASEADDR, (uint8_t*)&pProjectMan->heatingDownDelay, 4);
		
		AT24CXX_Read(DISLOCATERETURN_BASEADDR, (uint8_t*)&pProjectMan->dislocateReturn, 4);
		AT24CXX_Read(DISLOCATESPEED_BASEADDR, (uint8_t*)&pProjectMan->dislocateSpeed, 1);
		AT24CXX_Read(HEATINGUPSPEED_BASEADDR, (uint8_t*)&pProjectMan->heatingUpSpeed, 1);
		AT24CXX_Read(HEATINGDOWNSPEED_BASEADDR, (uint8_t*)&pProjectMan->heatingDownSpeed, 1);
		AT24CXX_Read(CUTOFF1HEATINGVOL_BASEADDR, (uint8_t*)&pProjectMan->cutoff1HeatingVoltage, 1);
		AT24CXX_Read(CUTOFF2HEATINGVOL_BASEADDR, (uint8_t*)&pProjectMan->cutoff2HeatingVoltage, 1);
		AT24CXX_Read(FUSINGHEATINGVOL_BASEADDR, (uint8_t*)&pProjectMan->fusingHeatingVoltage, 1);
		
		AT24CXX_Read(TOTALOUTPUT_BASEADDR, (uint8_t*)&pProjectMan->totalOutput, 4);
		
		AT24CXX_Read(USABLETIMES_BASEADDR, (uint8_t*)&pProjectMan->usableTimes, 2);
		
		AT24CXX_Read(EMERGENCYFLAG_BASEADDR, (uint8_t*)&pProjectMan->systemEmergencyFlag, 1);
	}
#endif
	
	PID_UpdateSetPoint(&(pProjectMan->cutoff1PID), pProjectMan->cutoff1Temperature);
	PID_UpdateSetPoint(&(pProjectMan->cutoff2PID), pProjectMan->cutoff2Temperature);
	
	/*��մ��ڽ��ջ�����*/
	queue_reset();

	//��ʱһ��ʱ��
	//for(j=0;j<65536;j++);
	//for(j=0;j<65536;j++);
	//for(j=0;j<65536;j++);
	
	cDebug("initUI success\n");

	//������������
	SetHandShake();//������������
	SetHandShake();//�������������һ����������ᶪʧ
	SetHandShake();//������������
	
	//SetScreen(MAINPAGE_INDEX);
	//SetScreen(STATUSPAGE_INDEX);
	
	//�����������״̬��ť
	for(j=1;j<=10;j++)
		SetControlEnable(MAINPAGE_INDEX, j, 0);

	//�������������״̬��ť
	for(j=1;j<=16;j++)
		SetControlEnable(INPUTPAGE_INDEX, j, 0);
}

void reflashMainPageButton(void)
{
	static uint8_t cnt = 0;
	static uint8_t flag = 0;
	static uint8_t lastStatus = 0;
	uint8_t j;
	
	cnt++;
//	if(lastStatus < pProjectMan->projectCurrentStatus)
//		cnt = 10;
	if(cnt > 10)
	{
		cnt = 0;
		
		xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
		if(pProjectMan->projectCurrentStatus == 0)
		{
			for(j=1;j<=10;j++)
				SetButtonValue(MAINPAGE_INDEX, j, 0);
			lastStatus = 0;
			xSemaphoreGive(pProjectMan->lcdUartSem);
			return;
		}
		else if(pProjectMan->projectCurrentStatus != 80)
		{
			if(pProjectMan->projectCurrentStatus == 2)
			{
				SetButtonValue(MAINPAGE_INDEX, 1, 1);
			}
			else if(pProjectMan->projectCurrentStatus == 4)
			{
				SetButtonValue(MAINPAGE_INDEX, 3, 1);
			}
			else if(pProjectMan->projectCurrentStatus == 6)
			{
				SetButtonValue(MAINPAGE_INDEX, 5, 1);
			}
			else if(pProjectMan->projectCurrentStatus == 10 && lastStatus == 0)
			{
				SetButtonValue(MAINPAGE_INDEX, 9, 1);
			}
			
			if(lastStatus < pProjectMan->projectCurrentStatus)
			{
				for(j=1;j<=lastStatus;j++)
					SetButtonValue(MAINPAGE_INDEX, j, 1);
			}
			else
			{
				flag = !flag;
				SetButtonValue(MAINPAGE_INDEX, pProjectMan->projectCurrentStatus, flag);
			}
			lastStatus = pProjectMan->projectCurrentStatus;

		}
		else
		{
			SetButtonValue(MAINPAGE_INDEX, lastStatus, 1);//9
		}
		
		if((pProjectMan->projectCurrentStatus == 9 || pProjectMan->projectCurrentStatus == 80)
				&& (pProjectMan->tipsBuzzeFlag == 1))
		{
			SetBuzzer(20);
		}
		
		xSemaphoreGive(pProjectMan->lcdUartSem);		
	}
	
	if(pProjectMan->tipsBuzzeFlag == 2)
	{
		SetBuzzer(20);
	}
//	
//	//�����������״̬��ť
//	xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
//	for(j=1;j<=10;j++)
//		SetControlEnable(MAINPAGE_INDEX, j, 0);
//	xSemaphoreGive(pProjectMan->lcdUartSem);
}

void Cutoff1PID(void)
{
	uint8_t pwmPercent;
	int32_t pidOut;

#if 0	
	if(adcTemp[0].temperature < pProjectMan->cutoff1Temperature)
	{
		if(pProjectMan->cutoff1Temperature - adcTemp[0].temperature > 10)
		{
			pwmPercent = 100;
		}
		else
		{
			PID_UpdateActualPoint(&(pProjectMan->cutoff1PID), adcTemp[0].temperature);
			pidOut = PID_Calc(&(pProjectMan->cutoff1PID));
			//pwmPercent += pidOut;
			//cDebug("---pidOut1 = %d\r\n", pidOut);
			if(pidOut > 5)
				pwmPercent = 100;
			else
				pwmPercent = (pidOut+5)*100/(5+5);
			//cDebug("---pwmPercent = %d\r\n", pwmPercent);
		}
	}
	else
	{
		if(adcTemp[0].temperature - pProjectMan->cutoff1Temperature > 5)
		{
			pwmPercent = 0;
		}
		else
		{
			PID_UpdateActualPoint(&(pProjectMan->cutoff1PID), adcTemp[0].temperature);
			pidOut = PID_Calc(&(pProjectMan->cutoff1PID));
			//pwmPercent += pidOut;
			//pwmPercent = pidOut;
			//cDebug("+++pidOut1 = %d\r\n", pidOut);
			if(pidOut < -5)
				pwmPercent = 0;
			else
				pwmPercent = (pidOut+5)*100/(5+5);
			//cDebug("+++pwmPercent = %d\r\n", pwmPercent);
		}
	}
	
	if(pwmPercent >= 100)
		pwmPercent = 100;
	else if(pwmPercent <= 0)
		pwmPercent = 0;
#else
	PID_UpdateActualPoint(&(pProjectMan->cutoff1PID), adcTemp[0].temperature);
	pidOut = PID_Calc(&(pProjectMan->cutoff1PID));
	if(pidOut > 0) pwmPercent = 100;
	else pwmPercent = 0;
#endif
		
	
	DCMotor_Run(CUTOFF1HEATDCMOTOR1, CW, pwmPercent);
	DCMotor_Run(CUTOFF1HEATDCMOTOR2, CW, pwmPercent);
	//cDebug("DCMotor Percent = %d\r\n", percent);
}

void Cutoff2PID(void)
{
	uint8_t pwmPercent;
	int32_t pidOut;

#if 0	
	if(adcTemp[1].temperature < pProjectMan->cutoff2Temperature)
	{
		if(pProjectMan->cutoff2Temperature - adcTemp[1].temperature > 10)
		{
			pwmPercent = 100;
		}
		else
		{
			PID_UpdateActualPoint(&(pProjectMan->cutoff2PID), adcTemp[1].temperature);
			pidOut = PID_Calc(&(pProjectMan->cutoff2PID));
			//pwmPercent += pidOut;
			//cDebug("---pidOut2 = %d\r\n", pidOut);
			if(pidOut > 5)
				pwmPercent = 100;
			else
				pwmPercent = (pidOut+5)*100/(5+5);
			//cDebug("---pwmPercent = %d\r\n", pwmPercent);
		}
	}
	else
	{
		if(adcTemp[1].temperature - pProjectMan->cutoff2Temperature > 5)
		{
			pwmPercent = 0;
		}
		else
		{
			PID_UpdateActualPoint(&(pProjectMan->cutoff2PID), adcTemp[1].temperature);
			pidOut = PID_Calc(&(pProjectMan->cutoff2PID));
			//pwmPercent += pidOut;
			//pwmPercent = pidOut;
			//cDebug("+++pidOut2 = %d\r\n", pidOut);
			if(pidOut < -5)
				pwmPercent = 0;
			else
				pwmPercent = (pidOut+5)*100/(5+5);
			//cDebug("+++pwmPercent = %d\r\n", pwmPercent);
		}
	}
	
	if(pwmPercent >= 100)
		pwmPercent = 100;
	else if(pwmPercent <= 0)
		pwmPercent = 0;
#else
	PID_UpdateActualPoint(&(pProjectMan->cutoff2PID), adcTemp[1].temperature);
	pidOut = PID_Calc(&(pProjectMan->cutoff2PID));
	if(pidOut > 0) pwmPercent = 100;
	else pwmPercent = 0;
#endif
	
	DCMotor_Run(CUTOFF2HEATDCMOTOR1, CW, pwmPercent);
	DCMotor_Run(CUTOFF2HEATDCMOTOR2, CW, pwmPercent);
}

void HeatingPID(void)
{
	if(!pProjectMan->fusingRaisingTempControlFlag)
	{
		if(adcTemp[2].temperature >= pProjectMan->fusingHoldingTemperature+5)
		{
			//RELAY = 0;
			DCMotor_Run(FUSINGHEATDCMOTOR, CW, 100);
		}
		else if(adcTemp[2].temperature <= (pProjectMan->fusingHoldingTemperature-5))
		{
			//RELAY = 1;
			DCMotor_Run(FUSINGHEATDCMOTOR, CW, 0);
		}
	}
	else
	{
		if(adcTemp[2].temperature >= pProjectMan->fusingTemperature)
		{
			//RELAY = 0;
			DCMotor_Run(FUSINGHEATDCMOTOR, CW, 100);
			pProjectMan->heatingUpFlag = 1;
		}
		else
		{
			//RELAY = 1;
			DCMotor_Run(FUSINGHEATDCMOTOR, CW, 0);
			pProjectMan->heatingUpFlag = 0;
		}
	}
}

void UITask(void)
{
    qsize size;
	
    while(1)
    {
        //����UI����
		size = queue_find_cmd(cmd_buffer, CMD_MAX_SIZE); //�ӻ�������ȡһ��ָ��
		if(size > 0)//���յ�ָ��
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//����ָ��
		}
		
        vTaskDelay(10);
		
        IWDG_Feed();//ι��
        
		NTCResistorTask(); //�����¶�ֵ
		
		vTaskDelay(10);
		
		IWDG_Feed();//ι��
		
		if(pProjectMan->inInputPageFlag)
			PhSensorScanTask();	//���������
		
		//������ҳ��״̬��ť
		reflashMainPageButton();
		
		LED1Task(); //LED1��˸ָʾUI�߳���������
		
		vTaskDelay(10);
		
		IWDG_Feed();//ι��
		
#if !TEMPERATURE_MASK
		//PID����
		if(!pProjectMan->cutoff1TempControlFlag)
			Cutoff1PID();
		if(!pProjectMan->cutoff2TempControlFlag)
			Cutoff2PID();
		if(!pProjectMan->fusingTempControlFlag)
			HeatingPID();
#endif
		
		//��������/��λ��ť��Ϣ
		if(pProjectMan->autoNResetFlag == 1)
		{
			//��λ
			if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				pProjectMan->projectStopFlag = 0;
				pProjectMan->projectStatus = PROJECT_STATUS_RESET;
				pProjectMan->projectStatus |= PROJECT_RUNNING;
				xSemaphoreGive(pProjectMan->projectStatusSem);
				
				DCMotor_Stop(STARTLAMP_MOTOR);
				
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				SetTextInt32(MAINPAGE_INDEX, MAIN_OUTPUT_EDIT, pProjectMan->totalOutput, 0, 0);
				SetScreen(MAINPAGE_INDEX);
				xSemaphoreGive(pProjectMan->lcdUartSem);
			}
		}
		else if(pProjectMan->autoNResetFlag == 2)
		{
			//�Զ�
			if(!(pProjectMan->projectStatus & PROJECT_RUNNING))
			{
				xSemaphoreTake(pProjectMan->projectStatusSem, portMAX_DELAY);
				pProjectMan->projectStopFlag = 0;
				pProjectMan->projectStatus = PROJECT_STATUS_AUTO;
				pProjectMan->projectStatus |= PROJECT_RUNNING;
				xSemaphoreGive(pProjectMan->projectStatusSem);

				DCMotor_Run(STARTLAMP_MOTOR, CW, 100);
				
				xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
				if(pProjectMan->usableTimes > 0)
				{
					SetTextInt32(MAINPAGE_INDEX, MAIN_OUTPUT_EDIT, pProjectMan->totalOutput, 0, 0);
					SetScreen(MAINPAGE_INDEX);
				}
				else
				{
					SetTextInt32(PERMITPAGE_INDEX, PERMIT_SERIAL_EDIT, pProjectMan->serial, 0, 0);
					SetTextInt32(PERMITPAGE_INDEX, PERMIT_RANDOMCODE_EDIT, pProjectMan->randomCode, 0, 0);
					SetTextInt32(PERMITPAGE_INDEX, PERMIT_USABLETIMES_EDIT, pProjectMan->usableTimes, 0, 0);
					SetScreen(PERMITPAGE_INDEX);
				}
				xSemaphoreGive(pProjectMan->lcdUartSem);
			}
		}

        vTaskDelay(10);
		
		//��ͣ��ť����
		if(pProjectMan->systemEmergencyButtonPressFlag)
		{
			//��ʾ�����ϵ�
			xSemaphoreTake(pProjectMan->lcdUartSem, portMAX_DELAY);
			//SetTextValue(TIPSPAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 ���ɿ���ͣ��ť��\r\n2 �������ϵ磡");
			SetTextValue(TIPSPAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"���ɿ���ͣ��ť��");
			SetScreen(TIPSPAGE_INDEX);
			xSemaphoreGive(pProjectMan->lcdUartSem);
			
			IWDG_Feed();//ι��
			pProjectMan->systemEmergencyFlag = 1;
			AT24CXX_Write(EMERGENCYFLAG_BASEADDR, (uint8_t*)(&(pProjectMan->systemEmergencyFlag)), 1);
			
			while(1)
			{
				if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0))
					IWDG_Feed();//ι��	
			}
		}
    }
}

void Clamp1Task(void * const pvParameters)
{
	while(1)
	{
		vTaskSuspend( NULL );
		Clamp1(*(uint8_t *)pvParameters, &(pProjectMan->projectStopFlag));
		xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP1);
	}
}

void Clamp2Task(void * const pvParameters)
{
	while(1)
	{
		vTaskSuspend( NULL );
		Clamp2(*(uint8_t *)pvParameters, &(pProjectMan->projectStopFlag));
		xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CLAMP2);
	}
}

void Cutoff1Task(void * const pvParameters)
{
	uint8_t action;
	while(1)
	{
		vTaskSuspend( NULL );
		action = *(uint8_t *)pvParameters;
		if(action == 0)
		{
			Cutoff1(0, &(pProjectMan->projectStopFlag));
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1);
		}
		else if(action == 1)
		{
			Cutoff1(1, &(pProjectMan->projectStopFlag));
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1);
		}
		else if(action == 2)
		{
			float temp = pProjectMan->cutoff1Temperature;
#if !TEMPERATURE_MASK
			while(adcTemp[0].temperature < pProjectMan->cutoff1Temperature && !pProjectMan->projectStopFlag)//�ȴ��¶ȵ�
				vTaskDelay(5);
#endif
			
			Cutoff1(1, &(pProjectMan->projectStopFlag));
			pProjectMan->cutoff1Temperature = 0;//Ѹ�ٽ���
			
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1);
			
			pProjectMan->timerExpireFlag[0] = 0;
			xTimerChangePeriod( pProjectMan->xTimerUser[0], pProjectMan->cutoffTime, 0);
			while(!pProjectMan->timerExpireFlag[0] && !pProjectMan->projectStopFlag)
				vTaskDelay(5);
			if(pProjectMan->projectStopFlag)
				xTimerStop( pProjectMan->xTimerUser[0], 0 );
			
			Cutoff1(0, &(pProjectMan->projectStopFlag));
			pProjectMan->cutoff1Temperature = temp;//���¼���

			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF1);
		}
	}
}

void Cutoff2Task(void * const pvParameters)
{
	uint8_t action;
	while(1)
	{
		vTaskSuspend( NULL );
		action = *(uint8_t *)pvParameters;
		if(action == 0)
		{
			Cutoff2(0, &(pProjectMan->projectStopFlag));
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2);
		}
		else if(action == 1)
		{
			Cutoff2(1, &(pProjectMan->projectStopFlag));
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2);
		}
		else if(action == 2)
		{
			float temp = pProjectMan->cutoff2Temperature;
#if !TEMPERATURE_MASK
			while(adcTemp[1].temperature < pProjectMan->cutoff2Temperature && !pProjectMan->projectStopFlag)//�ȴ��¶ȵ�
				vTaskDelay(5);
#endif			
			Cutoff2(1, &(pProjectMan->projectStopFlag));
			pProjectMan->cutoff2Temperature = 0;//Ѹ�ٽ���
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2);
			
			pProjectMan->timerExpireFlag[1] = 0;
			xTimerChangePeriod( pProjectMan->xTimerUser[1], pProjectMan->cutoffTime, 0);
			while(!pProjectMan->timerExpireFlag[1] && !pProjectMan->projectStopFlag)
			{
				vTaskDelay(5);
			}
			if(pProjectMan->projectStopFlag)
				xTimerStop( pProjectMan->xTimerUser[1], 0 );
			
			Cutoff2(0, &(pProjectMan->projectStopFlag));
			pProjectMan->cutoff2Temperature = temp;//���¼���
			
			xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_CUTOFF2);
		}
	}
}

void SeparationTask(void * const pvParameters)
{
	while(1)
	{
		vTaskSuspend( NULL );
		Separation(*(uint8_t *)pvParameters, &(pProjectMan->projectStopFlag));
		xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_SEPATATION);
	}
}

void DislocationTask(void * const pvParameters)
{
	uint8_t action;
	while(1)
	{
		vTaskSuspend( NULL );
		action = *(uint8_t *)pvParameters;
		if(action == 0)
		{
			Dislocation(0, &(pProjectMan->projectStopFlag));
		}
		else if(action == 1)
		{
			Dislocation(1, &(pProjectMan->projectStopFlag));
		}
		else if(action == 2)
		{
			DislocationDistance(0, pProjectMan->dislocateReturn, &(pProjectMan->projectStopFlag));
		}
		xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_DISLOCATION);
	}
}

void HeatingUpTask(void * const pvParameters)
{
	uint8_t action;
	while(1)
	{
		vTaskSuspend( NULL );
		action = *(uint8_t *)pvParameters;
		if(action == 0)
			HeatingUp(0, &(pProjectMan->projectStopFlag));
		else if(action == 1)
			HeatingUp(1, &(pProjectMan->projectStopFlag));
//		else if(action == 2)
//		{
//			Heating(&(pProjectMan->projectStopFlag));
//			HeatingUp(1, &(pProjectMan->projectStopFlag));
//			
//			
//		}
		xEventGroupSetBits(pProjectMan->projectEventGroup, 1UL<<PROJECT_EVENTPOS_HEATINGUP);
	}
}

#ifdef __cplusplus
}
#endif
