#ifndef __DH11_H
#define __DH11_H
#include "stm32f10x.h"                  // Device header
 /* 版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。
原文链接：https://blog.csdn.net/m0_55849362/article/details/126426768
*/


#define dht11_high GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define dht11_low GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define Read_Data GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)

void DHT11_GPIO_Init_OUT(void);
void DHT11_GPIO_Init_IN(void);
void DHT11_Start(void);
unsigned char DHT11_REC_Byte(void);
void DHT11_REC_Data(void);
extern unsigned int rec_data[4];


#endif

