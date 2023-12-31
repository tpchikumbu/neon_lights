/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "stm32f0xx.h"
#include <lcd_stm32f0.c>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
TIM_HandleTypeDef htim3;

// set the mode for transmission or reception
uint8_t MODE = TRASMIT_MODE; // 0 = sample, 1 = receive

// initialise the samples received and the samples sent
uint32_t samples_received = 0;
uint32_t samples_sent = 0;

/* USER CODE BEGIN PV */

uint32_t ticks_before = 0;
uint32_t ticks_currrent = 0;
uint32_t delay_t = 500; // Initialise delay to 500ms

uint16_t polling_speed = 500; // Poll every 0.5s 

uint32_t adc_val;       //Value read from ADC
char adc_string[16];    //Printable string

// protocol functions
void transmit(uint16_t package, char* lcd_print);
void receive();
uint16_t package(uint32_t polledData, uint8_t mode);
int package_valid(uint16_t package);

void transmitting_mode();
void receiving_mode();
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
void EXTI0_1_IRQHandler(void);
void writeLCD(char *char_in);
uint32_t pollADC(void);
uint32_t ADCtoCCR(uint32_t adc_val);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
uint16_t package(uint32_t polledData, uint8_t mode) {
  // create the message in the following format: 1 start bit, 1 mode bit, 12 data bits, 1 even parity bit, 1 stop bit
  uint16_t message = 0;
  uint16_t data = polledData & 0x0FFF; // get the 12 data bits
  uint16_t parity = 0; // parity bit
  uint16_t stop = 1; // stop bit
  uint16_t start = 1; // start bit

  //calculating the parity bit
  for (int i = 0; i < 12; i++) {
    if (data & (1 << i)) {
      parity = !parity;
    }
  }

  message = (start << 15) | (mode << 14) | (data << 2) | (parity << 1) | stop;
  return message;
  // if (mode == SAMPLE_TRANSMIT) {
  // } else if (mode == COUNT_TRANSMIT) {
  //   message = (start << 15) | (mode << 14) | (data << 2) | (parity << 1) | stop;
  // }
}

int package_valid(uint16_t package) {
  // 1. check if the first bit is a 1
  // 2. check if the last bit is a 1
  // 3. check if the parity bit is correct
  // 4. return 1 if all checks pass, 0 otherwise
  uint16_t start = package >> 15;
  uint16_t data = (package >> 2) & 0x0FFF;
  uint16_t parity = (package >> 1) & 0x0001;
  uint16_t stop = package & 0x0001;

  // check if the first bit is a 1
  if (start != 1) {
    return 0;
  }
  // check if the last bit is a 1
  if (stop != 1) {
    return 0;
  }
  // checking parity
  uint16_t parity_check = 0;
  for (int i = 0; i < 12; i++) {
    if (data & (1 << i)) {
      parity_check = !parity_check;
    }
  }
  if (parity_check != parity) {
    return 0;
  }
  return 1;
}

