/**
  ******************************************************************************
  * @file    ntc.c 
  * @author  Multicalor
  * @version V1.0.0
  * @date    06-03-2015
  * @brief   Read temperature
	@brief	Verventa 1.0.0
  ******************************************************************************
  * @attention
  *
  * 
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Verventa nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *		Verventa is copyrighted!
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
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "arm_math.h"
#include "GUI_verventa.h"
#include "logger.h"
  
/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef    		AdcHandle;
  
/*	ADC buffer	*/
__IO float uhADCxConvertedValue = 0;

/* Static variables ----------------------------------------------------------*/
const float a = 0.8142588126E-3;
const float b = 2.118034328E-4;
const float c = 0.6636952406E-7;

/* Private Functions ---------------------------------------------------------*/
static void get_vADC(void);
static void convert_cTemperature(void);

/******************************************************************************/
/*	Implement ----------------------------------------------------------------*/
/**
	* @brief	convert_cTemperature().
	* @param	None
	* @note		Steinhart-hart
	*			converter.
	* @retval	None
*/
static void convert_cTemperature(void){	
	uint16_t 	ADC_res 	= 4096;
	uint32_t	pad_res 	= 100000;
	float		therm_res;
	float		temp_val;
	float		temperature;
	
	therm_res = pad_res / ((ADC_res / uhADCxConvertedValue) - 1);
	
	temp_val = log(therm_res);
	
	temperature = 1 / (a + (b * temp_val) + (c * temp_val * temp_val * temp_val));
	temperature = temperature - 273.15;

	logger_info("Temperature", (uint8_t)temperature);
	AGUI_UpdateCurTemperature((int8_t)temperature);
}

/**
	* @brief	get_vADC().
	* @param	None
	* @note		read ADC value
	*			polled.
	* @retval	None
*/

static void get_vADC(void){
	if(HAL_ADC_Start(&AdcHandle) != HAL_OK)
	{
		/* Start Conversation Error */
		Error_Handler();
		//printf("conversion error\n");
	}
			
	HAL_ADC_PollForConversion(&AdcHandle, 1);
			
	if(HAL_ADC_GetState(&AdcHandle) == HAL_ADC_STATE_EOC_REG)
	{
		uhADCxConvertedValue = HAL_ADC_GetValue(&AdcHandle);
		//printf("NTC voltage: %d\n", uhADCxConvertedValue);
	}
}

/**
	* @brief	set_Temperature().
	* @param	None
	* @note		API for temperature
	*	
	* @retval	None
*/
void set_Temperature(void){
	get_vADC();
	
	convert_cTemperature();
}
  