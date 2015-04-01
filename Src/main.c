/*****************************************************************************
	*	MULTICALOR NV
	*	
	*	File:	main.c
	*		Update regularly the flags
	*
	******************************************************************************
	*	Verion V1.0.0
	*	Date: 21-01-2015
	*	Main program body
	******************************************************************************
*/
#if(defined(__VFP_FP__))
	#warning "__VFP_FP__ macro defined!"
#else
	#warning "__VFP_FP__ macro not defined!"
#endif
	
#if(defined(__FPU_USED))
	#warning "__FPU_USED macro defined!"
#else
	#warning "__FPU_USED macro not defined!"
#endif

/*****************************************
	*	Newlib libc
	*
	******************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*****************************************
	*	Include peripheral drivers
	*
	******************************************
*/
#include "main.h"
#include "stm32f4xx_it.h"

/*****************************************
	*	Include exported libs
	*
	******************************************
*/
#include "../drivers/display/ili_9341.h"
#include "stm32_ub_stemwin.h"
#include "GUI_verventa.h"
#include "ntc.h"
#include "logger.h"

/*	Modbus includes --------------------------------------------------*/
#include "mb.h"
#include "mbport.h"


/* Private Functions -------------------------------------------------*/
static void SystemClock_Config(void);
//static void Error_Handler(void);
static void init_leds(void);
static void sram_init(void);
static void I2C_init(void);
static void USART1_init(void);
static void ADC1_init(void);
static void EXTILine2_Config(void);
static void sd_detect_init(void);
static void timer_4_init(void);
static void timer_5_init(void);
static void Timer9_Init(void);

/* Private macro -----------------------------------------------------*/
/* Private variables -------------------------------------------------*/
SRAM_HandleTypeDef			hsram;
FMC_NORSRAM_TimingTypeDef	SRAM_Timing;
I2C_HandleTypeDef			I2cHandle;
UART_HandleTypeDef			UartHandle;
ADC_HandleTypeDef    		AdcHandle;
HAL_SD_CardInfoTypedef		SDCardInfo;
TIM_HandleTypeDef			TimHandle_9;
TIM_HandleTypeDef 			TimHandle_4;
TIM_HandleTypeDef 			TimHandle_5;

/*	Modbus variables */
bool            xtNeedPoll = true;
static uint16_t usRegInputStart = S_REG_INPUT_START;	//Input Register variables
static uint16_t usRegInputBuf[S_REG_INPUT_NREGS];		//Input Register variables

static uint16_t	usSRegHoldStart = S_REG_HOLDING_START;	//HoldingRegister variables
static uint16_t	usSRegHoldBuf[S_REG_HOLDING_NREGS];		//HoldingRegister variables

static uint16_t usSCoilStart = S_COIL_START;
#if S_COIL_NCOILS%8
	static uint8_t ucSCoilBuf[S_COIL_NCOILS/8+1];
	#else
	static uint8_t ucSCoilBuf[S_COIL_NCOILS/8];
#endif

/*	FATfs	*/
FATFS	SDFatFs;  	/* File system object for SD card logical drive */
char	SDPath[4];	/* SD card logical drive path */

/* External types ------------------------------------------------------------*/
extern bool gui_Exec;
extern bool modbus_Exec;
extern bool Update_Touch;
extern bool log_exec;

/* Defines -----------------------------------------------------------*/
#define	I2C_ADDRESS		0xFE

//****************************************************************************
/* Static variables --------------------------------------------------*/
static int Debug_ITMDebug = 0;

//****************************************************************************

void Debug_ITMDebugEnable(void){
	volatile unsigned int *ITM_TER      	= (volatile unsigned int *)0xE0000E00;
	volatile unsigned int *SCB_DHCSR 		= (volatile unsigned int *)0xE000EDF0;
	volatile unsigned int *DBGMCU_CR 		= (volatile unsigned int *)0xE0042004;
	
	*DBGMCU_CR |= 0x27; // DBGMCU_CR
	
	if ((*SCB_DHCSR & 1) && (*ITM_TER & 1)) // Enabled?
    Debug_ITMDebug = 1;
}

