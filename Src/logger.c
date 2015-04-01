/**
  ******************************************************************************
  * @file    logger.c 
  * @author  Multicalor
  * @version V1.0.0
  * @date    09-03-2015
  * @brief   TimeStamped Logger
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
#include "logger.h"

/* Static variables ----------------------------------------------------------*/
static void create_timestamp(void);

/* Implements ----------------------------------------------------------------*/
/**
  * @brief  Log all info
  * @param  *info string
  *	@param	args
  * @retval None
  */
void __logger_info(uint8_t *info, ...){	
	va_list arp;
	
	va_start(arp, info);
	
	f_printf(&MyFile, "%s: %d C\n", info, (uint8_t)va_arg(arp, int));
	
	va_end(arp);
	
	f_sync(&MyFile);
}

/**
  * @brief  Create a logger
  * @param  *filename -> long filename support Fatfs
  * @retval None
  */
void create_logger(uint8_t *filename){
	if(f_open(&MyFile, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK){
		Error_Handler();
	}
	
	printf("File created\n");
	
	f_sync(&MyFile);
}