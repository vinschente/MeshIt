#include "nrf_esb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "boards.h"
//#define NRF_LOG_MODULE_NAME "APP"
//#include "nrf_log.h"
//#include "nrf_log_ctrl.h"


#include "app_uart.h"

#define LED_ON          0
#define LED_OFF         1


#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256   

//#define NRF_ESB_LEGACY

/*lint -save -esym(40, BUTTON_1) -esym(40, BUTTON_2) -esym(40, BUTTON_3) -esym(40, BUTTON_4) -esym(40, LED_1) -esym(40, LED_2) -esym(40, LED_3) -esym(40, LED_4) */

static nrf_esb_payload_t tx_payload;
static nrf_esb_payload_t rx_payload;
static uint8_t m_state[4];
uint8_t led_nr;

volatile nrf_esb_evt_id_t lastEsbEvent = 0xFF;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
	switch (p_event->evt_id)
	{
		case NRF_ESB_EVENT_TX_SUCCESS:
			printf("[ESB] TX OK\r\n");
			break;
		case NRF_ESB_EVENT_TX_FAILED:
			printf("[ESB] TX Failed\r\n");
			(void) nrf_esb_flush_tx();
			break;
		case NRF_ESB_EVENT_RX_RECEIVED:
			while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) ;
			printf("[ESB] Receiving packet length:%d rssi:%d\r\n", rx_payload.length, rx_payload.rssi);
			printf(":");
			for(int i=0; i<rx_payload.length; i++) {
				printf(" 0x%02X", rx_payload.data[i]);
			}
			printf("\r\n");
			(void) nrf_esb_write_payload(&tx_payload);
#if defined(BSP_LED_1)
			nrf_gpio_pin_write(BSP_LED_1, LED_ON);
#endif
			//NRF_LOG_DEBUG("Queue transmitt packet: %02x\r\n", tx_payload.data[0]);
			break;
	}
	lastEsbEvent = p_event->evt_id;
}


void clocks_start( void )
{
    // Start HFCLK and wait for it to start.
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}


uint32_t esb_init( void )
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

#ifndef NRF_ESB_LEGACY
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
#else // NRF_ESB_LEGACY
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_LEGACY_CONFIG;
#endif // NRF_ESB_LEGACY
    nrf_esb_config.selective_auto_ack       = 0;
    nrf_esb_config.payload_length           = 3;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;

    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    tx_payload.length = 1;

    return NRF_SUCCESS;
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


int main(void)
{
	uint32_t err_code;

	gpio_init();
	err_code = esb_init();
	APP_ERROR_CHECK(err_code);
	clocks_start();
	
	err_code = uart_init();
	APP_ERROR_CHECK(err_code);
	printf("Gateway running.\r\n");

	err_code = nrf_esb_start_rx();
	APP_ERROR_CHECK(err_code);

	tx_payload.data[0] = m_state[0] << 0
										 | m_state[1] << 1
										 | m_state[2] << 2
										 | m_state[3] << 3;
	err_code = nrf_esb_write_payload(&tx_payload);
	APP_ERROR_CHECK(err_code);

	while (true) {
		power_manage();
		
		if(lastEsbEvent == NRF_ESB_EVENT_RX_RECEIVED) {
			nrf_delay_ms(50);
#if defined(BSP_LED_1)
			nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
#endif
			lastEsbEvent = 0xFF;
		}
	}
}
/*lint -restore */
