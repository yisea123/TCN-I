#ifndef __RELAYMOS_H
#define __RELAYMOS_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//蜂鸣器驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
//蜂鸣器端口定义
#define MOS1 PCout(7)	// BEEP,蜂鸣器接口
#define MOS2 PCout(6)
#define RELAY PBout(14)

#define CUTOFF1HEATMOS1	PCout(6)
#define CUTOFF1HEATMOS2	PCout(7)
#define CUTOFF2HEATMOS1	PDout(13)
#define CUTOFF2HEATMOS2	PDout(14)
#define FUSINGHEATMOS	PDout(15)

void RelayMOS_Init(void);	//初始化
		 				    
#endif

