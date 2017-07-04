#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "LED/led.h"
#include "Key/key.h"
#include "Beep/beep.h"
#include "Button/Button.h"
#include "PhotoelectricSensor/PhSensor.h"
#include "RelayMOS/RelayMOS.h"
#include "DCMotor/DCMotor.h"
#include "StepMotor/StepMotor.h"
#include "WDG/WDG.h"
#include "NTCResistor/NTCResistor.h"

#include "../Logic/managerment.h"
#include "CPrintf.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/************************************************

************************************************/
#define uart_TASK_PRIORITY           	( tskIDLE_PRIORITY + 3 )
#define project_TASK_PRIORITY           ( tskIDLE_PRIORITY + 2 )
#define ui_TASK_PRIORITY                ( tskIDLE_PRIORITY + 1 )

#define uart_TASK_STACKDEP            	( 100 )
#define project_TASK_STACKDEP           ( 100 )
#define ui_TASK_STACKDEP                ( 150 )

void UartTask(void);
void ProjectTask(void);
void UITask(void);

//������    500 / portTICK_PERIOD_MS = 500ms
int main(void)
{
//	uint8_t size;
//	size = sizeof(uint64_t);
//	printf("sizeof = ", size);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init();
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200
	LED_Init();			//��ʼ��LED
	//AT24CXX_Init();
	Beep_Init();
	Button_Init();
	PhSensor_Init();
	RelayMOS_Init();
	DCMotor_Init();
	StepMotor_Init();
	NTCResitor_Init();
	
	initUI();
	initSoftwareTimer();
	
	//IWDG_Init(4,625);    //���Ƶ��Ϊ64,����ֵΪ625,���ʱ��Ϊ1s
    
	cDebug("hello TCN-I project!\r\n");
	
	/* Start the tasks defined within this file/specific to this demo. */
	xTaskCreate( (TaskFunction_t)UITask, "UITask", ui_TASK_STACKDEP, NULL, ui_TASK_PRIORITY, NULL );
	xTaskCreate( (TaskFunction_t)UartTask, "UartTask", uart_TASK_STACKDEP, NULL, uart_TASK_PRIORITY, NULL );
	xTaskCreate( (TaskFunction_t)ProjectTask, "ProjectTask", project_TASK_STACKDEP, NULL, project_TASK_PRIORITY, NULL );
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	return 0;
}

