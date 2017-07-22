#include "Button.h"
#include "usart.h"
#include "stm32f10x_exti.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "../Logic/managerment.h"
#include "../DCMotor/DCMotor.h"
#include "../StepMotor/StepMotor.h"
#include "../RelayMOS/RelayMOS.h"
#include "../WDG/WDG.h"
#include "../HARDWARE/24CXX/24cxx.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

Button_TypeDef button[BUTTON_COUNT];

//��ʼ��PB8Ϊ�����.��ʹ������ڵ�ʱ��		    
//��������ʼ��
void Button_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��GPIOB�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;//BEEP-->PB.8 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
    GPIO_Init(GPIOE, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOB.8
    
    // NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;		//�����ȼ�3
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ��
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���  
    
    //�ⲿ�ж�����
    EXTI_ClearITPendingBit(EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3);
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
}

void EXTI0_IRQHandler(void)
{
	uint8_t i;
	uint32_t oldBasePri = portSET_INTERRUPT_MASK_FROM_ISR();
	
    button[0].flag = 1;

	//������������
//	vTaskSuspend(pProjectMan->projectTaskHandle);
//	vTaskSuspend(pProjectMan->uiTaskHandle);
//	vTaskSuspend(pProjectMan->heatingUpTaskHandle);
//	vTaskSuspend(pProjectMan->dislocationTaskHandle);
//	vTaskSuspend(pProjectMan->separationTaskHandle);
//	vTaskSuspend(pProjectMan->cutoff2TaskHandle);
//	vTaskSuspend(pProjectMan->cutoff1TaskHandle);
//	vTaskSuspend(pProjectMan->clamp2TaskHandle);
//	vTaskSuspend(pProjectMan->clamp1TaskHandle);
	
	//��ͣ��ֹͣ���е���ͼ���Ƭ�����
	for(i=0;i<DCMOTOR_COUNT;i++)
	{
		DCMotor_Stop((DCMotorEnum_TypeDef)i);
	}
	for(i=0;i<STEPMOTOR_COUNT;i++)
	{
		StepMotor_Stop(i);
		TIM_Cmd(StepMotorPin[i].TIMx, DISABLE);
	}
	RELAY = 0;

	pProjectMan->systemEmergencyButtonPressFlag = 1;
	
//	//��ʾ�����ϵ�
//	SetTextValue(TIPSPAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 ���ɿ���ͣ��ť��\r\n2 �������ϵ磡");
//	SetScreen(TIPSPAGE_INDEX);
//	
//	IWDG_Feed();//ι��	
//	pProjectMan->systemEmergencyFlag = 1;
//	AT24CXX_Write(EMERGENCYFLAG_BASEADDR, (uint8_t*)(&(pProjectMan->systemEmergencyFlag)), 1);
//	IWDG_Feed();//ι��	
//	
//	while(1)
//	{
//		//��ʾ�����ϵ�
//		SetTextValue(TIPSPAGE_INDEX, TIPS_TIPS_EDIT, (uint8_t*)"1 ���ɿ���ͣ��ť��\r\n2 �������ϵ磡");
//		SetScreen(TIPSPAGE_INDEX);
//		IWDG_Feed();//ι��	
//	}
	
    EXTI_ClearITPendingBit(EXTI_Line0);
	portCLEAR_INTERRUPT_MASK_FROM_ISR(oldBasePri);
}

void EXTI1_IRQHandler(void)
{
    button[1].flag = 1;
	
	pProjectMan->tipsBuzzeFlag = 0;//�ط�����
	
	if(!(pProjectMan->projectStatus&PROJECT_RUNNING) && pProjectMan->systemPowerUpFlag)
	{
		if(pProjectMan->autoButtonFlag)
		{
			pProjectMan->autoNResetFlag = 1;//��λ
			//pProjectMan->autoButtonFlag = 0;
		}
		else
		{
			pProjectMan->autoNResetFlag = 2;//�Զ�
			//pProjectMan->autoButtonFlag = 1;
		}
	}
	
    EXTI_ClearITPendingBit(EXTI_Line1);
}

void EXTI2_IRQHandler(void)
{
    button[2].flag = 1;
    
    EXTI_ClearITPendingBit(EXTI_Line2);
}

void EXTI3_IRQHandler(void)
{
    button[3].flag = 1;
    
    EXTI_ClearITPendingBit(EXTI_Line3);
}
