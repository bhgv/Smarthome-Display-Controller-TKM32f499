#include "tk499.h"
#include "LCD.h"
#include "ASCII.h"
#include "GB1616.h"	//16*16????
#include "GB4848.h"	//48*48????

__align(256) u32 LTDC_Buf[XSIZE_PHYS * YSIZE_PHYS*2];

void LCD_delay(volatile int time)  //??????
{
  volatile u32 i;
  while(time--)
    for(i = 500; i > 0; i--);
}
void LTDC_Clock_Set(void)    //??LTDC??
{
  RCC->AHB1ENR |= 1 << 31;
  RCC->CR |= 1 << 28;
  RCC->PLLDCKCFGR = 0x1 << 16; //???? 0~3 --> 2,4,6,8
  RCC->PLLLCDCFGR = 6 << 6;   	//????
}
void set_resolutionXX(LCD_FORM_TypeDef *LCD_FORM)
{
  u32 aHorStart;
  u32 aHorEnd;
  u32 aVerStart;
  u32 aVerEnd;

  aHorStart = LCD_FORM->blkHorEnd + 1;
  aHorEnd = aHorStart + LCD_FORM->aHorLen;
  aVerStart = LCD_FORM->blkVerEnd + 1 ;
  aVerEnd = aVerStart + LCD_FORM->aVerLen;

  LTDC->P_HOR = aHorEnd;//???
  LTDC->HSYNC = (LCD_FORM->sHsyncStart << 16 ) | LCD_FORM->sHsyncEnd; //???????????,???????
  LTDC->A_HOR = (aHorStart << 16) | aHorEnd; //?????????
  LTDC->A_HOR_LEN = LCD_FORM->aHorLen ;//???????
  LTDC->BLK_HOR = (0 << 16) | LCD_FORM->blkHorEnd; //?????????0~????
  LTDC->P_VER =  aVerEnd;
  LTDC->VSYNC = (LCD_FORM->sVsyncStart << 16) | LCD_FORM->sVsyncEnd;
  LTDC->A_VER = (aVerStart << 16) | aVerEnd;
  LTDC->A_VER_LEN = LCD_FORM->aVerLen ;
  LTDC->BLK_VER = (0 << 16) | LCD_FORM->blkVerEnd;
}


