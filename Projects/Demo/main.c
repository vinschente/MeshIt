/** @file
 * @defgroup rtc_example_main main.c
 * @{
 * @ingroup rtc_example
 * @brief Real Time Counter Example Application main file.
 *
 * This file contains the source code for a sample application using the Real Time Counter (RTC).
 *
 */

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_qdec.h"

#include "nrf_temp.h"

#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <stdint.h>
#include <stdbool.h>

#include "microbit_LedMatrix.h"
#include "microbit_uart.h"

#include "network.h"
#include "network_phy.h"
#include "Segger_Rtt.h"

enum {
	WAKEUP_BUTTON_0_PRESS,
	WAKEUP_BUTTON_0_RELEASE,
	WAKEUP_BUTTON_1_PRESS,
	WAKEUP_BUTTON_1_RELEASE,
	WAKEUP_NONE,
}wakeupReason = WAKEUP_NONE;

void power_manage( void )
{
	
    // WFE - SEV - WFE sequence to wait until a radio event require further actions.
    __WFE();
    __SEV();
    __WFE();
}


void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	switch(pin) {
		case BSP_BUTTON_0:
			if(bsp_board_button_state_get(0) == BUTTONS_ACTIVE_STATE) {
				wakeupReason = WAKEUP_BUTTON_0_PRESS;
			}else {
				wakeupReason = WAKEUP_BUTTON_0_RELEASE;
			}
			
			break;
		case BSP_BUTTON_1:
			if(bsp_board_button_state_get(1) == BUTTONS_ACTIVE_STATE) {
				wakeupReason = WAKEUP_BUTTON_1_PRESS;
			}else {
				wakeupReason = WAKEUP_BUTTON_1_RELEASE;
			}
			break;
		default:
			break;
	}
}

static void buttons_init()
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_0, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BSP_BUTTON_0, true);
	
    err_code = nrf_drv_gpiote_in_init(BSP_BUTTON_1, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BSP_BUTTON_1, true);
}


NetworkHandler_t hNet = {
	.Init.myAddr = 0x01,
//	.Init.hostAddr = 0
	.Init.DeviceType = DEVICE_GATEWAY,
	.Phy = &NetPhy,
};

/**
 * @brief Function for application main entry.
 */
int main(void)
{
//	SEGGER_RTT_Init();
//	SEGGER_RTT_WriteString(0, "Hello World\n");
	
	
	Matrix_Init();
	uBit_Uart_Init();
	
	buttons_init();
	
	Network_Init(&hNet);

//	lfclk_config();
//	qdec_init();

	while (true)
	{
		
		//power_manage();
		
		switch(wakeupReason) {
			case WAKEUP_BUTTON_0_PRESS:
				SEGGER_RTT_printf(0, "Wakeup Button 0 Press\n");
				Matrix_SetDot(0,0);
				wakeupReason = WAKEUP_NONE;
				break;
			case WAKEUP_BUTTON_0_RELEASE:
				SEGGER_RTT_printf(0, "Wakeup Button 0 Release\n");
				Matrix_ResetDot(0,0);
				wakeupReason = WAKEUP_NONE;
				break;
			case WAKEUP_BUTTON_1_PRESS:
				SEGGER_RTT_printf(0, "Wakeup Button 1 Press\n");
				Matrix_SetDot(0,4);
				wakeupReason = WAKEUP_NONE;
				break;
			case WAKEUP_BUTTON_1_RELEASE:
				SEGGER_RTT_printf(0, "Wakeup Button 1 Release\n");
				Matrix_ResetDot(0, 4);
				wakeupReason = WAKEUP_NONE;
				break;
			default:
				break;
		}
		
		

	}
}


/**  @} */