void transmit(uint16_t package, char* lcd_print) {
  // send one bit to signify start of transmission
  lcd_command(CLEAR);
  lcd_putstring("Sending: ");
  lcd_command(LINE_TWO);
  lcd_putstring(lcd_print);

  HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, Transmit_Pin, GPIO_PIN_SET);
  HAL_Delay(COMM_DELAY);
  // HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_RESET);
  // HAL_GPIO_WritePin(GPIOB, Transmit_Pin, GPIO_PIN_RESET);
  // clear LCD
  
  // transmit the message to the receiver bit by bit in delays of 500 ms
  for (int i = 15; i >= 0; i--) {
    // lcd_command(CLEAR);
    if (package & (1 << i)) {
      // set the transmit pin high
      HAL_GPIO_WritePin(GPIOB, Transmit_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_SET);
      // write the bit number and its value to the lcd
      // char bit_string[16];
      // sprintf(bit_string, "Bit: %d = 1", i);
      // lcd_putstring(bit_string);
    } else {
      // set the transmit pin low
      HAL_GPIO_WritePin(GPIOB, Transmit_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_RESET);
      // char bit_string[16];
      // sprintf(bit_string, "Bit: %d = 0", i);
      // lcd_putstring(bit_string);
    }
    HAL_Delay(COMM_DELAY);
  }
}
char *intToString(uint16_t num) {
  // write out the number as a string in binary
  char *bit_string = malloc(16 * sizeof(char));
  for (int i = 0; i < 16; i++) {
    if (num & (1 << i)) {
      bit_string[15 - i] = '1';
    } else {
      bit_string[15 - i] = '0';
    }
  }
  return bit_string;
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  init_LCD();

  lcd_putstring("Select mode:");
  lcd_command(LINE_TWO);
  lcd_putstring("SW1:TMT SW2:RCV");

  // infinitely loop until the user presses a button for the mode
  while (1)
  {
    // if button 1 is pressed, set the mode to transmit
    if (HAL_GPIO_ReadPin(GPIOA, Button1_Pin) == GPIO_PIN_RESET) {
      MODE = TRASMIT_MODE;
      break;
    }
    // if button 2 is pressed, set the mode to receive
    if (HAL_GPIO_ReadPin(GPIOA, Button2_Pin) == GPIO_PIN_RESET) {
      MODE = RECEIVE_MODE;
      break;
    }
  }

  lcd_command(CLEAR);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  if (MODE == TRASMIT_MODE) {
    writeLCD("Transmit M");
    HAL_Delay(1000);
    transmitting_mode();
  } else if (MODE == RECEIVE_MODE) {
    writeLCD("Receive M");
    HAL_Delay(1000);
    receiving_mode();
  } else {
    writeLCD("Invalid M");
    HAL_Delay(1000);
  }
}

void sampleAndSend(void) {
  HAL_GPIO_WritePin(GPIOB, LED7_Pin, GPIO_PIN_SET); // Turn on LED7

  // get the value and package it
  uint32_t pollVal = pollADC(); // Read ADC value during polling
  uint16_t message = package(pollVal, SAMPLE_TRANSMIT);

  // write the data to the lcd
  char* bit_string = intToString(message);
  lcd_command(CLEAR); // Clear LCD
  lcd_putstring("Sample TMT:");
  lcd_command(LINE_TWO);
  lcd_putstring(bit_string);

  char* lcd_print = malloc(16 * sizeof(char));
  sprintf(lcd_print, "Sample: %u", pollVal); //Format voltage as string
  // transmit the message
  transmit(message, lcd_print);
  HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_RESET); // Turn off LED6
  HAL_GPIO_WritePin(GPIOB, Transmit_Pin, GPIO_PIN_RESET); // take the transmit pin low

  samples_sent++;
  // stop transmitting for 2 seconds
  HAL_Delay(2000);
}

void countAndSend(void) {
  HAL_GPIO_WritePin(GPIOB, LED7_Pin, GPIO_PIN_SET); // Turn on LED7

  // get the value and package it
  uint32_t count = samples_sent;
  uint16_t message = package(count, COUNT_TRANSMIT);

  // write the data to the lcd
  char* bit_string = intToString(message);
  lcd_command(CLEAR); // Clear LCD
  lcd_putstring("Count Check:");
  lcd_command(LINE_TWO);
  lcd_putstring(bit_string);

  char* lcd_print = malloc(16 * sizeof(char));
  sprintf(lcd_print, "Count: %u", count); //Format voltage as string
  // transmit the message
  transmit(message, lcd_print);

  HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_RESET); // Turn off LED6
  HAL_GPIO_WritePin(GPIOB, Transmit_Pin, GPIO_PIN_RESET); // take the transmit pin low
  // stop transmitting for 2 seconds
  HAL_Delay(2000);
}