//****************************************************************************

void Debug_ITMDebugOutputChar(char ch){
	static volatile unsigned int *ITM_STIM0 = (volatile unsigned int *)0xE0000000; // ITM Port 0
	static volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC;
	
	if (Debug_ITMDebug && (*SCB_DEMCR & 0x01000000))
	{
		while(*ITM_STIM0 == 0);
		*((volatile char *)ITM_STIM0) = ch;
	}
}

//****************************************************************************

void Debug_ITMDebugOutputString(char *Buffer){
	if (Debug_ITMDebug)
	while(*Buffer)
	Debug_ITMDebugOutputChar(*Buffer++);
}

//******************************************************************************


/**
	* @brief	main().
	* @param	None
	* @note		Init the system
	*			Setup modbus
	*			Setup emWin
	* 			Update GUI every 10ms
	* 			Update Touch every 1ms
	* 			return from callee.
	* @retval	None
*/
int main(void){
	uint16_t 			counter = 1;
	eMBErrorCode		eStatus;
	DSTATUS				Disk_State;
	uint8_t				Disk_label[12];

	/* STM32f4xx HAL library initialization:
		- Cofigure Flash prefetch, flash preread and Buffer caches
		- Systick timer config
		- Low level init
	*/
	HAL_Init();
	/*********************************************************************/
	
	/* configure the system clock to 168 MHz */
	SystemClock_Config();
	
	/* Enable TRACE debug -----------------------------------------------*/
	Debug_ITMDebugEnable();
 	Debug_ITMDebugOutputString("SWV Enabled\n");
	/*********************************************************************/
	
	/* Init peripherals -------------------------------------------------*/
	sram_init();
	I2C_init();
	USART1_init();
	ADC1_init();
	EXTILine2_Config();
	Timer9_Init();
	//timer_4_init();
	timer_5_init();
	/*********************************************************************/
	
	/* Show us some status leds ----------------------------------------*/
	init_leds();
	
	/*********************************************************************/
	
	/*	Init modbus	slave -----------------------------------------------*/
	eStatus = eMBInit(MB_RTU, 0x0B, 0, 115200, MB_PAR_NONE);
	printf("MB init status: %s\n", eStatus ? "error": "no'error");
	
	/*	Enable the Modbus Protocol Stack --------------------------------*/
	eStatus = eMBEnable();
	printf("MB enable status: %s\n", eStatus ? "error": "no'error");
	
	/*********************************************************************/
	
	/*	Init STemWin V5.2.6 and GUI -------------------------------------*/
	if((UB_STemWIN_init()) != SUCCESS){
		printf("emWin INIT error...");
	}
	
	/*	Init Verventa GUI -----------------------------------------------*/
	if(( Init_GUI_verventa()) != SUCCESS){
		printf("can't INIT verventa GUI...");
	}
	
	/*	Link disk drive to filesystem */
	if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0){
		printf("SD driver linked...\n");
		if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 1) != FR_OK)
		{
			printf("No sd card!\n");
			//FatFs Initialization Error
			//Error_Handler();
		}
		else
		{
			if(f_mkfs((TCHAR const*)SDPath, 0, 0) != FR_OK)
			{
				//FatFs Format Error
				Error_Handler();
			}
			if(f_setlabel("LOG DISK") != FR_OK)
			{
				Error_Handler();
			}
			else{
				/*if(f_getlabel((TCHAR const*)SDPath, Disk_label, 0) != FR_OK)
				{
					Error_Handler();
				}*/
				/*else{
					printf("Disk Label: %s\n", Disk_label);
				}*/
				create_logger("march.log");
			}
			Disk_State = disk_status(0);
			printf("Disk_State: %d\n", Disk_State);
		}
	}
	
	
	while(1){
		
		//	printf("Compiled HAL\n");
		
		//	test_Cplusplus();
		
		if(xtNeedPoll == true){
			printf("xtNeedPoll: %s\n", xtNeedPoll ? "true" : "false");
			usRegInputBuf[499]++;
			usRegInputBuf[500]++;
			usRegInputBuf[501]++;
			usRegInputBuf[502]++;
			usSRegHoldBuf[149]++;
			usSRegHoldBuf[150]++;
			usSRegHoldBuf[151]++;
			usSRegHoldBuf[152]++;
			ucSCoilBuf[10] = 0b00010001;
			ucSCoilBuf[11] = 0b10010001;
			ucSCoilBuf[12] = 0b11010001;
			xtNeedPoll = false;
		}
		
		if(modbus_Exec == true){
			eStatus = eMBPoll();
			printf("MB status: %d\n", eStatus);
			
			/*	Clear Flag */
			modbus_Exec = false;
		}
		
		if(gui_Exec == true){
			WM_InvalidateWindow(hWinFanImg);
			GUI_Exec();
			
			//printf("SystemCoreClock: %d\n", SystemCoreClock);
			
			/*	Clear Flag */
			gui_Exec = false;
		}
		
		if(log_exec == true){
			set_Temperature();
			
			/*	Clear Flag */
			log_exec = false;
		}
		
		if(Update_Touch == true){
			UB_STemWIN_updateTouch();
			
			/*	Clear Flag */
			Update_Touch = false;
		}	
	}
}

