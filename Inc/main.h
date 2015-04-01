/**
  ******************************************************************************
  * @file    I2C/I2C_TwoBoards_ComDMA/Inc/main.h 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    26-December-2014
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

/*	Touch Driver */
#include "FT6206.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Definition for TIMx_5 clock resources */
#define TIMx_5                           TIM5
#define TIMx_5_CLK_ENABLE                __HAL_RCC_TIM5_CLK_ENABLE

/* Definition for TIMx_5's NVIC */
#define TIMx_5_IRQn                      TIM5_IRQn
#define TIMx_5_IRQHandler                TIM5_IRQHandler

/* Definition for TIMx_4 clock resources */
#define TIMx_4	                         TIM4
#define TIMx_4_CLK_ENABLE                __HAL_RCC_TIM4_CLK_ENABLE

/* Definition for TIMx_4's NVIC */
#define TIMx_4_IRQn                      TIM4_IRQn
#define TIMx_4_IRQHandler                TIM4_IRQHandler
/* Definition for TIMx clock resources */
#define TIMx							TIM9
#define TIMx_CLK_ENABLE					__HAL_RCC_TIM9_CLK_ENABLE

/* Definition for USARTx Pins */
#define TIMx_CHANNEL_GPIO_PORT()		__HAL_RCC_GPIOE_CLK_ENABLE()
#define GPIO_PIN_CHANNEL1				GPIO_PIN_5

/* Definition for USARTx clock resources */
#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()
//#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART1
#define USARTx_RX_PIN                    GPIO_PIN_10
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART1

/* Definition for USARTx's DMA */
//#define USARTx_TX_DMA_CHANNEL            DMA_CHANNEL_4
//#define USARTx_TX_DMA_STREAM             DMA1_Stream6
//#define USARTx_RX_DMA_CHANNEL            DMA_CHANNEL_4
//#define USARTx_RX_DMA_STREAM             DMA1_Stream5

/* Definition for USARTx's NVIC */
//#define USARTx_DMA_TX_IRQn               DMA1_Stream6_IRQn
//#define USARTx_DMA_RX_IRQn               DMA1_Stream5_IRQn
//#define USARTx_DMA_TX_IRQHandler         DMA1_Stream6_IRQHandler
//#define USARTx_DMA_RX_IRQHandler         DMA1_Stream5_IRQHandler
#define USARTx_IRQn                      USART1_IRQn
#define USARTx_IRQHandler                USART1_IRQHandler

/* User can use this section to tailor I2Cx/I2Cx instance used and associated resources */
/* Definition for I2Cx clock resources */
#define I2Cx                             I2C1
#define I2Cx_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
//#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()


/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1

/* Definition for ADCx clock resources */
#define ADCx                            ADC1
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC1_CLK_ENABLE()
//#define DMAx_CLK_ENABLE()               __HAL_RCC_DMA2_CLK_ENABLE()     
#define ADCx_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
     
#define ADCx_FORCE_RESET()              __HAL_RCC_ADC_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define ADCx_CHANNEL_PIN                GPIO_PIN_0
#define ADCx_CHANNEL_GPIO_PORT          GPIOB

/* Definition for ADCx's Channel */
#define ADCx_CHANNEL                    ADC_CHANNEL_8

/* Definition for ADCx's DMA */
//#define ADCx_DMA_CHANNEL                DMA_CHANNEL_0
//#define ADCx_DMA_STREAM                 DMA2_Stream0         

/* Definition for ADCx's NVIC */
//#define ADCx_DMA_IRQn                   DMA2_Stream0_IRQn
//#define ADCx_DMA_IRQHandler             DMA2_Stream0_IRQHandler

/*	modbus defines */
#define S_REG_INPUT_START 0
#define S_REG_INPUT_NREGS 600

#define	S_REG_HOLDING_START	0
#define	S_REG_HOLDING_NREGS	600

#define	S_COIL_START	0
#define	S_COIL_NCOILS	600

/*	Timer 1 PWM out	at 10KHz	*/
#define  PERIOD_VALUE					(1800 - 1)  /* Period Value  */
#define  PULSE1_VALUE					1350        /* Capture Compare 1 Value  */

/* Size of I2C_Transmission buffer */
#define TXBUFFERSIZEI2C					(COUNTOF(aTxBuffer_I2C))
/* Size of I2C_Reception buffer */
#define RXBUFFERSIZEI2C					TXBUFFERSIZEI2C

/* Size of USART_Transmission buffer */
#define TXBUFFERSIZEUSART				(COUNTOF(aTxBuffer_Usart) - 1)
/* Size of USART_Reception buffer */
#define RXBUFFERSIZEUSART				TXBUFFERSIZEUSART
  
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */
int test_Cplusplus(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
