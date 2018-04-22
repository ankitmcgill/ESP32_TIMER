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

#include "ESP32_TIMER.h"

//INTERNAL FUNCTIONS
static void s_timer_group_0_timer_0_isr(void* arg);
static void s_timer_group_0_timer_1_isr(void* arg);
static void s_timer_group_1_timer_0_isr(void* arg);
static void s_timer_group_1_timer_1_isr(void* arg);

//INTERNAL VARIABLES
static bool s_debug;

static void (*s_timer_group_0_timer_0_cb)(void*) = NULL;
static void (*s_timer_group_0_timer_1_cb)(void*) = NULL;
static void (*s_timer_group_1_timer_0_cb)(void*) = NULL;
static void (*s_timer_group_1_timer_1_cb)(void*) = NULL;

void ESP32_TIMER_SetDebug(bool enable)
{
	//SET MODULE DEBUG

	s_debug = enable;
}

esp_err_t ESP32_TIMER_Initialize(esp32_timer_group_type_t group,
                                    esp32_timer_timer_type_t timer_num,
                                    bool enable_interrupt,
                                    esp32_timer_direction_type_t direction,
                                    bool enable_auto_reload,
                                    uint32_t divider)
{
	//SET UP TIMER WITH THE SPECIFIED PARAMETERS

	timer_config_t config;
	esp_err_t err;

	config.alarm_en = enable_interrupt;
	config.counter_en = false;
	config.intr_type = TIMER_INTR_LEVEL;
	config.counter_dir = direction;
	config.auto_reload = enable_auto_reload;
	config.divider = divider;

	//SET INITIAL COUNTER VALUE TO 0
	//AUTORELOAD WILL RELOAD THIS VALUE AT EVERY ALARM
	err = timer_set_counter_value(group, timer_num, 0);

	err = timer_init(group, timer_num, &config);

	if(err != ESP_OK)
	{
		return ESP_FAIL;
	}

	if(s_debug)
	{
		ESP_LOGI(ESP32_TIMER_TAG, "Timer group = %u id = %u Initialize", group, timer_num);
	}
	return ESP_OK;
}

esp_err_t ESP32_TIMER_SetInterruptCb(esp32_timer_group_type_t group,
                                    esp32_timer_timer_type_t timer_num,
                                    uint64_t alarm_value,
									void (*cb)(void*))
{
	//SET TIMER INTERRUPT CB AS SPECIFIED

	esp_err_t err;

	err =timer_set_alarm_value(group, timer_num, alarm_value);
	
	//ENABLE TIMER ALARM EVENTS
	err = timer_set_alarm(group, timer_num, TIMER_ALARM_EN);

	//REGISTER ISR
	//SET CB FUNCTION VARIABLE
	if(group == TIMER_GROUP0)
	{
		if(timer_num == TIMER0)
		{
			err = timer_isr_register(group, 
										timer_num,
										s_timer_group_0_timer_0_isr,
										NULL,
										ESP_INTR_FLAG_LOWMED,
										NULL);
			s_timer_group_0_timer_0_cb = cb;
		}
		else if(timer_num == TIMER1)
		{
			err = timer_isr_register(group, 
										timer_num,
										s_timer_group_0_timer_1_isr,
										NULL,
										ESP_INTR_FLAG_LOWMED,
										NULL);
			s_timer_group_0_timer_1_cb = cb;
		}
	}
	else if(group == TIMER_GROUP1)
	{
		if(timer_num == TIMER0)
		{
			err = timer_isr_register(group, 
										timer_num,
										s_timer_group_1_timer_0_isr,
										NULL,
										ESP_INTR_FLAG_LOWMED,
										NULL);
			s_timer_group_1_timer_0_cb = cb;
		}
		else if(timer_num == TIMER1)
		{
			err = timer_isr_register(group, 
										timer_num,
										s_timer_group_1_timer_1_isr,
										NULL,
										ESP_INTR_FLAG_LOWMED,
										NULL);
			s_timer_group_1_timer_1_cb = cb;
		}
	}

	//ENABLE TIMER INTERRUPT
	err = timer_enable_intr(group, timer_num);

	if(err != ESP_OK)
	{
		return ESP_FAIL;
	}

	if(s_debug)
	{
		ESP_LOGI(ESP32_TIMER_TAG, "Timer group = %u id = %u ISR registered", group, timer_num);
	}
	return ESP_OK;
}

