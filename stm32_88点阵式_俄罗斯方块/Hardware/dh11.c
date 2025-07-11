#include "stm32f10x.h"                  // Device header
#include "dh11.h"
//数据

 /* 版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。
原文链接：https://blog.csdn.net/m0_55849362/article/details/126426768
*/
unsigned int rec_data[4];


void delay_us(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=10;  //自己定义
      while(i--) ;    
   }
}
void delay_ms(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=12000;  //自己定义
      while(i--) ;    
   }
 }
//对于stm32来说，是输出
void DH11_GPIO_Init_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

//对于stm32来说，是输入
void DH11_GPIO_Init_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}



//主机发送开始信号
void DHT11_Start(void)
{
	DH11_GPIO_Init_OUT(); //输出模式
	
	dht11_high; //先拉高
	delay_us(30);
	
	dht11_low; //拉低电平至少18us
	delay_ms(20);
	
	dht11_high; //拉高电平20~40us
	delay_us(30);
	
	DH11_GPIO_Init_IN(); //输入模式
}


//获取一个字节
char DHT11_Rec_Byte(void)
{
	unsigned char i = 0;
	unsigned char data;
	
	for(i=0;i<8;i++) //1个数据就是1个字节byte，1个字节byte有8位bit
	{
		while( Read_Data == 0); //从1bit开始，低电平变高电平，等待低电平结束
		delay_us(30); //延迟30us是为了区别数据0和数据1，0只有26~28us
		
		data <<= 1; //左移
		
		if( Read_Data == 1 ) //如果过了30us还是高电平的话就是数据1
		{
			data |= 1; //数据+1
		}
		
		while( Read_Data == 1 ); //高电平变低电平，等待高电平结束
	}
	
	return data;
}

//获取数据

void DHT11_REC_Data(void)
{
	unsigned int R_H,R_L,T_H,T_L;
	unsigned char RH,RL,TH,TL,CHECK;
	
	DHT11_Start(); //主机发送信号
	dht11_high; //拉高电平
	
	if( Read_Data == 0 ) //判断DHT11是否响应
	{
		while( Read_Data == 0); //低电平变高电平，等待低电平结束
		while( Read_Data == 1); //高电平变低电平，等待高电平结束
		
		R_H = DHT11_Rec_Byte();
		R_L = DHT11_Rec_Byte();
		T_H = DHT11_Rec_Byte();
		T_L = DHT11_Rec_Byte();
		CHECK = DHT11_Rec_Byte(); //接收5个数据
		
		dht11_low; //当最后一bit数据传送完毕后，DHT11拉低总线 50us
		delay_us(55); //这里延时55us
		dht11_high; //随后总线由上拉电阻拉高进入空闲状态。
		
		if(R_H + R_L + T_H + T_L == CHECK) //和检验位对比，判断校验接收到的数据是否正确
		{
			RH = R_H;
			RL = R_L;
			TH = T_H;
			TL = T_L;
		}
	}
	rec_data[0] = RH;
	rec_data[1] = RL;
	rec_data[2] = TH;
	rec_data[3] = TL;
}
