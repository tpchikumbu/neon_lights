/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Button0_Pin LL_GPIO_PIN_0
#define Button0_GPIO_Port GPIOA

// button 1 for sending a sample or toggling listen mode
#define Button1_Pin LL_GPIO_PIN_1 
#define Button1_GPIO_Port GPIOA

// button 2 for sending a count
#define Button2_Pin LL_GPIO_PIN_2 
#define Button2_GPIO_Port GPIOA

#define LED7_Pin LL_GPIO_PIN_7
#define LED7_GPIO_Port GPIOB
#define LED6_Pin LL_GPIO_PIN_6
#define LED6_GPIO_Port GPIOB

// Set gpio pin 3 to be the output for the transmit signal
#define Transmit_Pin LL_GPIO_PIN_3
#define Transmit_GPIO_Port GPIOB

// Set gpio pin 4 to be the input for the receive signal
#define Receive_Pin LL_GPIO_PIN_4
#define Receive_GPIO_Port GPIOB


#define TRASMIT_MODE 0
#define RECEIVE_MODE 1

#define SAMPLE_TRANSMIT 0
#define COUNT_TRANSMIT 1

#define COMM_DELAY 500
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