/**
	* @brief	Builds the Input Registers frame.
	* @param	*pucRegBuffer
	* @param	usAddress
	* @param	usNRegs
	* @retval	eMBErrorCode
*/
eMBErrorCode
eMBRegInputCB(uint8_t* pucRegBuffer, uint16_t usAddress, uint16_t usNRegs)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
	
	/* it already plus one in modbus function method. */
    usAddress--;
	
    if( (usAddress >= S_REG_INPUT_START)
	&& (usAddress + usNRegs <= S_REG_INPUT_START + S_REG_INPUT_NREGS))
    {
        iRegIndex = (int)(usAddress - usRegInputStart);
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = (unsigned char)(usRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (unsigned char)(usRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
		}
	}
    else
    {
        eStatus = MB_ENOREG;
	}
	
    return eStatus;
}

/**
	* @brief	Builds the Holding Registers frame.
	* @param	*pucRegBuffer
	* @param	usAddress
	* @param	usNRegs
	* @param	eMode
	* @retval	eMBErrorCode
*/
eMBErrorCode
eMBRegHoldingCB(uint8_t* pucRegBuffer, uint16_t usAddress, uint16_t usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    uint16_t          iRegIndex;
    uint16_t *        pusRegHoldingBuf;
    uint16_t          REG_HOLDING_START;
    uint16_t          REG_HOLDING_NREGS;
    uint16_t          usRegHoldStart;
	
    pusRegHoldingBuf = usSRegHoldBuf;
    REG_HOLDING_START = S_REG_HOLDING_START;
    REG_HOLDING_NREGS = S_REG_HOLDING_NREGS;
    usRegHoldStart = usSRegHoldStart;
	
    /* it already plus one in modbus function method. */
    usAddress--;
	
    if ((usAddress >= REG_HOLDING_START)
	&& (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode)
        {
			/* read current register values from the protocol stack. */
			case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (uint8_t) (pusRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (uint8_t) (pusRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
			}
            break;
			
			/* write current register values with new values from the protocol stack. */
			case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
			}
            break;
		}
	}
    else
    {
        eStatus = MB_ENOREG;
	}
    return eStatus;
}

