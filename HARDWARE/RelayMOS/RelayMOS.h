#ifndef __RELAYMOS_H
#define __RELAYMOS_H	 
#include "sys.h"
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
//�������˿ڶ���
#define MOS1 PCout(7)	// BEEP,�������ӿ�
#define MOS2 PCout(6)
#define RELAY PBout(14)

#define CUTOFF1HEATMOS1	PCout(6)
#define CUTOFF1HEATMOS2	PCout(7)
#define CUTOFF2HEATMOS1	PDout(13)
#define CUTOFF2HEATMOS2	PDout(14)
#define FUSINGHEATMOS	PDout(15)

void RelayMOS_Init(void);	//��ʼ��
		 				    
#endif