void transmitting_mode() {
  // PWM setup
  uint32_t CCR = 0;
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3); // Start PWM on TIM3 Channel 3
  /* USER CODE END 2 */

  while (1)
  {
    // Toggle LED0
    // HAL_GPIO_TogglePin(GPIOB, LED7_Pin);
    // HAL_GPIO_WritePin(GPIOB, LED6_Pin, GPIO_PIN_SET); // Turn on LED6

    // if PB6 pressed, toggle LED7 
    if (HAL_GPIO_ReadPin(GPIOA, Button1_Pin) == GPIO_PIN_RESET) {
      sampleAndSend();
      continue;
    }

    if (HAL_GPIO_ReadPin(GPIOA, Button2_Pin) == GPIO_PIN_RESET) {
      countAndSend();
      continue;
    }

    // the idle display showing the value of the potentiometer
    // ADC to LCD; TODO: Read POT1 value and write to LCD
    uint32_t voltage =  pollADC();
    sprintf(adc_string, "Curr Val: %u", voltage); //Format voltage as string

    // write the data to the lcd
    lcd_command(CLEAR);
    lcd_putstring("Sampling");
    lcd_command(LINE_TWO);
    lcd_putstring(adc_string);
    // writeLCD(adc_string);
    
    // Update PWM value; TODO: Get CCR
    CCR = ADCtoCCR(voltage); //Set CCR to converted value
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, CCR);

    // Wait for delay ms
    HAL_Delay (delay_t);
      /* USER CODE END WHILE */

      /* USER CODE BEGIN 3 */
  }
}


