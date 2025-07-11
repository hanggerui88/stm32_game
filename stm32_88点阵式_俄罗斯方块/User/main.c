#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "timerclock.h"

#define DIN_GPIO_PORT GPIOB
#define DIN_PIN       GPIO_Pin_7
#define CS_GPIO_PORT  GPIOB
#define CS_PIN        GPIO_Pin_5
#define CLK_GPIO_PORT GPIOB
#define CLK_PIN       GPIO_Pin_6

//长条水平和竖直
#define shuzhi  0
#define shuiping 1

#define L_L 0//L
#define L_2 1//2竖，三横
#define L_7 2//7
#define L_3 3

#define Z_Z 0//L
#define Z_N 1//2竖，三横


u8 boxsave[8][8];
u8 mix=20,i=1,getmix=2;//mix mode savemode模拟随机，坐标
u8 time=3,key=0,keyvalue=0,zhuangtai=0,run=1;//key按键检测
u8 left,right,turn,mode,savemode;
u8 tgame=2;
u8 score=0;

uint8_t led_matrix[8]; 

    
     u8 defaultChars1[3][8] = {
        {0x7C, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7C, 0x00}, // E
        {0x00, 0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x00}, // N
        {0x7C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x00}, // D
				
    };
		 u8 defaultChars2[10][8] = {
			{0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},//0
				{0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00},//1
				{0x3E, 0x02, 0x02, 0x3E, 0x20, 0x20, 0x3E, 0x00} ,//2
				{0x3E, 0x02, 0x02, 0x3E, 0x02, 0x02, 0x3E, 0x00}, //3
				{0x08, 0x18, 0x28, 0x48, 0xFE, 0x08, 0x08, 0x00},//4
				{0x3C, 0x20, 0x20, 0x3C, 0x04, 0x04, 0x3C, 0x00}, //5
				{0x3C, 0x20, 0x20, 0x3C, 0x24, 0x24, 0x3C, 0x00}, //6
				{0x3E, 0x02, 0x04, 0x08, 0x08, 0x08,0x08, 0x00},//7
				{0x3C, 0x24, 0x3C, 0x24, 0x24, 0x3C, 0x00, 0x00},//8
				{0x3E,0x22,0x22,0x3E,0x02,0x02,0x3E,0x00} //9
				};
		 
void MAX7219_GPIO_Init(void);
void MAX7219_GPIO_SPI_Write(uint8_t byte);
void MAX7219_Write(uint8_t reg, uint8_t data);
void MAX7219_Init(void);
void LED_Clear(void);
void LED_Update(void);
void LED_DrawPoint(int16_t x, int16_t y, uint8_t color);
void draw_fallen_boxes(void);

void showbox(void);
void showstick(void);
void showL(void);
void showZ(void);
void thinkstickleft(void);
void thinkstickturn(void);
void thinkstickright(void);
void showlleft(void);
void showlright(void);
void showlturn(void);
void zleft(void);
void zright(void);
void zturn(void);
void box(void);
void stick(void);
void L(void);
void Z(void);
void getbox(void);
void keyintput(void);
void buttonini(void);

//向max7219写数据
void MAX7219_GPIO_SPI_Write(uint8_t byte) {
    GPIO_SetBits(CS_GPIO_PORT, CS_PIN);
    GPIO_ResetBits(CLK_GPIO_PORT, CLK_PIN);

    for (int i = 0; i < 8; i++) {
        if (byte & (1 << (7 - i))) {
					//检查要发送的位是不是1，是则把该位cin置为1
            GPIO_SetBits(DIN_GPIO_PORT, DIN_PIN);
        } else {
            GPIO_ResetBits(DIN_GPIO_PORT, DIN_PIN);
        }
        GPIO_SetBits(CLK_GPIO_PORT, CLK_PIN);
        Delay_us(1);
        GPIO_ResetBits(CLK_GPIO_PORT, CLK_PIN);
        Delay_us(1);
    }
}
void MAX7219_Write(uint8_t reg, uint8_t data) {
	//拉低电平
    GPIO_ResetBits(CS_GPIO_PORT, CS_PIN);
    MAX7219_GPIO_SPI_Write(reg);//写入地址
    MAX7219_GPIO_SPI_Write(data);//写入数据
    GPIO_SetBits(CS_GPIO_PORT, CS_PIN);//拉高片选引脚
}
void MAX7219_Init(void) {
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
    GPIO_InitStructure.GPIO_Pin = DIN_PIN | CLK_PIN | CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(CS_GPIO_PORT, CS_PIN);
    GPIO_ResetBits(CLK_GPIO_PORT, CLK_PIN);

    MAX7219_Write(0x09, 0x00);  
    MAX7219_Write(0x0A, 0x07);   //中等亮度
    MAX7219_Write(0x0B, 0x07);   
    MAX7219_Write(0x0C, 0x01);  
    MAX7219_Write(0x0F, 0x00);  

    LED_Clear();
}

