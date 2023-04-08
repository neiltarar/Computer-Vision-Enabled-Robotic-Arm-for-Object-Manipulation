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
#define RX_BUFFER_SIZE 48
#define TX_BUFFER_SIZE 48
#define NUM_JOINTS 5
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
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
typedef enum {
	BASE1, // PIN: PA15
	ARM2,  // PIN: PB3
	ARM3,  // PIN: PB4
	ARM4,  // PIN: PB10
	CLAW7  // PIN: PB11
} CCR_Register;

typedef struct {
	CCR_Register ccr_register;
	uint32_t targetCCR;
	uint32_t currentCCR;
} JointMove;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_UART4_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
static void moveRobotArmJoint(uint32_t angle[], CCR_Register ccr_register[], int num_joints) {
	static bool first_run = true; // Add this line to track if it's the first run
    JointMove joint_moves[num_joints];
    bool joints_moving[num_joints];

    if (first_run) { // Add this condition to check if it's the first run
                uint32_t generalInitialCCR = (uint32_t)((90 / 180.0) * 2000 + 2000);
                uint32_t arm2InitialCCR = (uint32_t)((65 / 180.0) * 2000 + 2000);
                uint32_t arm3InitialCCR = (uint32_t)((180 / 180.0) * 2000 + 2000);
                uint32_t clawInitialCCR = (uint32_t)((150 / 180.0) * 2000 + 2000);
                // Set all PWM registers (CCR1 to CCR4) to 90 degrees
                htim2.Instance->CCR1 = generalInitialCCR;
                htim2.Instance->CCR2 = arm2InitialCCR;
                htim2.Instance->CCR3 = generalInitialCCR;
                htim2.Instance->CCR4 = clawInitialCCR;
                htim3.Instance->CCR1 = arm3InitialCCR;
                first_run = false;
            }
    for (int i = 0; i < num_joints; i++) {
        joint_moves[i].ccr_register = ccr_register[i];
        joint_moves[i].targetCCR = (uint32_t)((angle[i] / 180.0) * 2000 + 2000);
        joints_moving[i] = true;

		switch (ccr_register[i]) {
			case BASE1:
				joint_moves[i].currentCCR = htim2.Instance->CCR1;
				break;
			case ARM2:
				joint_moves[i].currentCCR = htim2.Instance->CCR2;
				break;
			case ARM3:
				joint_moves[i].currentCCR = htim3.Instance->CCR1;
				break;
			case ARM4:
				joint_moves[i].currentCCR = htim2.Instance->CCR3;
				break;
			case CLAW7:
				joint_moves[i].currentCCR = htim2.Instance->CCR4;
				break;
			default:
				// handle error case
				return;
		}

    }

    bool all_joints_reached_target = false;

    while (!all_joints_reached_target) {
        all_joints_reached_target = true;
        for (int i = 0; i < num_joints; i++) {
            if (joints_moving[i]) {
                if (joint_moves[i].currentCCR < joint_moves[i].targetCCR) {
                    joint_moves[i].currentCCR++;
                } else if (joint_moves[i].currentCCR > joint_moves[i].targetCCR) {
                    joint_moves[i].currentCCR--;
                } else {
                    joints_moving[i] = false;
                    continue;
                }

                switch (joint_moves[i].ccr_register) {
                    case BASE1:
                        htim2.Instance->CCR1 = joint_moves[i].currentCCR;
                        break;
                    case ARM2:
                        htim2.Instance->CCR2 = joint_moves[i].currentCCR;
                        break;
                    case ARM3:
						htim3.Instance->CCR1 = joint_moves[i].currentCCR;
						break;
                    case ARM4:
                        htim2.Instance->CCR3 = joint_moves[i].currentCCR;
                        break;
                    case CLAW7:
						htim2.Instance->CCR4 = joint_moves[i].currentCCR;
						break;
                    default:
                        // handle error case
                        return;
                }

                all_joints_reached_target = false;
            }
        }
        HAL_Delay(0.3); // Adjust the delay value to control the speed
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
    // BASE 1 - PIN: PA15
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  	// ARM 2 - PIN: PB3
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	// ARM 4 - PIN: PB10
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	// CLAW 7 - PIN: PB11
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	// ARM 3 - PIN: PB4
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
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

		uint32_t angles[NUM_JOINTS];
		CCR_Register ccr_registers[NUM_JOINTS];
		int joint_count = 0;

		while (joint_split_str != NULL) {

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
		    } else if (strcmp(JOINT, "ARM3") == 0) {
		        ccr_register = ARM3;
		    } else if (strcmp(JOINT, "ARM4") == 0) {
		        ccr_register = ARM4;
		    } else if (strcmp(JOINT, "CLAW7") == 0) {
		        ccr_register = CLAW7;
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

		    if (movement_angle_int >= 0 && movement_angle_int <= 180) { // Check if the value is within valid range
		            angles[joint_count] = movement_angle_int;
		            ccr_registers[joint_count] = ccr_register;
		            joint_count++;
		    }
		    joint_split_str = strtok(NULL, ",");
		}

	if (joint_count > 0) {
		moveRobotArmJoint(angles, ccr_registers, joint_count);
	}
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
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  htim3.Init.Prescaler = 41;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 39999;
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
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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