void toggleListen(void) {
  // if led7 is on then turn it off and show that the device is not listening
  if (HAL_GPIO_ReadPin(GPIOB, LED7_Pin) == GPIO_PIN_SET) {
    HAL_GPIO_TogglePin(GPIOB, LED7_Pin);
    lcd_command(CLEAR);
    lcd_putstring("Receive Mode:");
    lcd_command(LINE_TWO);
    lcd_putstring("Off");
  } else {
    // if led7 is off then turn it on and show that the device is listening
    HAL_GPIO_TogglePin(GPIOB, LED7_Pin);
    lcd_command(CLEAR);
    lcd_putstring("Receive Mode:");
    lcd_command(LINE_TWO);
    lcd_putstring("Listening...");
  }
}
void receiving_mode() {
  while (1) {
    // do nothing until button 1 is pressed
    while (1)
    {
      if (HAL_GPIO_ReadPin(GPIOA, Button1_Pin) == GPIO_PIN_RESET) {
        toggleListen();
        break;
      }
    }
    
    // start listening for transmissions
    while (1) {
      // press the button again to stop listening
      if (HAL_GPIO_ReadPin(GPIOA, Button1_Pin) == GPIO_PIN_RESET) {
        toggleListen();
        break;
      }
      // listen if there is a transmission
      // transmission is signified by the LED 6 being on for 1 second
      uint16_t message = 0;
      if (HAL_GPIO_ReadPin(GPIOB, Receive_Pin) == GPIO_PIN_SET) {
        // clear the LCD
        lcd_command(CLEAR);
        lcd_putstring("Receiving...");
        // read the message bit by bit
        for (int i = 15; i >= 0; i--) {
          HAL_Delay(COMM_DELAY);
          if (HAL_GPIO_ReadPin(GPIOB, Receive_Pin) == GPIO_PIN_SET) {
            message |= (1 << i);
          }
        }

        // check if the message is valid
        if (!package_valid(message)) {
          lcd_command(CLEAR);
          lcd_putstring("Invalid message");
          HAL_Delay(2000);
          continue;
        }

        //check the transmission type and deal with it accordingly
        int message_type = (message >> 14) & 0x0001;
        if (message_type == SAMPLE_TRANSMIT) {
          samples_received++;
          // if the message is valid, unpack the message
          uint16_t unpacked_message = (message >> 2) & 0x0FFF;
          char* unpacked_string = malloc(16 * sizeof(char));
          sprintf(unpacked_string, "Sample: %u", unpacked_message); //Format voltage as string
          // writeLCD(unpacked_string);
          // write the data to the lcd
          lcd_command(CLEAR);
          lcd_putstring("Received Value:");
          lcd_command(LINE_TWO);
          lcd_putstring(unpacked_string);
          HAL_Delay(2000);
        }
        else {
          // if the message is valid, unpack the message
          uint16_t unpacked_count = (message >> 2) & 0x0FFF;
          
          // compare the received count with the sent count
          if (unpacked_count == samples_received) {
            lcd_command(CLEAR);
            lcd_putstring("Count Received");
            lcd_command(LINE_TWO);
            lcd_putstring("Success");
            HAL_Delay(2000);
          } else {
            lcd_command(CLEAR);
            lcd_putstring("Count Received");
            lcd_command(LINE_TWO);
            lcd_putstring("Fail. Updating.");
            samples_received = unpacked_count;
            //flashes LED 7 to show that the count is being updated
            for (int i = 0; i < 8; i++) {
              HAL_GPIO_TogglePin(GPIOB, LED7_Pin);
              HAL_Delay(COMM_DELAY/8);
            }
            // HAL_Delay(2000);
          }
        }
        // // put in line two LCD that the device is actively listening
        // lcd_command(CLEAR);
        // lcd_putstring("Receive M");
        // lcd_command(LINE_TWO);
        // lcd_putstring("Listening...");
      }
    }
  }
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI14_Enable();

   /* Wait till HSI14 is ready */
  while(LL_RCC_HSI14_IsReady() != 1)
  {

  }
  LL_RCC_HSI14_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_SetSystemCoreClock(8000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_HSI14_EnableADCControl();
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */
  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */
  ADC1->CR |= ADC_CR_ADCAL;
  while(ADC1->CR & ADC_CR_ADCAL);			// Calibrate the ADC
  ADC1->CR |= (1 << 0);						// Enable ADC
  while((ADC1->ISR & (1 << 0)) == 0);		// Wait for ADC ready
  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 47999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_ResetOutputPin(LED7_GPIO_Port, LED7_Pin);
  LL_GPIO_ResetOutputPin(LED6_GPIO_Port, LED6_Pin);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);

  /**/
  LL_GPIO_SetPinPull(Button0_GPIO_Port, Button0_Pin, LL_GPIO_PULL_UP);

  /**/
  LL_GPIO_SetPinMode(Button0_GPIO_Port, Button0_Pin, LL_GPIO_MODE_INPUT);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);


  /**/
  GPIO_InitStruct.Pin = Button1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(Button1_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Button2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(Button2_GPIO_Port, &GPIO_InitStruct);
  /**/
  GPIO_InitStruct.Pin = LED7_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LED7_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LED6_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; // 
  LL_GPIO_Init(LED6_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Transmit_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; //
  LL_GPIO_Init(Transmit_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = Receive_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; //
  LL_GPIO_Init(Receive_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void EXTI0_1_IRQHandler(void)
{


	// TODO: Add code to switch LED7 delay frequency
  if (HAL_GetTick() - ticks_before < 500) { // If button pressed within 0.5s
    HAL_GPIO_EXTI_IRQHandler(Button0_Pin);
    return;
  } else {
    ticks_before = HAL_GetTick(); // Update ticks_before
  }

	HAL_GPIO_EXTI_IRQHandler(Button0_Pin); // Clear interrupt flags
}

// TODO: Complete the writeLCD function
void writeLCD(char *char_in){
  delay(3000);
	lcd_command(CLEAR);
	lcd_putstring(char_in); //Write input string to the LCD
}

// Get ADC value
uint32_t pollADC(void){
  // TODO: Complete function body to get ADC val
  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, polling_speed); //Poll ADC every 500ms
  uint32_t value = HAL_ADC_GetValue(&hadc);
	HAL_ADC_Stop(&hadc); //Stop ADC after getting value
  return value;
}

// Calculate PWM CCR value
uint32_t ADCtoCCR(uint32_t adc_val){
  // TODO: Calculate CCR val using an appropriate equation
	uint32_t val = adc_val * (47999/4096);  // CCR = ADC * (ARR/levels)
	return val;
}

void ADC1_COMP_IRQHandler(void)
{
	adc_val = HAL_ADC_GetValue(&hadc); // Read ADC value during polling
	HAL_ADC_IRQHandler(&hadc); //Clear flags
}
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