esp_err_t ESP32_TIMER_Start(esp32_timer_group_type_t group, esp32_timer_timer_type_t timer_num)
{
	//START SPECIFIED TIMER

	esp_err_t err;

	//SET TIMER COUNTER VALUE TO 0
	err = timer_set_counter_value(group, timer_num, 0);

	//START TIMER
	err = timer_start(group, timer_num);

	if(err != ESP_OK)
	{
		return ESP_FAIL;
	}

	if(s_debug)
	{
		ESP_LOGI(ESP32_TIMER_TAG, "Timer group = %u id = %u Started", group, timer_num);
	}
	return ESP_OK;
}

esp_err_t ESP32_TIMER_Stop(esp32_timer_group_type_t group, esp32_timer_timer_type_t timer_num)
{
	//STOP SPECIFIED TIMER

	esp_err_t err;

	err = timer_pause(group, timer_num);

	if(err != ESP_OK)
	{
		return ESP_FAIL;
	}

	if(s_debug)
	{
		ESP_LOGI(ESP32_TIMER_TAG, "Timer group = %u id = %u Stopped", group, timer_num);
	}
	return ESP_OK;
}

esp_err_t ESP32_TIMER_GetCounterValue(esp32_timer_group_type_t group, 
                                    	esp32_timer_timer_type_t timer_num,
                                    	uint64_t* retval)
{
	//GET SPECIFIED TIMER COUNTER VALUE

	return timer_get_counter_value(group, timer_num, retval);
}

//ISR
static void s_timer_group_0_timer_0_isr(void* arg)
{
	//GROUP 0, TIMER 0 ISR

	//CLEAR INTERRUPT
	TIMERG0.int_clr_timers.t0 = 1;

	//RESET ALARM
	//ESP32 ALARMS ARE ONE SHOT
	//NEED TO BE MANUALLY SET EVERY TIME
	timer_set_alarm(TIMER_GROUP0, TIMER0, TIMER_ALARM_EN);

	//CALL USER CB FUNCTION IF NOT NULL
	if(s_timer_group_0_timer_0_cb != NULL)
	{
		(*s_timer_group_0_timer_0_cb)(NULL);
	}
}

static void s_timer_group_0_timer_1_isr(void* arg)
{
	//GROUP 0, TIMER 1 ISR

	//CLEAR INTERRUPT
	TIMERG0.int_clr_timers.t1 = 1;

	//CALL USER CB FUNCTION IF NOT NULL
	if(s_timer_group_0_timer_1_cb != NULL)
	{
		(*s_timer_group_0_timer_1_cb)(NULL);
	}
}

static void s_timer_group_1_timer_0_isr(void* arg)
{
	//GROUP 1, TIMER 0 ISR

	//CLEAR INTERRUPT
	TIMERG1.int_clr_timers.t0 = 1;

	//CALL USER CB FUNCTION IF NOT NULL
	if(s_timer_group_1_timer_0_cb != NULL)
	{
		(*s_timer_group_1_timer_0_cb)(NULL);
	}
}

static void s_timer_group_1_timer_1_isr(void* arg)
{
	//GROUP 1, TIMER 1 ISR

	//CLEAR INTERRUPT
	TIMERG1.int_clr_timers.t1 = 1;

	//CALL USER CB FUNCTION IF NOT NULL
	if(s_timer_group_1_timer_1_cb != NULL)
	{
		(*s_timer_group_1_timer_1_cb)(NULL);
	}
}