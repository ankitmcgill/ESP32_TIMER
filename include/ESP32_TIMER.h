//////////////////////////////////////////////////////////////////
// GPIO DRIVER - TIMER
//
// LIBRARY FOR TIMER OPERATIONS ON ESP32
//
// NOTE : FROM ESP-IDF MANUAL
//        http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/timer.html
//
//        ESP32 PROVIDES 2 TIMER GROUPS WITH 2 TIMER IN EACH GROUP
//        ALL TIMER ARE 64 BIT WITH 16 BIT PRESCALER (TO SCALE 80MHZ SYSTEM CLOCK)
//        ALL TIMER ARE UP / DOWN
//
// APRIL 22, 2018
//
// ANKIT BHATNAGAR
// ANKIT.BHATNAGARINDIA@GMAIL.COM
//////////////////////////////////////////////////////////////////

#ifndef _ESP32_TIMER_
#define _ESP32_TIMER_

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/timer.h"

#define ESP32_TIMER_TAG                     "ESP32:TIMER"

//UTIL TO CONVERT MS PERIOD TO REQUIRED COUNT
//x = ms PERIOD
//x = DIVIDER USED WHILE INITIALIZING THE TIME
#define ESP32_TIMER_MS_TO_CNT_VALUE(x,y)    ((TIMER_BASE_CLK/(y * 1000)) * x)

typedef enum
{
    TIMER_GROUP0 = 0,
    TIMER_GROUP1
}esp32_timer_group_type_t;

typedef enum
{
    TIMER0 = 0,
    TIMER1
}esp32_timer_timer_type_t;

typedef enum
{
    TIMER_DIRECTION_DOWN = 0,
    TIMER_DIRECTION_UP
}esp32_timer_direction_type_t;

void ESP32_TIMER_SetDebug(bool enable);

esp_err_t ESP32_TIMER_Initialize(esp32_timer_group_type_t group,
                                    esp32_timer_timer_type_t timer_num,
                                    bool enable_interrupt,
                                    esp32_timer_direction_type_t direction,
                                    bool enable_auto_reload,
                                    uint32_t divider);

esp_err_t ESP32_TIMER_SetInterruptCb(esp32_timer_group_type_t group,
                                    esp32_timer_timer_type_t timer_num,
                                    uint64_t alarm_value,
									void (*cb)(void*));

esp_err_t ESP32_TIMER_Start(esp32_timer_group_type_t group, 
                            esp32_timer_timer_type_t timer_num);
esp_err_t ESP32_TIMER_Stop(esp32_timer_group_type_t group, 
                            esp32_timer_timer_type_t timer_num);

esp_err_t ESP32_TIMER_GetCounterValue(esp32_timer_group_type_t group, 
                                        esp32_timer_timer_type_t timer_num,
                                        uint64_t* retval);

#endif