/**
	* @brief	Builds the Coils frame.
	* @param	*pucRegBuffer
	* @param	usAddress
	* @param	usNCoils
	* @param	eMode
	* @retval	eMBErrorCode
*/
eMBErrorCode
eMBRegCoilsCB(uint8_t* pucRegBuffer, uint16_t usAddress, uint16_t usNCoils, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    uint16_t          iRegIndex , iRegBitIndex , iNReg;
    uint8_t *         pucCoilBuf;
    uint16_t          COIL_START;
    uint16_t          COIL_NCOILS;
    uint16_t          usCoilStart;
    iNReg =  (usNCoils / 8) + 1;
	printf("usNCoils: %d\n", usNCoils);
	printf("iNReg: %d\n", iNReg);
	
    pucCoilBuf = ucSCoilBuf;
    COIL_START = S_COIL_START;
    COIL_NCOILS = S_COIL_NCOILS;
    usCoilStart = usSCoilStart;
	
    /* it already plus one in modbus function method. */
    usAddress--;
	printf("usAddress: %d\n", usAddress);
	
    if( (usAddress >= COIL_START) &&
	(usAddress + usNCoils <= COIL_START + COIL_NCOILS))
    {
        iRegIndex = (uint16_t) (usAddress - usCoilStart) / 8;
        iRegBitIndex = (uint16_t) (usAddress - usCoilStart) % 8;
		printf("iRegIndex: %d\n", iRegIndex);
		printf("iRegBitIndex: %d\n", iRegBitIndex);
        switch (eMode)
        {
			/* read current coil values from the protocol stack. */
			case MB_REG_READ:
            while (iNReg > 0)
            {
                *pucRegBuffer++ = xMBUtilGetBits(&pucCoilBuf[iRegIndex++],
				iRegBitIndex, 8);
                iNReg--;
			}
            pucRegBuffer--;
            /* last coils */
            usNCoils = usNCoils % 8;
            /* filling zero to high bit */
            *pucRegBuffer = *pucRegBuffer << (8 - usNCoils);
            *pucRegBuffer = *pucRegBuffer >> (8 - usNCoils);
            break;
			
            /* write current coil values with new values from the protocol stack. */
			case MB_REG_WRITE:
            while (iNReg > 1)
            {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8,
				*pucRegBuffer++);
                iNReg--;
			}
            /* last coils */
            usNCoils = usNCoils % 8;
            /* xMBUtilSetBits has bug when ucNBits is zero */
            if (usNCoils != 0)
            {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
				*pucRegBuffer++);
			}
            break;
		}
	}
    else
    {
        eStatus = MB_ENOREG;
	}
    return eStatus;
}

/**
	* @brief	Builds the Discrete frame.
	* @param	*pucRegBuffer
	* @param	usAddress
	* @param	usNDiscrete
	* @retval	eMBErrorCode
*/
eMBErrorCode
eMBRegDiscreteCB(uint8_t* pucRegBuffer, uint16_t usAddress, uint16_t usNDiscrete)
{
    ( void )pucRegBuffer;
    ( void )usAddress;
    ( void )usNDiscrete;
    return MB_ENOREG;
}

/**
	* @brief	System Clock configuration
	*		System clock Source	= PLL(HSE)
	*		SYSCLK(Hz)		= 168000000
	*		HCLK(Hz)		= 168000000
	*		AHB Prescale		= 1
	*		APB1 Prescale		= 4
	*		APB2 Prescale		= 2
	*		HSE frequency(Hz)	= 25000000
	*		PLL_M			= 25
	*		PLL_N			= 336
	*		PLL_P			= 2
	*		PLL_Q			= 7
	*		VDD(V)			= 3.3
	*		Main reg out Volt	= Scale1 mode
	*		Flash lat(ws)		= 5
	*	@param None
	*	@retval None
*/
static void SystemClock_Config(void){
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	
	/* Enable Power control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	
	/* The voltage scaling allows optimizing the power consumption when the
		device is clocked below the maximum system frequency, to update the
		voltage scaling value regarding system frequency refer to product
	datasheet. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
		/* Initialization Error */
		Error_Handler();
	}
	/* Select PLL as system clock source and configure the HCLK, PCLK1
	and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK	| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK){
		/* Initialization Error */
		Error_Handler();
	}
	
	/* STM32F407x Revision Z devices: prefetch is supported */
	if(HAL_GetREVID() == 0x1001){
		/* Enable the Flash prefetch */
		__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
	}
}