void Set_LCD_Timing_to_LTDC(void)
{
  LCD_FORM_TypeDef LCD_FORM;
  LTDC->OUT_EN = 0;
  LTDC->DP_ADDR0 = (u32)LTDC_Buf;//?0???
//  LTDC->DP_ADDR1 = (u32)(LTDC_Buf + SDRAM_RGB_OFFSET);//?????

  LTDC->BLK_DATA = 0x0000;//???

  //===============5? 854*480===============//
		LCD_FORM.sHsyncStart = 0x2;  //??????
    LCD_FORM.sHsyncEnd = 0x3;    //??????
    LCD_FORM.aHorLen = 480 - 1;  //?????
    LCD_FORM.blkHorEnd = 0x3f;    //????

    LCD_FORM.sVsyncStart = 0x2;  //??????
    LCD_FORM.sVsyncEnd = 0x3;    //??????
    LCD_FORM.aVerLen= 854 - 1; 	 //?????
    LCD_FORM.blkVerEnd = 0xf;   //????
	
    set_resolutionXX(&LCD_FORM);

		LTDC->VI_FORMAT = 0;
		LTDC->POL_CTL = 0x3+8;
		LTDC->OUT_EN |= 0x107;
}
void LTDC_IRQHandler(void)
{
  LTDC->INTR_CLR = 2;
  LTDC->DP_SWT ^= 1;//????????
  if(LTDC->DP_SWT != 0 )
  {
    //        fun_test(SDRAM_RGB_ADD);
  }
  else
  {
    //        fun_test(SDRAM_RGB_ADD+SDRAM_RGB_OFFSET);
  }
  //    if(LTDC->INTR_STA & 2)
  //    {

  //    }
}
void GPIO_RGB_INIT(void)
{
GPIO_InitTypeDef GPIO_InitStructure;//??GPIO????????

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOD | RCC_AHBPeriph_GPIOE, ENABLE);
	
	//DE=PB4, PCLK=PB5, HSYNC=PB6, VSYNC=PB7
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//                                   SPI_DCLK     LCD_SPI_CS
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_9 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_0 ;  // SPI_SDA 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	//lcd_reset:PD6      LCD_Black_Light On Off  :PD8
	GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_6 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	Lcd_Initialize();
	

  GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_AF_LTDC); //PB4~7???LTDC??????
	GPIO_PinAFConfig(GPIOE, GPIO_Pin_All, GPIO_AF_LTDC); //GPIOE???IO?????LTDC????
	
}
static void LCD_Reset(void)
{
  //??,??????,?????????
  LCD_RST(0);
  LCD_delay(200);
  LCD_RST(1);
  LCD_delay(200);

}
void LCD_WriteByteSPI(unsigned char byte)
{
  unsigned char n;

  for(n = 0; n < 8; n++)
  {
    if(byte & 0x80) SPI_SDA(1)
      else SPI_SDA(0)
        byte <<= 1;
    SPI_DCLK(0);
    SPI_DCLK(1);
  }
}
void SPI_WriteComm(u16 CMD)//3?9bit ????
{			
	LCD_SPI_CS(0);
	SPI_SDA(0);
	SPI_DCLK(0);
	SPI_DCLK(1);
	LCD_WriteByteSPI(CMD);
	LCD_SPI_CS(1);
}
void SPI_WriteData(u16 tem_data)
{			
	LCD_SPI_CS(0);
	SPI_SDA(1);
	SPI_DCLK(0);
	SPI_DCLK(1);
	LCD_WriteByteSPI(tem_data);
	LCD_SPI_CS(1);
}
void Lcd_Initialize(void)
{
  LCD_SPI_CS(1);
  LCD_delay(20);
  LCD_SPI_CS(0);
  LCD_Reset();

 //****************************************************************************//
//****************************** Page 1 Command ******************************//
//****************************************************************************//
SPI_WriteComm(0xFF); // Change to Page 1
SPI_WriteData(0xFF); 
SPI_WriteData(0x98); 
SPI_WriteData(0x06);
SPI_WriteData(0x04); 
SPI_WriteData(0x01);

SPI_WriteComm(0X08);SPI_WriteData(0X10);                 // output SDA

SPI_WriteComm(0X21);SPI_WriteData(0X01);                 // DE = 1 Active

SPI_WriteComm(0X30);SPI_WriteData(0X01);                 // 480 X 854

SPI_WriteComm(0X31);SPI_WriteData(0X02);                 // 2-dot Inversion

SPI_WriteComm(0X40);SPI_WriteData(0X18);                // DDVDH/L BT 17 -10 5.94 -5.47 -2.3 1.6 43.4 21.88 0.17

SPI_WriteComm(0X41);SPI_WriteData(0X66);              // DDVDH/L CLAMP 

SPI_WriteComm(0X42);SPI_WriteData(0X02);                 // VGH/VGL 

SPI_WriteComm(0X43);SPI_WriteData(0X0A);                 // VGH/VGL 

SPI_WriteComm(0X50);SPI_WriteData(0X78);                 // VGMP 4.6 

SPI_WriteComm(0X51);SPI_WriteData(0X78);                 // VGMN 4.6 

SPI_WriteComm(0X52);SPI_WriteData(0X00);                 // Flicker 

SPI_WriteComm(0X53);SPI_WriteData(0X1F);                 // Flicker 

SPI_WriteComm(0X57);SPI_WriteData(0X50);

SPI_WriteComm(0X60);SPI_WriteData(0X07);                 // SDTI

SPI_WriteComm(0X61);SPI_WriteData(0X01);                 // CRTI 00

SPI_WriteComm(0X62);SPI_WriteData(0X07);                 // EQTI 07

SPI_WriteComm(0X63);SPI_WriteData(0X00);                 // PCTI  00


////++++++++++++++++++ Gamma Setting ++++++++++++++++++//
//SPI_WriteComm(0xFF); // Change to Page 1
//SPI_WriteData(0xFF); 
//SPI_WriteData(0x98); 
//SPI_WriteData(0x06);
//SPI_WriteData(0x04); 
//SPI_WriteData(0x01);

SPI_WriteComm(0XA0);SPI_WriteData(0X00);  // Gamma 0 

SPI_WriteComm(0XA1);SPI_WriteData(0X0A);  // Gamma 4 

SPI_WriteComm(0XA2);SPI_WriteData(0X12);  // Gamma 8

SPI_WriteComm(0XA3);SPI_WriteData(0X0D);  // Gamma 16

SPI_WriteComm(0XA4);SPI_WriteData(0X07);  // Gamma 24

SPI_WriteComm(0XA5);SPI_WriteData(0X09);  // Gamma 52

SPI_WriteComm(0XA6);SPI_WriteData(0X06); // Gamma 80

SPI_WriteComm(0XA7);SPI_WriteData(0X03);  // Gamma 108

SPI_WriteComm(0XA8);SPI_WriteData(0X09);  // Gamma 147

SPI_WriteComm(0XA9);SPI_WriteData(0X0D);  // Gamma 175

SPI_WriteComm(0XAA);SPI_WriteData(0X15);  // Gamma 203

SPI_WriteComm(0XAB);SPI_WriteData(0X07);  // Gamma 231

SPI_WriteComm(0XAC);SPI_WriteData(0X0D);  // Gamma 239

SPI_WriteComm(0XAD);SPI_WriteData(0X0D);  // Gamma 247

SPI_WriteComm(0XAE);SPI_WriteData(0X06);  // Gamma 251

SPI_WriteComm(0XAF);SPI_WriteData(0X00);  // Gamma 255

///==============Nagitive
SPI_WriteComm(0XC0);SPI_WriteData(0X00);  // Gamma 0 

SPI_WriteComm(0XC1);SPI_WriteData(0X0A);  // Gamma 4

SPI_WriteComm(0XC2);SPI_WriteData(0X12);  // Gamma 8

SPI_WriteComm(0XC3);SPI_WriteData(0X0E);  // Gamma 16

SPI_WriteComm(0XC4);SPI_WriteData(0X07);  // Gamma 24

SPI_WriteComm(0XC5);SPI_WriteData(0X09);  // Gamma 52

SPI_WriteComm(0XC6);SPI_WriteData(0X07);  // Gamma 80

SPI_WriteComm(0XC7);SPI_WriteData(0X03);  // Gamma 108

SPI_WriteComm(0XC8);SPI_WriteData(0X09);  // Gamma 147

SPI_WriteComm(0XC9);SPI_WriteData(0X0C);  // Gamma 175

SPI_WriteComm(0XCA);SPI_WriteData(0X15);  // Gamma 203

SPI_WriteComm(0XCB);SPI_WriteData(0X07);  // Gamma 231

SPI_WriteComm(0XCC);SPI_WriteData(0X0C);  // Gamma 239

SPI_WriteComm(0XCD);SPI_WriteData(0X0D); // Gamma 247

SPI_WriteComm(0XCE);SPI_WriteData(0X06); // Gamma 251

SPI_WriteComm(0XCF);SPI_WriteData(0X00);  // Gamma 255


//****************************************************************************//
//****************************** Page 6 Command ******************************//
//****************************************************************************//
SPI_WriteComm(0xFF); // Change to Page 6
SPI_WriteData(0xFF); 
SPI_WriteData(0x98); 
SPI_WriteData(0x06);
SPI_WriteData(0x04); 
SPI_WriteData(0x06);

SPI_WriteComm(0X00);SPI_WriteData(0X21);

SPI_WriteComm(0X01);SPI_WriteData(0X06);

SPI_WriteComm(0X02);SPI_WriteData(0XA0 );   

SPI_WriteComm(0X03);SPI_WriteData(0X02);

SPI_WriteComm(0X04);SPI_WriteData(0X01);

SPI_WriteComm(0X05);SPI_WriteData(0X01);

SPI_WriteComm(0X06);SPI_WriteData(0X80);    

SPI_WriteComm(0X07);SPI_WriteData(0X04);

SPI_WriteComm(0X08);SPI_WriteData(0X00);

SPI_WriteComm(0X09);SPI_WriteData(0X80);  

SPI_WriteComm(0X0A);SPI_WriteData(0X00);   

SPI_WriteComm(0X0B);SPI_WriteData(0X00);    

SPI_WriteComm(0X0C);SPI_WriteData(0X2c);

SPI_WriteComm(0X0D);SPI_WriteData(0X2c);

SPI_WriteComm(0X0E);SPI_WriteData(0X1c);

SPI_WriteComm(0X0F);SPI_WriteData(0X00);

SPI_WriteComm(0X10);SPI_WriteData(0XFF);

SPI_WriteComm(0X11);SPI_WriteData(0XF0);

SPI_WriteComm(0X12);SPI_WriteData(0X00);

SPI_WriteComm(0X13);SPI_WriteData(0X00);

SPI_WriteComm(0X14);SPI_WriteData(0X00);

SPI_WriteComm(0X15);SPI_WriteData(0XC0);

SPI_WriteComm(0X16);SPI_WriteData(0X08);

SPI_WriteComm(0X17);SPI_WriteData(0X00);

SPI_WriteComm(0X18);SPI_WriteData(0X00);

SPI_WriteComm(0X19);SPI_WriteData(0X00);

SPI_WriteComm(0X1A);SPI_WriteData(0X00);

SPI_WriteComm(0X1B);SPI_WriteData(0X00);

SPI_WriteComm(0X1C);SPI_WriteData(0X00);

SPI_WriteComm(0X1D);SPI_WriteData(0X00);


SPI_WriteComm(0X20);SPI_WriteData(0X01);

SPI_WriteComm(0X21);SPI_WriteData(0X23);

SPI_WriteComm(0X22);SPI_WriteData(0X45);

SPI_WriteComm(0X23);SPI_WriteData(0X67);

SPI_WriteComm(0X24);SPI_WriteData(0X01);

SPI_WriteComm(0X25);SPI_WriteData(0X23);

SPI_WriteComm(0X26);SPI_WriteData(0X45);

SPI_WriteComm(0X27);SPI_WriteData(0X67);


SPI_WriteComm(0X30);SPI_WriteData(0X12);

SPI_WriteComm(0X31);SPI_WriteData(0X22);

SPI_WriteComm(0X32);SPI_WriteData(0X22);

SPI_WriteComm(0X33);SPI_WriteData(0X22);

SPI_WriteComm(0X34);SPI_WriteData(0X87);

SPI_WriteComm(0X35);SPI_WriteData(0X96);

SPI_WriteComm(0X36);SPI_WriteData(0XAA);

SPI_WriteComm(0X37);SPI_WriteData(0XDB);

SPI_WriteComm(0X38);SPI_WriteData(0XCC);

SPI_WriteComm(0X39);SPI_WriteData(0XBD);

SPI_WriteComm(0X3A);SPI_WriteData(0X78);

SPI_WriteComm(0X3B);SPI_WriteData(0X69);

SPI_WriteComm(0X3C);SPI_WriteData(0X22);

SPI_WriteComm(0X3D);SPI_WriteData(0X22);

SPI_WriteComm(0X3E);SPI_WriteData(0X22);

SPI_WriteComm(0X3F);SPI_WriteData(0X22);

SPI_WriteComm(0X40);SPI_WriteData(0X22);

SPI_WriteComm(0X52);SPI_WriteData(0X10);

SPI_WriteComm(0X53);SPI_WriteData(0X10);


//SPI_WriteComm(0xFF); // Change to Page 7
//SPI_WriteData(0xFF); 
//SPI_WriteData(0x98); 
//SPI_WriteData(0x06);
//SPI_WriteData(0x04); 
//SPI_WriteData(0x07);

////SPI_WriteComm(0X17);SPI_WriteData(0X22);

//SPI_WriteComm(0X18);SPI_WriteData(0X1D);

////SPI_WriteComm(0X02);SPI_WriteData(0X77);

////SPI_WriteComm(0XE1);SPI_WriteData(0X79);

////SPI_WriteComm(0X06);SPI_WriteData(0X01);


//****************************************************************************//
SPI_WriteComm(0xFF); // Change to Page 0
SPI_WriteData(0xFF); 
SPI_WriteData(0x98); 
SPI_WriteData(0x06);
SPI_WriteData(0x04); 
SPI_WriteData(0x00);

SPI_WriteComm(0X36);SPI_WriteData(0X00);
SPI_WriteComm(0X3A);SPI_WriteData(0X77);


SPI_WriteComm(0X11);              // Sleep-Out
LCD_delay(120);
SPI_WriteComm(0X29);               // Display On

}

