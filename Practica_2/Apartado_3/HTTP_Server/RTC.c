#include "RTC.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"


#include "lcd.h"
#include <time.h>
#include "rl_net_lib.h"


///* RTC handler declaration */

  RTC_HandleTypeDef RtcHandle;
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;
  RTC_AlarmTypeDef  alarmRtc;


uint8_t segundos_tim=0;
/************************************************
        hilo de alarma
***************************************************/
osThreadId_t tid_ThAlarm;                        // thread id
void ThAlarm (void *argument);                   // thread function

// timer
static uint32_t exec;
osTimerId_t tim_1s;
  
  
/***Configuracion********/

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  
  /*##-2- Configure LSE as RTC clock source ###################################*/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
  /*##-3- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

/**
  * @brief RTC MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_RTC_DISABLE();

  /*##-2- Disables the PWR Clock and Disables access to the backup domain ###################################*/
  HAL_PWR_DisableBkUpAccess();
  __HAL_RCC_PWR_CLK_DISABLE();
  
}

/***************************/


/* Private function prototypes -----------------------------------------------*/
void RTC_Init(){

  RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
  HAL_RTC_Init(&RtcHandle);
  

}


void RTC_CalendarConfig(void)
{
  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday March 01th 2023 */
  sdatestructure.Year = 0x23;
  sdatestructure.Month = RTC_MONTH_MARCH;
  sdatestructure.Date = 0x01;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  
  HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD);


  /*##-2- Configure the Time #################################################*/
  /* Set Time: 18:23:00 */
  stimestructure.Hours = 0x18;
  stimestructure.Minutes = 0x23;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD);
  
  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);

}

void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{

    /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BIN);
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN);

  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%2d:%2d:%2d", stimestructure.Hours, stimestructure.Minutes, stimestructure.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%2d-%2d-%2d", sdatestructure.Month, sdatestructure.Date, 2000 + sdatestructure.Year);
  
}


//Alarma
void RTC_AlarmConfig(void)
{
  alarmRtc.AlarmTime.Hours =  stimestructure.Hours;
  alarmRtc.AlarmTime.Minutes = stimestructure.Minutes;
  alarmRtc.AlarmTime.Seconds = 2;
  alarmRtc.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  alarmRtc.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  alarmRtc.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
  alarmRtc.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  alarmRtc.AlarmDateWeekDay = 0x1;
  alarmRtc.Alarm = RTC_ALARM_A;
  HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	
	// Enable the alarm interrupt
	HAL_RTC_SetAlarm_IT(&RtcHandle, &alarmRtc, RTC_FORMAT_BIN);
	
    // Unmask the RTC Alarm A interrupt
    CLEAR_BIT(RtcHandle.Instance->CR, RTC_CR_ALRAIE);
}






static void Timer_Callback_1s (void const *arg) {
  
  segundos_tim++;
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
	if (segundos_tim==6){
		segundos_tim=0;
		osTimerStop(tim_1s);
	}
}

int Init_timers (void) {
	exec = 1U;
	tim_1s = osTimerNew((osTimerFunc_t)&Timer_Callback_1s, osTimerPeriodic, &exec, NULL);
}

	int Init_ThAlarm (void) {
	 
		tid_ThAlarm = osThreadNew(ThAlarm, NULL, NULL);
		if (tid_ThAlarm == NULL) {
			return(-1);
		}
		return(0);
	}

 // Thread Alarma
void ThAlarm (void *argument) {
  while (1) {
    ; // Insert thread code here...
		osThreadFlagsWait(0x02, osFlagsWaitAny, osWaitForever);
		osTimerStart(tim_1s, 1000U);			
	  osThreadYield(); // suspend thread  
  }

}