void LED_DrawPoint(int16_t x, int16_t y, uint8_t color) {
    if (x < 0 || x > 8 || y < 0 || y > 8) 
        return;
    if (color) {
        led_matrix[y] |= (1 << (7 - x));
    } else {
        led_matrix[y] &= ~(1 << (7 - x));
    }
}

void keyintput()
{
    if (key > 1) {
        key = 0;//按键之后多等一会再检测
        uint8_t pa0 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0;
        uint8_t pa1 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0;
        uint8_t pa2 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0;
        uint8_t pa3 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0;
        if (pa0) {
            keyvalue = 3;
        }
        if (pa1) {
            keyvalue = 1;
        }
        if (pa2) {
            keyvalue = 2;
        }
        if (pa3) {
            tgame = 0;
        }
    }
}

void buttonini() {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void thinkstickleft()
{
    if (zhuangtai == shuzhi) {
        if ((boxsave[i/5][getmix-1] + boxsave[(i-5)/5][getmix-1] + 
             boxsave[(i-10)/5][getmix-1] + boxsave[(i-15)/5][getmix-1] == 0) && 
            getmix != 0) {
            left = 1;
        }
    }
    else if (zhuangtai == shuiping) {
        if (getmix < 3) {
            if ((boxsave[i/5][getmix-1] == 0) && getmix != 0) {
                left = 1;
            }
        }
        else {
            if ((boxsave[i/5][getmix-3] == 0) && getmix != 2) {
                left = 1;
            }
        }
    }
}

void thinkstickturn()
{		u8	row=i/5;
     if (zhuangtai == shuiping) {
        // 有足够4行垂直空间
        if (row >= 3) {
            if (boxsave[row][getmix] == 0 &&boxsave[row-1][getmix] == 0 && boxsave[row-2][getmix] == 0 &&
                boxsave[row-3][getmix] == 0) {
                turn = 1;
            } else {
            turn = 0; 
        }}
        
    }
    else if (zhuangtai == shuzhi) {
        if (getmix < 3) {//左侧
            if (boxsave[row][getmix] + boxsave[row][getmix+1] + boxsave[row][getmix+2] == 0) {
                turn = 1;
            }
        }
        else {//右侧
            if (boxsave[row][getmix] + boxsave[row][getmix-1] + boxsave[row][getmix-2] == 0) {
                turn = 1;
            }
        }
    }
}

void thinkstickright()
{
    if (zhuangtai == shuzhi) {
        if ((boxsave[i/5][getmix+1] + boxsave[(i-5)/5][getmix+1] + 
             boxsave[(i-10)/5][getmix+1] + boxsave[(i-15)/5][getmix+1] == 0) && 
            getmix < 8) {
            right = 1;
        }
    }
    else if (zhuangtai == shuiping) {
        if (getmix < 3) {
            if ((boxsave[i/5][getmix+3] == 0) && getmix != 6) {
                right = 1;
            }
        }
				//向右侧移动
        else {
            if ((boxsave[i/5][getmix] == 0) && getmix < 8) {
                right = 1;
            }
        }
    }
}

//getmix，长条的位置
//i/5，i是帧，这样降低帧的降低速度
void showstick()
{
    if (zhuangtai == shuzhi) {
        LED_DrawPoint(getmix, i/5 - 0, 1);  
        LED_DrawPoint(getmix, i/5 - 1, 1);  
        LED_DrawPoint(getmix, i/5 - 2, 1);  
        LED_DrawPoint(getmix, i/5 - 3, 1);  
    }
    else if (zhuangtai == shuiping) {
			//水平要分区处理，左移和右移的时候标准是最左和最右
        if (getmix < 3) {
            LED_DrawPoint((getmix + 0), (i/5 - 0), 1);  
            LED_DrawPoint((getmix + 1), (i/5 - 0), 1);  
            LED_DrawPoint((getmix + 2), (i/5 - 0), 1);  
            LED_DrawPoint((getmix + 3), (i/5 - 0), 1); 
        }
        else {
            LED_DrawPoint((getmix - 3), (i/5 - 0), 1);  
            LED_DrawPoint((getmix - 2), (i/5 - 0), 1);  
            LED_DrawPoint((getmix - 1), (i/5 - 0), 1); 
            LED_DrawPoint((getmix - 0), (i/5 - 0), 1); 
        }
    }
}
void stick()
{
    u8 ii;
    if (time >= tgame)
    {
        time = 0;
        LED_Clear();  
        draw_fallen_boxes(); 
        showstick();  
        LED_Update(); 
        if (keyvalue == 1) 
        {
            thinkstickleft();
            if (left == 1)
            {              
                LED_Clear();
                draw_fallen_boxes();
                getmix--;
                showstick();
                LED_Update();
                
                keyvalue = 0;
                left = 0;
            }
        }
        else if (keyvalue == 2) 
        {
            thinkstickright();
            if (right == 1)
            {
                LED_Clear();
                draw_fallen_boxes();
                getmix++;
                showstick();
                LED_Update();
                keyvalue = 0;
                right = 0;
            }
        }
        else if (keyvalue == 3) 
        {
            thinkstickturn();
            if (turn == 1)
            {
                LED_Clear();
                draw_fallen_boxes();
                zhuangtai = !zhuangtai;
                showstick();
                LED_Update();
                keyvalue = 0;
                turn = 0;
            }
        }
//-----------控制检查不要太快
         if (i % 5 == 0)
        {
            if (zhuangtai == shuzhi)
            {
                if (boxsave[i/5][getmix] == 1)
                {
                    for (ii = 0; ii < 4; ii++)
                    {
                        boxsave[(i/5) - ii][getmix] = 1;//方块固定
                    }
                    i = 61;
                }
            }
            else if (zhuangtai == shuiping) 
            {
                if (getmix < 3)
                {
                    if (boxsave[i/5][getmix] + boxsave[i/5][getmix+1] + boxsave[i/5][getmix+2] != 0)
                    {
                        for (ii = 0; ii < 3; ii++)
                        {
                            boxsave[(i-5)/5][getmix + ii] = 1;
                        }
                        i = 61;
                    }
                }
                else
                {
                    if (boxsave[i/5][getmix] + boxsave[i/5][getmix-1] + boxsave[i/5][getmix-2] != 0)
                    {
                        for (ii = 0; ii < 3; ii++)
                        {
                            boxsave[(i-5)/5][getmix - ii] = 1;
                        }
                        i = 61;
                    }
                }
            }
        }
        i++;
        
        if (i >= 61) {
            getbox();
            getmix = mix / 10;
            if (getmix > 90) getmix = 0;
            i = 1;
            zhuangtai = 0;  
            tgame = 2;
            savemode = mode;
            
            LED_Clear();
            draw_fallen_boxes();
            showL();
            LED_Update();
        }
    }
}

void draw_fallen_boxes()
{
    for (u8 x = 0; x < 8; x++) {
        for (u8 y = 0; y < 8; y++) {
            if (boxsave[y][x] == 1) {
                LED_DrawPoint((x), (y), 1);
            }
        }
    }
}
void showL() {
    u8 base_y = i / 5;
    
    if (zhuangtai == L_L) 
    {
        LED_DrawPoint(getmix,base_y,1);  
        LED_DrawPoint(getmix,base_y - 1, 1);  
        LED_DrawPoint(getmix,base_y - 2, 1);  
        LED_DrawPoint(getmix + 1, base_y,1);
    }
    else if (zhuangtai == L_2) 
    {
        LED_DrawPoint(getmix,base_y,1);  
        LED_DrawPoint(getmix + 1, base_y,1);  
        LED_DrawPoint(getmix + 2, base_y,1);  
        LED_DrawPoint(getmix,base_y - 1, 1); 
    }
    else if (zhuangtai == L_7)
    {
        LED_DrawPoint(getmix,base_y,1);  
        LED_DrawPoint(getmix,base_y - 1, 1);  
        LED_DrawPoint(getmix,base_y - 2, 1);  
        LED_DrawPoint(getmix + 1, base_y - 2, 1); 
    }
    else if (zhuangtai == L_3)
    {
        LED_DrawPoint(getmix,base_y,1); 
        LED_DrawPoint(getmix + 1, base_y,1);  
        LED_DrawPoint(getmix + 2, base_y,1);  
        LED_DrawPoint(getmix + 2, base_y - 1, 1); 
    }
}
void showlleft()
{
    u8 base_y = i / 5;
    
    if (zhuangtai == L_L) {
        if (base_y >= 2 && 
            (boxsave[base_y][getmix - 1] + 
             boxsave[base_y - 1][getmix - 1] + 
             boxsave[base_y - 2][getmix - 1] == 0) && 
            getmix != 0) {
            left = 1;
        }
    }  
    if (zhuangtai == L_2) {
        if (boxsave[base_y][getmix - 1] == 0 && getmix != 0) {
            left = 1;
        }
    }
    if (zhuangtai == L_7) {
        if (base_y >= 2 && 
            (boxsave[base_y][getmix - 1] + 
             boxsave[base_y - 1][getmix - 1] + 
             boxsave[base_y - 2][getmix - 1] == 0) && 
            getmix != 0) {
            left = 1;
        }
    }
    if (zhuangtai == L_3) {
        if (getmix >= 3 && 
            (boxsave[base_y][getmix - 3] + 
             boxsave[base_y - 2][getmix - 3] == 0) && 
            getmix != 2) {
            left = 1;
        }
    }
}
void showlright()
{
    u8 base_y = i / 5;
    
    if (zhuangtai == L_L) {
        if (getmix < 7 && 
            (boxsave[base_y][getmix + 2] + 
             boxsave[base_y - 1][getmix + 1] + 
             boxsave[base_y - 2][getmix + 1] == 0)) {
            right = 1;
        }
    }
    else if (zhuangtai == L_2) {
        if (getmix < 6 && 
            (boxsave[base_y][getmix + 3] + 
             boxsave[base_y - 2][getmix + 3] == 0)) {
            right = 1;
        }
    }
    else if (zhuangtai == L_7) {
        if (getmix < 7 && 
            (boxsave[base_y][getmix + 1] + 
             boxsave[base_y - 1][getmix + 1] + 
             boxsave[base_y - 2][getmix + 2] == 0)) {
            right = 1;
        }
    }
    else if (zhuangtai == L_3) {
        if (getmix < 8 && boxsave[base_y][getmix] == 0) {
            right = 1;
        }
    }
}
void showlturn() {
    u8 base_y = i / 5;  
    
    if (zhuangtai == L_L) {
        if (getmix <= 5) {
            if (boxsave[base_y][getmix + 1] == 0 && 
                boxsave[base_y][getmix + 2] == 0 && 
                boxsave[base_y - 1][getmix] == 0) {
                turn = 1;
            }
        }
    } 
    else if (zhuangtai == L_2) {
        if (base_y >= 1) {
            if (boxsave[base_y - 1][getmix] == 0 && 
                boxsave[base_y + 1][getmix + 1] == 0) {
                turn = 1;
            }
        }
    } 
    else if (zhuangtai == L_7) {
        if (getmix >= 1) {
            if (boxsave[base_y][getmix - 1] == 0 && 
                boxsave[base_y - 1][getmix + 1] == 0 && 
                boxsave[base_y - 2][getmix + 1] == 0) {
                turn = 1;
            }
        }
    } 
    else if (zhuangtai == L_3) {
        if (boxsave[base_y][getmix - 1] == 0 && 
            boxsave[base_y][getmix + 1] == 0) {
            turn = 1;
        }
    }
}

void showZ()
{
    u8 base_y = i / 5; 
    
    if (zhuangtai == Z_Z) 
    {
    LED_DrawPoint((getmix - 1), (base_y - 1), 1); 
    LED_DrawPoint((getmix), (base_y - 1), 1);     
    LED_DrawPoint((getmix), (base_y), 1);     
    LED_DrawPoint((getmix + 1), (base_y ), 1); 
    }
    else if (zhuangtai == Z_N) 
    {
        LED_DrawPoint(getmix, base_y - 2, 1);   
        LED_DrawPoint(getmix - 1, base_y - 1, 1); 
        LED_DrawPoint(getmix, base_y - 1, 1);     
        LED_DrawPoint(getmix - 1, base_y, 1);   
    }
}

void zleft()
{
    u8 base_y = i / 5;
    
    if (zhuangtai == Z_Z) 
    {
        if ((boxsave[(base_y - 2)][getmix - 2] == 0 && 
             boxsave[(base_y - 1)][getmix - 1] == 0) && 
            (getmix - 1 > 0)) 
        {
            left = 1;
        }
    }
    else if (zhuangtai == Z_N) 
    {
        if ((boxsave[(base_y - 3)][getmix - 2] == 0 && 
             boxsave[(base_y - 2)][getmix - 2] == 0 && 
             boxsave[(base_y - 1)][getmix - 1] == 0) && 
            (getmix - 1 > 0)) 
        {
            left = 1;
        }
    }
}

void zright()
{
    u8 base_y = i / 5;
    
    if (zhuangtai == Z_Z) 
    {
        if ((boxsave[(base_y - 2)][getmix + 1] == 0 && 
             boxsave[(base_y - 1)][getmix + 2] == 0) && 
            (getmix + 1 < 8)) 
        {
            right = 1;
        }
    }
    else if (zhuangtai == Z_N) 
    {
        if ((boxsave[(base_y - 3)][getmix] == 0 && 
             boxsave[(base_y - 2)][getmix + 1] == 0 && 
             boxsave[(base_y - 1)][getmix + 1] == 0) && 
            (getmix < 8)) 
        {
            right = 1;
        }
    }
}

void zturn()
{
    u8 base_y = i / 5;
    
    if (zhuangtai == Z_Z) 
    {
        if (boxsave[(base_y - 3)][getmix - 1] == 0) 
        {
            turn = 1;
        }
    }
    else if (zhuangtai == Z_N) 
    {
        if ((boxsave[(base_y - 1)][getmix + 1] == 0) && (getmix + 1 < 8)) 
        {
            turn = 1;
        }
    }
}
void MAX7219_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = DIN_PIN | CLK_PIN | CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(CS_GPIO_PORT, CS_PIN);
    GPIO_ResetBits(CLK_GPIO_PORT, CLK_PIN);
}