/**********************************************
???:Lcd??????

????:xStart x??????
          ySrart y??????
          xLong ??????x????
          yLong  ??????y????
???:?
***********************************************/
void Lcd_ColorBox(u16 xStart, u16 yStart, u16 xLong, u16 yLong, u32 Color)
{
  u16 i, j;
  u32 temp;
  temp = YSIZE_PHYS * xStart;
  for(i = 0; i < yLong; i++)
  {
    for(j = 0; j < xLong; j++)
      LTDC_Buf[yStart + i + YSIZE_PHYS * j + temp] = Color;
  }

  //	u16 i,j;
  //	u32 temp;
  //	temp = XSIZE_PHYS*yStart;
  //	for(i=0;i<yLong;i++)
  //	{
  //		for(j=0;j<xLong;j++)
  //		LTDC_Buf[xStart+j+XSIZE_PHYS*i+temp]=Color;
  //	}
}
void LCD_Initial(void) //LCD?????
{
  GPIO_RGB_INIT();//????????GPIO
  LTDC_Clock_Set();
  Set_LCD_Timing_to_LTDC();

  Lcd_Light_ON;//????
}
/******************************************
???:Lcd????100*100
??:?Lcd????????
????:
******************************************/
void LCD_Fill_Pic(u16 x, u16 y, u16 pic_H, u16 pic_V, u32 *pic)
{
//  u16 i, j;
//  u32 Xstart, k = 0;
//  Xstart = YSIZE_PHYS * x;
//  for(i = 0; i < pic_V; i++)
//  {
//    for(j = 0; j < pic_H; j++)
//      LTDC_Buf[Xstart + i + YSIZE_PHYS * j + y] = pic[k++];
//  }

  //	u16 i,j;
  //	u32 Ystart,k=0;
  //	Ystart = XSIZE_PHYS*y;
  //	for(i=0;i<pic_V;i++)
  //	{
  //		for(j=0;j<pic_H;j++)
  //		LTDC_Buf[x+j+XSIZE_PHYS*i+Ystart]=pic[k++];
  //	}
}
//=============== ?x,y ?????????Color?? ===============
void DrawPixel(u16 x, u16 y, int Color)
{
//  LTDC_Buf[y + YSIZE_PHYS * x] = Color;
	LTDC_Buf[x + XSIZE_PHYS * y] = Color;
}
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//8*16?? ASCII? ??
//???:SPILCD_ShowChar
//?  ?:
//(x,y):
//num:??????:" "--->"~"
//flag:????(1)????(0)
void SPILCD_ShowChar(unsigned short x, unsigned short y, unsigned char num, unsigned int fColor, unsigned int bColor, unsigned char flag)
{
  unsigned char temp;
  unsigned int pos, i, j;

  num = num - ' '; //???????
  i = num * 16;
  for(pos = 0; pos < 16; pos++)
  {
    temp = nAsciiDot[i + pos];	//????ASCII??
    for(j = 0; j < 8; j++)
    {
      if(temp & 0x80)
        DrawPixel(x + j, y, fColor);
      else if(flag) DrawPixel(x + j, y, bColor); //???????flag?1
      temp <<= 1;
    }
    y++;
  }
}

