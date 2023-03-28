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
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 32
#define NUM_JOINTS 3
#define JOINT_NAME_SIZE 5
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart4;
uint32_t targetCCRs[NUM_JOINTS];
uint32_t currentCCRs[NUM_JOINTS];
/* USER CODE BEGIN PV */
typedef enum {
  BASE1, // PIN: PA15
  ARM2,  // PIN: PB3
  ARM4   // PIN: PB10
} CCR_Register;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */
static void moveRobotArmJoint(uint32_t angles[NUM_JOINTS], CCR_Register ccr_registers[NUM_JOINTS]) {
    uint32_t targetCCRs[NUM_JOINTS];
    uint32_t currentCCRs[NUM_JOINTS];

    for (int i = 0; i < NUM_JOINTS; i++) {
        targetCCRs[i] = (uint32_t)((angles[i] / 180.0) * 2000 + 2000);
        switch (ccr_registers[i]) {
            case BASE1:
                currentCCRs[i] = htim2.Instance->CCR1;
                break;
            case ARM2:
                currentCCRs[i] = htim2.Instance->CCR2;
                break;
            case ARM4:
                currentCCRs[i] = htim2.Instance->CCR3;
                break;
            default:
                // handle error case
                return;
        }
    }

    uint32_t startTime = HAL_GetTick();
    uint32_t elapsedTime = 0;
    while (1) {
        uint8_t jointsCompleted = 0;
        elapsedTime = HAL_GetTick() - startTime;

        for (int i = 0; i < NUM_JOINTS; i++) {
            if (currentCCRs[i] == targetCCRs[i]) {
                jointsCompleted++;
                continue;
            }

            if (elapsedTime >= 0.3) { // Adjust the delay value to control the speed
                startTime = HAL_GetTick();
                if (currentCCRs[i] < targetCCRs[i]) {
                    currentCCRs[i]++;
                } else {
                    currentCCRs[i]--;
                }

                switch (ccr_registers[i]) {
                    case BASE1:
                        htim2.Instance->CCR1 = currentCCRs[i];
                        break;
                    case ARM2:
                        htim2.Instance->CCR2 = currentCCRs[i];
                        break;
                    case ARM4:
                        htim2.Instance->CCR3 = currentCCRs[i];
                        break;
                    default:
                        // handle error case
                        return;
                }
            }
        }

        if (jointsCompleted == NUM_JOINTS) {
            break;
        }
    }
}

static void send_echo(const char* message) {
    HAL_UART_Transmit(&huart4, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char rx_buffer[RX_BUFFER_SIZE];
	char tx_data[TX_BUFFER_SIZE];
	uint8_t rx_data;
	int rx_index = 0;
	bool start_detected = false;

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
  MX_TIM2_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
    // BASE 1 - PIN: PA15
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  	// ARM 2 - PIN: PB3
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	// ARM 4 - PIN: PB10
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	 printf("Waiting for Rx data...\n");
		// waiting for a # as the start character
	start_detected = false;
	while (!start_detected)
	{
		HAL_UART_Receive(&huart4, &rx_data, 1, HAL_MAX_DELAY);
		if (rx_data == '#')
		{
			start_detected = true;
			rx_index = 0;
		}
	}

	while(rx_data != '\n' && rx_index < RX_BUFFER_SIZE - 1 && start_detected == true) {
		// Receive next byte
		HAL_UART_Receive(&huart4, &rx_data, 1, HAL_MAX_DELAY);
		// Add byte to the buffer
		rx_buffer[rx_index] = rx_data;
		rx_index++;
	}

		char *joint_split_str = strtok(rx_buffer, ",");
		char my_string[5];
		char JOINT[5];
		int movement_angle_int;
		CCR_Register ccr_registers[NUM_JOINTS];
		uint32_t angles[NUM_JOINTS];
		int joint_count = 0;

		while (joint_split_str != NULL && joint_count < NUM_JOINTS) {

		    sscanf(joint_split_str, "%[^-]-%d", my_string, &movement_angle_int);

		    for(int i=0; i<sizeof(my_string);i++){
		        JOINT[i] = my_string[i];
		    }
		    JOINT[sizeof(my_string)] = '\0';

		    // Determine the CCR register based on joint name
		    CCR_Register ccr_register;
		    if (strcmp(JOINT, "BASE1") == 0) {
		        ccr_register = BASE1;
		    } else if (strcmp(JOINT, "ARM2") == 0) {
		        ccr_register = ARM2;
		    } else if (strcmp(JOINT, "ARM4") == 0) {
		        ccr_register = ARM4;
		    } else {
		        // Handle error case
		        continue; // Set to a default value
		    }

		    sprintf(tx_data, "Value: %d\n", movement_angle_int);
		    tx_data[strlen(tx_data)] = '\0';
		    send_echo(tx_data);
		    memset(tx_data, 0, sizeof(tx_data));
		    memset(JOINT, 0, sizeof(JOINT));
		    memset(my_string, 0, sizeof(my_string));
		    if (movement_angle_int >= 0 && movement_angle_int <= 180) {
		            ccr_registers[joint_count] = ccr_register;
		            angles[joint_count] = movement_angle_int;
		            joint_count++;
		        }
			joint_split_str = strtok(NULL, ",");
		    ccr_register = 0;

		}
	// Call the modified moveRobotArmJoint function with arrays of joint names and values
	moveRobotArmJoint(angles, ccr_registers);
	rx_index = 0;
	rx_buffer[rx_index] = '\0';
	memset(rx_buffer, 0, sizeof(rx_buffer));
	start_detected = false;
	// Split the received data into two variables at '-'

//	  resetRxBuffer(rx_buffer, rx_index);
  }

  /* USER CODE END 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 41;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 39999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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