void LED_Clear(void) {
     for (uint8_t row = 0; row < 8; row++) {
            led_matrix[row] = 0x00;
	}
}


void LED_Update(void) {
    for (uint8_t row = 0; row < 8; row++) {
			MAX7219_Write(row + 1, led_matrix[row]);
    }
}


void showbox(void) {
    u8 base_y = i / 5;
 
    LED_DrawPoint((getmix), (base_y - 0), 1);
    LED_DrawPoint((getmix + 1), (base_y - 0), 1);
    LED_DrawPoint((getmix), (base_y - 1), 1);
    LED_DrawPoint((getmix + 1), (base_y - 1), 1);
}


void box(void) {
    if (time >= tgame) {
        time = 0;

        LED_Clear();
        draw_fallen_boxes();
        showbox();
        LED_Update();

        if (keyvalue == 1) { 
            if (boxsave[(i-5)/5][getmix-1] == 0 && getmix > 0) {
                LED_Clear();
                draw_fallen_boxes();
                getmix--;
                showbox();
                LED_Update();
                keyvalue = 0;
            }
        } 
        else if (keyvalue == 2) { 
            if (boxsave[(i-5)/5][getmix+2] == 0 && getmix < 7) {
                LED_Clear();
                draw_fallen_boxes();
                getmix++;
                showbox();
                LED_Update();
                keyvalue = 0;
            }
        }
        
        if (i % 5 == 0) {
            if (boxsave[i/5][getmix] == 1 || boxsave[i/5][getmix+1] == 1) {
                boxsave[(i-5)/5][getmix] = 1;
                boxsave[(i-5)/5][getmix+1] = 1;
                boxsave[(i-10)/5][getmix] = 1;
                boxsave[(i-10)/5][getmix+1] = 1;
                i = 61;
            }
        }
        
        i++;
        
        if (i >= 61) {
            getbox();
            getmix = mix / 10;
            if (getmix > 90) getmix = 0;
            i = 1;
            zhuangtai = 0;
            tgame = 2;
            savemode = mode;
            
            LED_Clear();
            draw_fallen_boxes();
            showbox();
            LED_Update();
        }
    }
}


