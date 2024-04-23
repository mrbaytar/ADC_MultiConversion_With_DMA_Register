#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

void ADC_Init(void);
void ADC_Enable(void);
void ADC_Start(void);
void DMA_Init(void);
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint16_t size);

uint16_t RxData[2];

int main(void)
{
 HAL_Init();

 /* Configure the system clock */
  SystemClock_Config();

  MX_GPIO_Init();

  ADC_Init();
  ADC_Enable();
  DMA_Init();

  DMA_Config((uint32_t) &ADC1->DR, (uint32_t) RxData, 2);

  ADC_Start();

  while (1)
  {

  }
}

void ADC_Init(void)
{
	/************** STEPS TO FOLLOW *****************
		1. Enable ADC and GPIO clock
		2. Set the prescaler in the Common Control Register (CCR)
		3. Set the Scan Mode and Resolution in the Control Register 1 (CR1)
		4. Set the Continuous Conversion, EOC, and Data Alignment in Control Reg 2 (CR2)
		5. Set the Sampling Time for the channels in ADC_SMPRx
		6. Set the Regular channel sequence length in ADC_SQR1
		7. Set the Respective GPIO PINs in the Analog Mode
	************************************************/
	// 1. Enable ADC and GPIO clock
	RCC->APB2ENR |= (1 << 8); // ADC1 clock enable
	RCC->AHB1ENR |= (1 << 0); // GPIO port A clock enable

	// 2. Set the prescaler in the Common Control Register (CCR)
	ADC->CCR |=  (2 << 16); // PCLK2 divided by 6 (DC_CLK = 90/6 = 15MHz)

	// 3. Set the Scan Mode and Resolution in the Control Register 1 (CR1)
	ADC1->CR1 |= (1 << 8); // Scan mode enabled
	ADC1->CR1 &= ~(1 << 24); // 12-bit Resolution

	// 4. Set the Continuous Conversion, EOC, and Data Alignment in Control Reg 2 (CR2)
	ADC1->CR2 |= (1 << 1); // Enable continuous conversion mode
	ADC1->CR2 |= (1 << 10); // The EOC bit is set at the end of each regular conversion. Overrun detection is enabled.
	ADC1->CR2 &= ~(1 << 11); // Data RIGHT alignment
	// DMA mode enabled
	ADC1->CR2 |= (1 << 8);
	// DMA requests are issued as long as data are converted and DMA=1 (Enable Continuous Request)
	ADC1->CR2 |= (1 << 9);

	// 5. Set the Sampling Time for the channels in ADC_SMPRx
	ADC1->SMPR2 &= ~((1 << 3) | (1 << 12)); // Sampling time of 3 cycles for channel 1 and channel 4

	// 6. Set the Regular channel sequence length in ADC_SQR1
	ADC1->SQR1 |= (1 << 20); // SQR1_L =2 for 2 conversions

	// Channel Sequence
	ADC1->SQR3 |= (1 << 0);  // SEQ1 for Channel 1
	ADC1->SQR3 |= (4 << 5);  // SEQ2 for Channel 4

	// 7. Set the Respective GPIO PINs in the Analog Mode
	GPIOA->MODER |= (3 << 2); // analog mode for PA 1
	GPIOA->MODER |= (3 << 8); // analog mode for PA 4
}

void ADC_Enable(void)
{
	/************** STEPS TO FOLLOW *****************
		1. Enable the ADC by setting ADON bit in CR2
		2. Wait for ADC to stabilize (approx 10us)
	************************************************/
	ADC1->CR2 |= (1 << 0); // ADON =1 enable ADC1

	uint32_t delay = 10000;
	while (delay--);
}

void ADC_Start(void)
{
	/************** STEPS TO FOLLOW *****************
		1. Clear the Status register
		2. Start the Conversion by Setting the SWSTART bit in CR2
	************************************************/
	ADC1->SR = 0; // clear the status register
	ADC1->CR2 |= (1 << 30); // Starts conversion of regular channels
}

void DMA_Init(void)
{
	/************** STEPS TO FOLLOW *****************
		1. Enable DMA clock
		2. Set the DATA Direction
		3. Enable/Disable the Circular Mode
		4. Enable/Disable the Memory Increment and Peripheral Increment
		5. Set the Data Size
		6. Select the channel for the Stream
	************************************************/
	// 1. Enable DMA clock
	RCC->AHB1ENR |= (1 << 22); //DMA2EN = 1
	// 2. Set the DATA Direction
	DMA2_Stream0->CR &= ~(1 << 6); // Peripheral to memory
	// 3. Enable/Disable the Circular Mode
	DMA2_Stream0->CR |= (1 << 8); // Circular mode enabled
	// 4. Enable/Disable the Memory Increment and Peripheral Increment
	DMA2_Stream0->CR |= (1 << 10); // Memory address pointer is incremented after each data transfer (Enable Memory Address Increment)
	// 5. Set the Data Size
	DMA2_Stream0->CR |= (1 << 13); // Memory data size half-word (16-bit)
	DMA2_Stream0->CR |= (1 << 11); // Peripheral data size half-word (16-bit)
	// 6. Select the channel for the Stream
	DMA2_Stream0->CR &= ~(0 << 25); // channel 0 selected
}

void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint16_t size)
{
	/************** STEPS TO FOLLOW *****************
	1. Set the Data Size in the NDTR Register
	2. Set the Peripheral Address and the Memory Address
	3. Enable the DMA Stream
	************************************************/
	// 1. Set the Data Size in the NDTR Register
	DMA2_Stream0->NDTR = size; // Set the size of the transfer
	// 2. Set the Peripheral Address and the Memory Address
	DMA2_Stream0->PAR = srcAdd; // Source address is peripheral address
	DMA2_Stream0->M0AR = destAdd; // Destination Address is memory address
	// 3. Enable the DMA Stream
	DMA2_Stream0->CR |= (1 << 0); // Enable the DMA Stream
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
