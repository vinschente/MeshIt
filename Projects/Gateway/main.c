//#include "nrf_esb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
//#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "boards.h"
//#define NRF_LOG_MODULE_NAME "APP"
//#include "nrf_log.h"
//#include "nrf_log_ctrl.h"

#include "network.h"

#include "app_uart.h"

#define LED_ON          0
#define LED_OFF         1


#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256   

//#define NRF_ESB_LEGACY

/*lint -save -esym(40, BUTTON_1) -esym(40, BUTTON_2) -esym(40, BUTTON_3) -esym(40, BUTTON_4) -esym(40, LED_1) -esym(40, LED_2) -esym(40, LED_3) -esym(40, LED_4) */

uint8_t led_nr;




void clocks_start( void )
{
    // Start HFCLK and wait for it to start.
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}

void gpio_init( void )
{
#if LEDS_NUMBER > 0
    //nrf_gpio_range_cfg_output(8, 31);
    bsp_board_leds_init();
    nrf_gpio_pin_write(BSP_LED_0, LED_OFF);

	
    nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
    nrf_gpio_pin_write(BSP_LED_2, LED_OFF);
#endif
}


void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

uint32_t uart_init( void )
{
	uint32_t err_code;
	
	const app_uart_comm_params_t comm_params = {
		RX_PIN_NUMBER,
		TX_PIN_NUMBER,
		0,
		0,
		APP_UART_FLOW_CONTROL_DISABLED,
		false,
		UART_BAUDRATE_BAUDRATE_Baud115200
	};

   APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);
	return err_code;
}


void power_manage( void )
{
    // WFE - SEV - WFE sequence to wait until a radio event require further actions.
    __WFE();
    __SEV();
    __WFE();
}

uint8_t txBuffer[MESSAGE_SIZE];

NetworkHandler_t hNetwork = {
	.Init.DeviceType = DEVICE_GATEWAY,
	.Init.TxBufferPtr = txBuffer,
};

int main(void)
{
	uint32_t err_code;

	gpio_init();
	
	Network_Init(&hNetwork);
	
	clocks_start();
	
	err_code = uart_init();
	APP_ERROR_CHECK(err_code);
	printf("Gateway running.\r\n");

	for(int i=0;i< 2;i++) {
		nrf_gpio_pin_write(BSP_LED_2, LED_ON);
		nrf_delay_ms(125);
		nrf_gpio_pin_write(BSP_LED_2, LED_OFF);
		nrf_delay_ms(125);
	}


	while (true) {
		static int tick = 0;
		Network_Manage(&hNetwork);
		
		if(tick%100 == 0) {
			Network_SendBuffer(&hNetwork, 0xB00B, NULL, 0);
		}
		tick++;
		
		nrf_delay_ms(10);
		
		//power_manage();		
	}
}
/*lint -restore */