void L(void) {
    u8 ii;
    if (time >= tgame) {
        time = 0;

        LED_Clear();
        draw_fallen_boxes();
        showL();
        LED_Update();

        if (keyvalue == 1) {
            showlleft();
            if (left == 1) {
                LED_Clear();
                draw_fallen_boxes();
                getmix--;
                showL();
                LED_Update();
                keyvalue = 0;
                left = 0;
            }
        } else if (keyvalue == 2) { 
            showlright();
            if (right == 1) {
                LED_Clear();
                draw_fallen_boxes();
                getmix++;
                showL();
                LED_Update();
                keyvalue = 0;
                right = 0;
            }
        } else if (keyvalue == 3) { 
            showlturn();
            if (turn == 1) {
                LED_Clear();
                draw_fallen_boxes();
                if (zhuangtai == 0) {
                    if (getmix < 3) zhuangtai = 1;
                    else zhuangtai = 3;
                } else if (zhuangtai == 1) {
                    zhuangtai = 2;
                } else if (zhuangtai == 2) {
                    zhuangtai = 3;
                } else if (zhuangtai == 3) {
                    zhuangtai = 0;
                }
                showL();
                LED_Update();
                keyvalue = 0;
                turn = 0;
            }
        }

        if (i % 5 == 0) {
            u8 base_y = i / 5; 
            
            if (zhuangtai == 0) { 
                if (boxsave[base_y][getmix] == 1 || boxsave[base_y][getmix+1] == 1) {
                    for (ii = 1; ii < 4; ii++) {
                        boxsave[base_y - ii][getmix] = 1;
                    }
                    boxsave[base_y - 1][getmix + 1] = 1;
                    i = 61;
                }
            } else if (zhuangtai == 1) { 
                if (getmix > 2) {
                    if (boxsave[base_y][getmix - 2] == 1 || boxsave[base_y][getmix] == 1) {
                        boxsave[base_y][getmix] = 1;
                        boxsave[base_y - 1][getmix] = 1;
                        boxsave[base_y - 2][getmix] = 1;
                        boxsave[base_y][getmix - 2] = 1;
                        i = 61;
                    }
                } else {
                    if (boxsave[base_y][getmix] == 1 || boxsave[base_y][getmix+1] == 1) {
                        
                        boxsave[base_y][getmix] = 1;
                        boxsave[base_y - 1][getmix] = 1;
                        boxsave[base_y - 2][getmix] = 1;
                        boxsave[base_y][getmix + 1] = 1;
                        i = 61;
                    }
                }
            } else if (zhuangtai == 2) { 
                if (boxsave[base_y][getmix] == 1) {
                    
                    for (ii = 1; ii < 4; ii++) {
                        boxsave[base_y - ii][getmix] = 1;
                    }
                    boxsave[base_y - 3][getmix + 1] = 1;
                    i = 61;
                }
            } else if (zhuangtai == 3) {
                u8 check_getmix = (getmix >= 2) ? getmix : 0;
                if (boxsave[base_y][check_getmix] == 1 || 
                    boxsave[base_y][check_getmix - 1] == 1 || 
                    boxsave[base_y][check_getmix - 2] == 1) {

                    boxsave[base_y][check_getmix - 0] = 1;
                    boxsave[base_y][check_getmix - 1] = 1;
                    boxsave[base_y][check_getmix - 2] = 1;
                    boxsave[base_y - 2][check_getmix - 2] = 1;
                    i = 61;
                }
            }
        }
        
        i++;
        
        if (i >= 61) {
            getbox();
            getmix = mix / 10;
            if (getmix > 90) getmix = 0;
            i = 1;
            zhuangtai = 0;  
            tgame = 2;
            savemode = mode;
            
            LED_Clear();
            draw_fallen_boxes();
            showL();
            LED_Update();
        }
    }
}

