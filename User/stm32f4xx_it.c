/**
  ******************************************************************************
  * @file    FMC_SDRAM/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   Main Interrupt Service Routines.
  *         This file provides template for all exceptions handler and
  *         peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include <string.h>
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"
#include "bsp_debug_usart.h"
#include "bsp_dht11.h"
#include "stm32f4xx_exti.h"
#include "usart_protocol.h"
#include "app_tasks.h"

#include "FreeRTOS.h"
#include "task.h"

#define TASK_DELAY_NUM  2
#define TASK_DELAY_0    200
#define TASK_DELAY_1    50

uint32_t Task_Delay_Group[TASK_DELAY_NUM];

/* DHT11 read finish flag */
int read_dht11_finish;
extern int16_t brightness;
extern uint8_t sw_key_flag;

extern DHT11_Data_TypeDef DHT11_Data;

/** @addtogroup STM32F429I_DISCOVERY_Examples
  * @{
  */

/** @addtogroup FMC_SDRAM
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{}


/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
extern void xPortSysTickHandler(void);
//systick�жϷ�����
void SysTick_Handler(void)
{	
    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
      if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
      {
    #endif  /* INCLUDE_xTaskGetSchedulerState */  
        xPortSysTickHandler();
    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
      }
    #endif  /* INCLUDE_xTaskGetSchedulerState */
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f429_439xx.s).                         */
/******************************************************************************/

/**
  * @}
  */ 
void EXTI0_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
        brightness += 20;
        if (brightness > 99) brightness = 99;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI15_10_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line13) != RESET) {
        brightness -= 20;
        if (brightness < 0) brightness = 0;
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
        sw_key_flag = 1;
        g_encoder_pressed = 1;  /* Set encoder pressed flag */
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
}

void macESP8266_USART_INT_FUN ( void )
{
	uint8_t ucCh;


	if ( USART_GetITStatus ( macESP8266_USARTx, USART_IT_RXNE ) != RESET )
	{
		ucCh  = USART_ReceiveData( macESP8266_USARTx );

		if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )
			strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ucCh;

	}

	if ( USART_GetITStatus( macESP8266_USARTx, USART_IT_IDLE ) == SET )
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;

		ucCh = USART_ReceiveData( macESP8266_USARTx );

  }

}

/**
  * @brief  USART1 IDLE interrupt handler (IDLE + DMA)
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
    uint32_t temp;
    uint16_t recv_len;

    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) {
        /* Clear IDLE interrupt */
        temp = USART1->SR;
        temp = USART1->DR;

        /* Disable DMA */
        DMA_Cmd(DMA2_Stream2, DISABLE);

        /* Calculate received data length */
        recv_len = USART_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA2_Stream2);

        if(recv_len > 0) {
            /* Parse protocol frame */
            Protocol_ParseFrame(usart_rx_buffer, recv_len);

            /* Clear receive buffer */
            memset(usart_rx_buffer, 0, USART_RX_BUFFER_SIZE);
        }

        /* Re-enable DMA */
        DMA_SetCurrDataCounter(DMA2_Stream2, USART_RX_BUFFER_SIZE);
        DMA_Cmd(DMA2_Stream2, ENABLE);
    }
}

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