/**
	* @brief	Handles general errors.
	* @param	None
	* @retval	None
*/
void Error_Handler(void){
	
	while(1){
		/* Put error on LED3 */
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		printf("Peripheral config error\n");
	}
}

/**
	* @brief	Init leds PD12-13
	* @param	None
	* @retval	None
*/
static void init_leds(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// Enable AHB1per Clock
	__GPIOD_CLK_ENABLE();
	
	//Config pins
	GPIO_InitStructure.Pin		= GPIO_PIN_13;
	GPIO_InitStructure.Mode		= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed	= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull		= GPIO_PULLUP;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/**
	* @brief init SRAM
	* @param None
	* @retval None
*/
static void sram_init(void){
	/*	Configure the FMC bus ---------------------------*/
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	
	SRAM_Timing.AddressSetupTime       = 2;
	SRAM_Timing.AddressHoldTime        = 1;
	SRAM_Timing.DataSetupTime          = 2;
	SRAM_Timing.BusTurnAroundDuration  = 1;
	SRAM_Timing.CLKDivision            = 2;
	SRAM_Timing.DataLatency            = 2;
	SRAM_Timing.AccessMode             = FMC_ACCESS_MODE_A;
	
	hsram.Init.NSBank				= FMC_NORSRAM_BANK1;
	hsram.Init.DataAddressMux		= FMC_DATA_ADDRESS_MUX_DISABLE;
	hsram.Init.MemoryType			= FMC_MEMORY_TYPE_SRAM;
	hsram.Init.MemoryDataWidth		= FMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram.Init.BurstAccessMode		= FMC_BURST_ACCESS_MODE_DISABLE;
	hsram.Init.WaitSignalPolarity	= FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram.Init.WrapMode				= FMC_WRAP_MODE_DISABLE;
	hsram.Init.WaitSignalActive		= FMC_WAIT_TIMING_BEFORE_WS;
	hsram.Init.WriteOperation		= FMC_WRITE_OPERATION_ENABLE;
	hsram.Init.WaitSignal			= FMC_WAIT_SIGNAL_DISABLE;
	hsram.Init.ExtendedMode			= FMC_EXTENDED_MODE_DISABLE;
	hsram.Init.AsynchronousWait		= FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram.Init.WriteBurst			= FMC_WRITE_BURST_DISABLE;
	hsram.Init.ContinuousClock    	= FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
	
	/* Initialize the SRAM controller ----------------------------*/
	if(HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK){
		/* Init Error */
		Error_Handler();
	}
}

/**
	* @brief init I2C1
	* @param None
	* @retval None
*/
static void I2C_init(void){
	/*	Configure the I2C peripheral ---------------------------*/
	I2cHandle.Instance             = I2Cx;
	
	I2cHandle.Init.AddressingMode	= I2C_ADDRESSINGMODE_7BIT;
	I2cHandle.Init.ClockSpeed		= 400000;
	I2cHandle.Init.DualAddressMode	= I2C_DUALADDRESS_DISABLE;
	I2cHandle.Init.DutyCycle		= I2C_DUTYCYCLE_2;
	I2cHandle.Init.GeneralCallMode	= I2C_GENERALCALL_ENABLE;
	I2cHandle.Init.NoStretchMode	= I2C_NOSTRETCH_DISABLE;
	I2cHandle.Init.OwnAddress1		= I2C_ADDRESS;
	I2cHandle.Init.OwnAddress2		= I2C_ADDRESS;
	
	if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();    
	}
}