void Z(void) {
    if (time >= tgame) {
        time = 0;
        
        LED_Clear();
        draw_fallen_boxes();
        showZ();
        LED_Update();

        if (keyvalue == 1) { 
            zleft();
            if (left == 1) {
                LED_Clear();
                draw_fallen_boxes();
                getmix--;
                showZ();
                LED_Update();
                keyvalue = 0;
                left = 0;
            }
        } else if (keyvalue == 2) { 
            zright();
            if (right == 1) {
                LED_Clear();
                draw_fallen_boxes();
                getmix++;
                showZ();
                LED_Update();
                keyvalue = 0;
                right = 0;
            }
        } else if (keyvalue == 3) { 
            zturn();
            if (turn == 1) {
                LED_Clear();
                draw_fallen_boxes();

                zhuangtai = (zhuangtai == 0) ? 1 : 0;
                showZ();
                LED_Update();
                keyvalue = 0;
                turn = 0;
            }
        }

        if (i % 5 == 0) {
    u8 base_y = i / 5;
    
    if (zhuangtai == Z_Z)
    {
        if (base_y < 8 - 1) {
            if (
                boxsave[base_y][getmix - 1] == 1 || 
                boxsave[base_y][getmix] == 1 ||     
                boxsave[base_y][getmix + 1] == 1     
            ) {
                boxsave[base_y - 1][getmix - 1] = 1; 
                boxsave[base_y - 1][getmix] = 1;      
                boxsave[base_y][getmix] = 1;          
                boxsave[base_y][getmix + 1] = 1;       
                i = 61; 
            }
        } else { 
            boxsave[base_y - 1][getmix - 1] = 1;
            boxsave[base_y - 1][getmix] = 1;
            boxsave[base_y][getmix] = 1;
            boxsave[base_y][getmix + 1] = 1;
            i = 61;
        }
    }
    else if (zhuangtai == Z_N) 
    {

        if (base_y < 8 - 1) {
            if (
                boxsave[base_y - 1][getmix - 1] == 1 || 
                boxsave[base_y][getmix - 1] == 1 ||     
                boxsave[base_y][getmix] == 1            
            ) {

                boxsave[base_y - 2][getmix] = 1;     
                boxsave[base_y - 1][getmix - 1] = 1; 
                boxsave[base_y - 1][getmix] = 1;     
                boxsave[base_y][getmix - 1] = 1;    
                i = 61; 
            }
        } else { 
            boxsave[base_y - 2][getmix] = 1;
            boxsave[base_y - 1][getmix - 1] = 1;
            boxsave[base_y - 1][getmix] = 1;
            boxsave[base_y][getmix - 1] = 1;
            i = 61; 
        }
    }
}
i++; 

if (i >= 61) {
    getbox(); 
    getmix = mix / 10; 
    if (getmix > 90) getmix = 0; 
    i = 1;
    zhuangtai = Z_Z; 
    tgame = 2;
    savemode = mode;
    LED_Clear();
    draw_fallen_boxes();
    showZ();
    LED_Update();
}
}
}
//检查游戏结束和消除
void getbox() {
    u8 xx, yy, hasFullLine;
    u8 topRow = 0;
    for (u8 i = 0; i < 8; i++) {
        if (boxsave[0][i] != 0) {
            topRow = 1;
            break;
        }
    }
    if (topRow) {
        run = 0;  
        return;
    }
    u8 redraw = 0;
    for (u8 currentRow = 6; currentRow != 255; currentRow--) { 
        hasFullLine = 1;
        for (u8 col = 0; col < 8; col++) {
            if (boxsave[currentRow][col] == 0) {
                hasFullLine = 0;
                break;
            }
        }

        if (hasFullLine) {
            for (yy = currentRow; yy > 0; yy--) {
                for (xx = 0; xx < 8; xx++) {
                    boxsave[yy][xx] = boxsave[yy - 1][xx];
                }
            }
            for (xx = 0; xx < 8; xx++) {
                boxsave[0][xx] = 0;
            }
            
            score += 1;        
            redraw = 1;  
            currentRow++;     
        }
    }

    if (redraw) {
        LED_Clear();
        for (xx = 0; xx < 8; xx++) {
            for (yy = 0; yy < 8; yy++) {
                if (boxsave[yy][xx] != 0) {
                    LED_DrawPoint(xx, yy, 1); 
                }
            }
        }
        LED_Update();
    }
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
        mix++;
        if (mix > 70) mix = 20;
        time++;
        if (time > 7) time = 3;
        key++;
			//通过定时器控制按键扫描频率
        if (key > 10) key = 5;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

typedef struct {
    u8 offset;        //偏移
    u8 chars[13][8]; 
    u8 char_count;      
    u16 speed;         
} ScrollState;

//状态，延时时间
void ScrollState_Init(ScrollState *state, u16 speed_ms ) {
    state->offset = 0;
    state->speed = speed_ms;
    state->char_count = 5; 
}

void ScrollState_score(ScrollState *state){
	for(u8 i = 0; i < 3; i++) {
    for(u8 j = 0; j < 8; j++) {
        state->chars[i][j] = defaultChars1[i][j];
    }
}
    u8 tens = (score / 10);     
    u8 one =score;
    for(u8 j = 0; j < 8; j++) {
        state->chars[3][j] = defaultChars2[tens][j];
    }
    for(u8 j = 0; j < 8; j++) {
        state->chars[4][j] = defaultChars2[one][j];
    }
}

//状态
void ScrollEND_Frame(ScrollState *state) {

    if (state->char_count < 2) return; 

    u8 char_index = (state->offset / 8) % state->char_count;
    u8 char_offset = state->offset % 8;
    u8 next_char_index = (char_index + 1) % state->char_count;
    
    LED_Clear(); 

    for (u8 y = 0; y < 8; y++) {
        for (u8 x = 0; x < 8; x++) {
            u8 bit = 0;
            
            if (x + char_offset < 8) {
                u8 line = state->chars[char_index][y];
                bit = (line >> (7 - (x + char_offset))) & 1;
            } else if (x + char_offset < 16) {
                u8 line = state->chars[next_char_index][y];
                bit = (line >> (7 - (x + char_offset - 8))) & 1;
            }
            LED_DrawPoint(x, y, bit);
        }
    }   
    LED_Update();
    state->offset = (state->offset + 1) % (state->char_count * 8);
    Delay_ms(state->speed);
}

int main(void) {
    MAX7219_Init();  
    timerini();       
    buttonini();      
		ScrollState end_scroll;
		ScrollState_Init(&end_scroll,100);
    for (u8 i = 0; i < 8; i++) {
        boxsave[7][i] = 1; //界限
    }
    savemode = mode;  
    LED_Clear();       
    LED_Update();    
    while (1) {
        if (run == 1) { 
            keyintput(); 
            if (mix < 40 && mix > 30) 
                mode = 0;
            else if (mix >= 40 && mix < 55) 
                mode = 1;
            else if (mix >= 55) 
                mode = 2;
            else 
                mode = 3;
            switch (savemode) {
                case 0: box(); break;    
                case 1: stick(); break;   
                case 2: L(); break;      
                case 3: Z(); break;      
            }
        } 
        else { 
            LED_Clear();
						ScrollState_score(&end_scroll);
            ScrollEND_Frame(&end_scroll); 
            LED_Update();
        }
        Delay_ms(100);
    }
}