//???16x16???
void PutGB1616(unsigned short x, unsigned short  y, unsigned char c[2], unsigned int fColor, unsigned int bColor, unsigned char flag)
{
  unsigned int i, j, k;
  unsigned short m;
  for (k = 0; k < 64; k++) //64???????????,??????
  {
    if ((codeGB_16[k].Index[0] == c[0]) && (codeGB_16[k].Index[1] == c[1]))
    {
      for(i = 0; i < 32; i++)
      {
        m = codeGB_16[k].Msk[i];
        for(j = 0; j < 8; j++)
        {
          if((m & 0x80) == 0x80)
          {
            DrawPixel(x + j, y, fColor);
          }
          else
          {
            if(flag) DrawPixel(x + j, y, bColor);
          }
          m = m << 1;
        }
        if(i % 2)
        {
          y++;
          x = x - 8;
        }
        else x = x + 8;
      }
    }
  }
}

//?????
void LCD_PutString(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor, unsigned char flag)
{
  unsigned char l = 0;
  while(*s)
  {
    if( *s < 0x80)
    {
      SPILCD_ShowChar(x + l * 8, y, *s, fColor, bColor, flag);
      s++;
      l++;
    }
    else
    {
      PutGB1616(x + l * 8, y, (unsigned char *)s, fColor, bColor, flag);
      s += 2;
      l += 2;
    }
  }
}
//???n*n???
void Put_GB_n(unsigned short x, unsigned short y, unsigned char c[2], unsigned int fColor, unsigned int bColor, unsigned char flag, unsigned int n)
{
  unsigned int i, j, k, a = 0;
  unsigned short m;
  for (k = 0; k < 12; k++) //12???????????,??????
  {
    if ((codeGB_48[k].Index[0] == c[0]) && (codeGB_48[k].Index[1] == c[1]))
    {
      for(i = 0; i < ((n * n) >> 3); i++)
      {
        for(a = 0; a < (n >> 3); a++)
        {
          m = codeGB_48[k].Msk[i + a];
          for(j = 0; j < 8; j++)
          {
            if((m & 0x80) == 0x80)
            {
              DrawPixel(x + j, y, fColor);
            }
            else
            {
              if(flag) DrawPixel(x + j, y, bColor);
            }
            m = m << 1;
          }
          x = x + 8;
        }
        i = i + a - 1;
        y++;
        x = x - 8 * a;
      }
    }
  }
}
//??????,???n
void LCD_PutString_GB_n(unsigned short x, unsigned short y, char *s, unsigned int fColor, unsigned int bColor, unsigned char flag, unsigned int n)
{
  unsigned int l = 0;
  while(*s)
  {
    Put_GB_n(x + l, y, (unsigned char *)s, fColor, bColor, flag, n);
    s += 2;
    l += n;;
  }
}