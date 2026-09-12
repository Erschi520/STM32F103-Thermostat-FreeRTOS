/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ds18b20.h"
#include "ssd1306.h"
#include "tim.h"
#include <stdio.h>
#include <math.h>  // 在文件顶部添加
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
/* USER CODE BEGIN Variables */
/* ---- 放在 freertos.c 里, TempDisplayTask 函数外面的全局区域 ---- */
static TaskHandle_t s_tempTaskHandle = NULL;
volatile float g_setTemp  = 25.0f;   /* 设定温度, 初始25.0C */
volatile float g_lastTemp = 25.0f; /* 缓存上一次读到的真实温度 */


#define PWM_PULSE_MAX   999    /* 对应tim.c里 htim1.Init.Period = 999 */
#define SET_TEMP_STEP   0.5f
#define SET_TEMP_MIN    0.0f
#define SET_TEMP_MAX    80.0f


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void TempDisplay_Refresh(void)
{
    OLED_Clear();

    char buf[16];
    OLED_ShowString8x16(0, 2, "SET:");
    snprintf(buf, sizeof(buf), "%.1f", g_setTemp);
    OLED_ShowString8x16(40, 2, buf);

    if (g_lastTemp > -55.0f && g_lastTemp < 125.0f)
    {
        OLED_ShowString8x16(0, 5, "NOW:");
        snprintf(buf, sizeof(buf), "%.1f", g_lastTemp);
        OLED_ShowString8x16(40, 5, buf);
    }
    else
    {
        OLED_ShowString8x16(0, 4, "ERROR");
    }

    OLED_Refresh();
}

void TempDisplayTask(void *argument)
{
    (void)argument;
    OLED_Init();
    OLED_Clear();

    TickType_t lastCtrlTick = xTaskGetTickCount();

    for (;;)
    {
        //按键通知仅刷新屏幕
        if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)) > 0)
        {
            TempDisplay_Refresh();
        }

        TickType_t nowTick = xTaskGetTickCount();
        if((nowTick - lastCtrlTick) >= pdMS_TO_TICKS(250))
        {
            lastCtrlTick = nowTick;

            g_lastTemp = DS18B20_Get_Temp();

            if (g_lastTemp > -55.0f && g_lastTemp < 125.0f)
            {
                if(g_lastTemp > g_setTemp)
                {
                    //温度高于设定：开风扇，关闭加热
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, PWM_PULSE_MAX);

                }
                else if(g_lastTemp < g_setTemp)
                {
                    //温度低于设定：关闭风扇，开加热
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, PWM_PULSE_MAX);
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
                }
                else
                {
                    //温度相等，全部关闭
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
                }
            }
            else
            {
                //传感器故障，全部关闭
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
            }
            TempDisplay_Refresh();
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static TickType_t lastTickUp = 0, lastTickDown = 0;
    TickType_t now = xTaskGetTickCountFromISR();
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_2)               
    {
        if ((now - lastTickUp) > pdMS_TO_TICKS(100))  
        {
            lastTickUp = now;
            g_setTemp += SET_TEMP_STEP;
            if (g_setTemp > SET_TEMP_MAX) g_setTemp = SET_TEMP_MAX;
            vTaskNotifyGiveFromISR(s_tempTaskHandle, &xHigherPriorityTaskWoken);
        }
    }
    else if (GPIO_Pin == GPIO_PIN_3)           
    {
        if ((now - lastTickDown) > pdMS_TO_TICKS(100))
        {
            lastTickDown = now;
            g_setTemp -= SET_TEMP_STEP;
            if (g_setTemp < SET_TEMP_MIN) g_setTemp = SET_TEMP_MIN;
            vTaskNotifyGiveFromISR(s_tempTaskHandle, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	xTaskCreate(TempDisplayTask, "TempDisp", 512, NULL, tskIDLE_PRIORITY + 2, &s_tempTaskHandle);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