/**
	* @brief init USART1
	* @param None
	* @retval None
*/
static void USART1_init(void){
	uint32_t baudrate = 19200;
	/*	Configure the USART1 peripheral in the Asynchronous mode (UART Mode)------*/
	/*	Configure the USART1 peripheral in the Asynchronous mode (UART Mode)------*/
	/*UART1 configured as follow:
		- Word Length	= 8 Bits
		- Stop Bit		= One Stop bit
		- Parity		= None
		- BaudRate		= 115200 baud
		- Hardware flow control disabled (RTS and CTS signals) */
	UartHandle.Instance          = USARTx;
	
	UartHandle.Init.BaudRate     = baudrate;
	UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits     = UART_STOPBITS_1;
	UartHandle.Init.Parity       = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode         = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
	if(HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief  Configures sd detect pin
  * @param  None
  * @retval None
  */
static void sd_detect_init(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOB clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  /* Configure PB8 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
  GPIO_InitStructure.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief  Configures EXTI Line2 in interrupt mode
  * @param  None
  * @retval None
  */
static void EXTILine2_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* Configure PC2 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

/**
	* @brief init ADC1 for NTC1
	* @param None
	* @retval None
*/
static void ADC1_init(void){
	ADC_ChannelConfTypeDef sConfig;
	
	AdcHandle.Instance = ADCx;
	
	AdcHandle.Init.ClockPrescaler			= ADC_CLOCKPRESCALER_PCLK_DIV2;
	AdcHandle.Init.Resolution				= ADC_RESOLUTION_12B;
	AdcHandle.Init.ScanConvMode				= DISABLE;
	AdcHandle.Init.ContinuousConvMode		= DISABLE;
	AdcHandle.Init.DiscontinuousConvMode	= DISABLE;
	AdcHandle.Init.NbrOfDiscConversion		= 0;
	AdcHandle.Init.ExternalTrigConvEdge		= ADC_EXTERNALTRIGCONVEDGE_NONE;
	AdcHandle.Init.ExternalTrigConv			= ADC_EXTERNALTRIGCONV_T1_CC1;
	AdcHandle.Init.DataAlign				= ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.NbrOfConversion			= 1;
	AdcHandle.Init.DMAContinuousRequests	= DISABLE;
	AdcHandle.Init.EOCSelection				= DISABLE;
		  
	if(HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
	
	sConfig.Channel			= ADCx_CHANNEL;
	sConfig.Rank 			= 1;
	sConfig.SamplingTime 	= ADC_SAMPLETIME_28CYCLES;
	sConfig.Offset 			= 0;
	
	if(HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	{
		/* Channel Configuration Error */
		Error_Handler();
	}
}

/**
	* @brief init Timer9 for PWM
	* @param None
	* @retval None
*/
static void Timer9_Init(void){
	TIM_OC_InitTypeDef sConfig;
	uint32_t uwPrescalerValue = 0;
	
	uwPrescalerValue = ((SystemCoreClock /2) / 18000000) - 1;
	
	TimHandle_9.Instance = TIMx;
	
	TimHandle_9.Init.Prescaler     = uwPrescalerValue;
	TimHandle_9.Init.Period        = PERIOD_VALUE;
	TimHandle_9.Init.ClockDivision = 0;
	TimHandle_9.Init.CounterMode   = TIM_COUNTERMODE_UP;
	if(HAL_TIM_PWM_Init(&TimHandle_9) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
	
	sConfig.OCMode			= TIM_OCMODE_PWM1;
	sConfig.OCPolarity		= TIM_OCPOLARITY_HIGH;
	sConfig.OCNPolarity		= TIM_OCNPOLARITY_HIGH;
	sConfig.OCNIdleState	= TIM_OCNIDLESTATE_RESET;
	sConfig.OCIdleState		= TIM_OCIDLESTATE_RESET;
	sConfig.OCFastMode		= TIM_OCFAST_DISABLE;
	
	sConfig.Pulse = PULSE1_VALUE;  
	if(HAL_TIM_PWM_ConfigChannel(&TimHandle_9, &sConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Configuration Error */
		Error_Handler();
	}
	
	if(HAL_TIM_PWM_Start(&TimHandle_9, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}
}

/**
  * @brief init timer_4
  * @param None
  * @retval None
  */
static void timer_4_init(void){
	RCC_ClkInitTypeDef sClokConfig;
	uint32_t uwTimclock, uwAPB1Prescaler = 0;
	uint32_t uwPrescalerValue = 0;
	uint32_t pFLatency;
	
	HAL_RCC_GetClockConfig(&sClokConfig, &pFLatency);
	
	uwAPB1Prescaler = sClokConfig.APB1CLKDivider;
	if (uwAPB1Prescaler == 0) 
	{
		uwTimclock = HAL_RCC_GetPCLK1Freq();
	}
	else
	{
		uwTimclock = 2 * HAL_RCC_GetPCLK1Freq();
	}
	
	uwPrescalerValue = (uint32_t) ((uwTimclock / 20000) - 1);
	
	TimHandle_4.Instance = TIMx_4;
	
	TimHandle_4.Init.Period			= 20000 - 1;
	TimHandle_4.Init.Prescaler		= uwPrescalerValue;
	TimHandle_4.Init.ClockDivision	= 0;
	TimHandle_4.Init.CounterMode	= TIM_COUNTERMODE_UP;
	if(HAL_TIM_Base_Init(&TimHandle_4) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
}

/**
  * @brief init timer_5 at 20KHz
  * @param None
  * @retval None
  */
static void timer_5_init(void){
	RCC_ClkInitTypeDef sClokConfig;
	uint32_t uwTimclock, uwAPB1Prescaler = 0;
	uint32_t uwPrescalerValue = 0;
	uint32_t pFLatency;
	
	HAL_RCC_GetClockConfig(&sClokConfig, &pFLatency);
	
	uwAPB1Prescaler = sClokConfig.APB1CLKDivider;
	if (uwAPB1Prescaler == 0) 
	{
		uwTimclock = HAL_RCC_GetPCLK1Freq();
	}
	else
	{
		uwTimclock = 2 * HAL_RCC_GetPCLK1Freq();
	}
	
	uwPrescalerValue = (uint32_t) ((uwTimclock / 20000) - 1);
	
	TimHandle_5.Instance = TIMx_5;
	
	TimHandle_5.Init.Period			= 70 - 1;
	TimHandle_5.Init.Prescaler		= uwPrescalerValue;
	TimHandle_5.Init.ClockDivision	= 0;
	TimHandle_5.Init.CounterMode	= TIM_COUNTERMODE_UP;
	if(HAL_TIM_Base_Init(&TimHandle_5) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
}

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  AdcHandle : AdcHandle handle
  * @note
  *
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	//printf("NTC voltage: %d\n", uhADCxConvertedValue);
}

/******************************************************************************/

/**
	* @brief  Retargets the C library printf function to the USART (GNU)
	* @param  None
	* @retval None
*/
int __io_putchar(int ch){
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	
	Debug_ITMDebugOutputChar(ch); // SWV
	
	return(ch);
}

//******************************************************************************

/**
	* @brief  Retargets the C library scanf function to the USART (GNU)
	* @param  None
	* @retval None
*/
int __io_getchar(void){
	/* Place your implementation of fgetc here */
	/* e.g. read a character from the USART */
	
	return((int)-1);
}

//******************************************************************************

#ifdef  USE_FULL_ASSERT
	/**
		* @brief  Reports the name of the source file and the source line number
		*         where the assert_param error has occurred.
		* @param  file: pointer to the source file name
		* @param  line: assert_param error line source number
		* @retval None
	*/
	void assert_failed(uint8_t* file, uint32_t line){
		printf("Wrong parameters value: file %s on line %d\r\n", file, line);
		
		while(1); /* Infinite loop */
	}
#endif

//******************************************************************************
