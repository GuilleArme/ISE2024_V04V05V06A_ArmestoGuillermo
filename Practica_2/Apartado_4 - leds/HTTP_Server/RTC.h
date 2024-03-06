#include "main.h"

#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */


void RTC_Init(void);
void RTC_CalendarConfig(void);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void RTC_AlarmConfig(void);
int Init_timers (void);
void SNTP_init(void);

static void time_callback (uint32_t seconds, uint32_t seconds_fraction);



int Init_ThAlarm (void);
