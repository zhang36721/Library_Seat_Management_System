/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kt_debug.h"
#include "kt_config.h"
#include "kt_port_uart.h"
#include "kt_port_gpio.h"
#include "kt_task/kt_task.h"
#include "kt_app/app_io.h"
#include "kt_app/main_keys.h"
#include "kt_app/main_controller_app.h"
#include "kt_system/kt_boot_count.h"
#include "kt_system/kt_system_health.h"
#include "kt_system/kt_watchdog.h"
#include "kt_modules/kt_modules.h"
#include "kt_modules/kt_uart_links.h"
#include "kt_modules/kt_esp32_link.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
#if APP_HEARTBEAT_LED_ENABLE
static void app_heartbeat_task(void);
#endif
#if KT_ENABLE_UPTIME_LOG
static void app_status_task(void);
#endif
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief  Heartbeat task — toggle PC13 LED every 500ms
 */
#if APP_HEARTBEAT_LED_ENABLE
static void app_heartbeat_task(void)
{
    kt_led_toggle(&app_led);
}
#endif

/**
 * @brief  Status task — print uptime every 3000ms
 */
#if KT_ENABLE_UPTIME_LOG
static void app_status_task(void)
{
    static uint32_t last_print_ms = 0;
    char buf[64];
    uint32_t uptime_s = HAL_GetTick() / 1000;

    kt_port_uart_tx_string("[LOG] Uptime: ");
    snprintf(buf, sizeof(buf), "%lu s\r\n", (unsigned long)uptime_s);
    kt_port_uart_tx_string(buf);
}
#endif

/* USER CODE END 0 */

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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  kt_system_health_init();
  kt_boot_count_init();
  kt_debug_init();
  kt_debug_print_system_info();
  kt_system_health_print_reset_reason();
  kt_watchdog_init();
  kt_port_uart_start_receive_it();
  kt_modules_init();

  /* v1.0: App I/O initialization (LED, Button, Buzzer - multi-instance) */
  app_io_init();
  main_keys_init();
  main_controller_app_init();
  kt_led_off(&app_led);  /* Ensure LED starts off */

  /* v1.0: Task scheduler - includes device & app tasks */
  kt_task_init();
  kt_task_register("debug",     kt_debug_task,       1);
  kt_task_register("app_io",    app_io_tasks,       10);   /* LED blink + button debounce + buzzer */
  kt_task_register("main_keys", main_keys_task,     10);
  kt_task_register("main_app",  main_controller_app_task, MAIN_APP_TASK_PERIOD_MS);
#if APP_HEARTBEAT_LED_ENABLE
  kt_task_register("heartbeat", app_heartbeat_task, 500);
#endif
#if KT_ENABLE_UPTIME_LOG
  kt_task_register("status",    app_status_task,    3000);
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    kt_task_run();
    kt_modules_task();
    kt_system_health_note_main_loop();
    kt_watchdog_task();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  UART RX complete callback
  * @param  huart  UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    kt_zigbee_uart_rx_callback(huart);
    kt_esp32_link_uart_rx_callback(huart);
    kt_port_uart_rx_callback(huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    kt_esp32_link_uart_tx_callback(huart);
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
  uint32_t i;
  kt_system_health_note_fault("ERROR_HANDLER");
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
  for (i = 0; i < 6000000U; i++) {
    __NOP();
  }
  NVIC_SystemReset();
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
