#include "delay.h"

static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,��ucos��,����ÿ�����ĵ�ms��
			   
//��ʼ���ӳٺ���
//��ʹ��OS��ʱ��,�˺������ʼ��OS��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��
void delay_init()
{

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);   /* 使用 HCLK (72MHz) */
	fac_us = SystemCoreClock / 1000000;                /* 72MHz → fac_us=72 */
	fac_ms = (u16)(fac_us * 1000);                     /* 72000 */
}								    


//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void delay_us(uint32_t nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//ʱ�����	  		 
	SysTick->VAL=0x00;        					//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;      					 //��ռ�����	 
}
//��ʱnms
//ע��nms�ķ�Χ
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK��λΪHz,nms��λΪms
//��72M������,nms<=1864 
/* delay_ms: 用循环调用 delay_us 实现，避免 fac_ms u16 溢出
 * 及 SysTick 24 位 LOAD 寄存器超限问题 (72MHz 下单次最大 ~233ms)
 */
void delay_ms(uint16_t nms)
{
    while (nms--)
        delay_us(1000);
}

