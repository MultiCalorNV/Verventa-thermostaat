/**
  ******************************************************************************
  * @file    logger.h
  * @author  Multicalor
  * @version V1.0.0
  * @date    09-03-2015
  * @brief   header file for logger.c
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
#include "main.h"
#include <Stdarg.h>

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FATFS_LOGGER_H
#define __FATFS_LOGGER_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef FATFS_LOGGER_FMT
	#define FATFS_LOGGER_FMT "%10s"
#endif

/* Exported functions ------------------------------------------------------- */
FIL MyFile;
uint8_t *info;

/* Exported functions ------------------------------------------------------- */
void create_logger(uint8_t *filename);
void __logger_info(uint8_t *info, ...);

/**
  *	@brief	Log an info message to FILE myfile.
  */
#ifdef LOG_INFO
#define logger_info(info, ...)	__logger_info(info, __VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif