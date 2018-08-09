#include "nrf_esb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sdk_common.h"
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_util.h"

#include "app_uart.h"

/* UART configuration */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */


#define RESET_MEMORY_TEST_BYTE  (0x0DUL)        /**< Known sequence written to a special register to check if this wake up is from System OFF. */
#define RAM_RETENTION_OFF       (0x00000003UL)  /**< The flag used to turn off RAM retention on nRF52. */

#define BTN_PRESSED     0                       /**< Value of a pressed button. */
#define BTN_RELEASED    1                       /**< Value of a released button. */

//#define NRF_ESB_LEGACY

/*lint -save -esym(40, BUTTON_1) -esym(40, BUTTON_2) -esym(40, BUTTON_3) -esym(40, BUTTON_4) -esym(40, LED_1) -esym(40, LED_2) -esym(40, LED_3) -esym(40, LED_4) */

static nrf_esb_payload_t tx_payload = NRF_ESB_CREATE_PAYLOAD(0, 0x01, 0x00);
static nrf_esb_payload_t rx_payload;
static uint32_t button_state_1;
static uint32_t button_state_2;
static uint32_t button_state_3;
static uint32_t button_state_4;
static volatile bool esb_completed = false;

void system_off( void )
{
#ifdef NRF51
    NRF_POWER->RAMON |= (POWER_RAMON_OFFRAM0_RAM0Off << POWER_RAMON_OFFRAM0_Pos) |
                        (POWER_RAMON_OFFRAM1_RAM1Off << POWER_RAMON_OFFRAM1_Pos);
#endif //NRF51
#ifdef NRF52
    NRF_POWER->RAM[0].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[1].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[2].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[3].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[4].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[5].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[6].POWER = RAM_RETENTION_OFF;
    NRF_POWER->RAM[7].POWER = RAM_RETENTION_OFF;
#endif //NRF52

    // Turn off LEDs before sleeping to conserve energy.
//    bsp_board_leds_off();

    // Set nRF5 into System OFF. Reading out value and looping after setting the register
    // to guarantee System OFF in nRF52.
    NRF_POWER->SYSTEMOFF = 0x1;
    (void) NRF_POWER->SYSTEMOFF;
    while (true);
}

volatile nrf_esb_evt_id_t lastEsbEvent = 0xFF;


void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
	switch (p_event->evt_id) {
		case NRF_ESB_EVENT_TX_SUCCESS:
			printf("[ESB] TX OK\r\n");
			break;
		case NRF_ESB_EVENT_TX_FAILED:
			printf("[ESB] TX FAIL\r\n");
			(void) nrf_esb_flush_tx();
			break;
		case NRF_ESB_EVENT_RX_RECEIVED:
			printf("[ESB] RX Received\r\n");
			// Get the most recent element from the RX FIFO.
			while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) ;

			// For each LED, set it as indicated in the rx_payload, but invert it for the button
			// which is pressed. This is because the ack payload from the PRX is reflecting the
			// state from before receiving the packet.
			//            nrf_gpio_pin_write(LED_1, !( ((rx_payload.data[0] & 0x01) == 0) ^ (button_state_1 == BTN_PRESSED)) );
			//            nrf_gpio_pin_write(LED_2, !( ((rx_payload.data[0] & 0x02) == 0) ^ (button_state_2 == BTN_PRESSED)) );
			//            nrf_gpio_pin_write(LED_3, !( ((rx_payload.data[0] & 0x04) == 0) ^ (button_state_3 == BTN_PRESSED)) );
			//            nrf_gpio_pin_write(LED_4, !( ((rx_payload.data[0] & 0x08) == 0) ^ (button_state_4 == BTN_PRESSED)) );
		break;
	}
	
	lastEsbEvent = p_event->evt_id;
	esb_completed = true;
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
    nrf_esb_config.retransmit_count         = 6;
    nrf_esb_config.selective_auto_ack       = false;
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;

    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    tx_payload.length  = 3;
    tx_payload.pipe    = 0;
    tx_payload.data[0] = 0x00;

    return NRF_SUCCESS;
}


uint32_t gpio_check_and_esb_tx()
{
	uint32_t err_code;
	button_state_1 = nrf_gpio_pin_read(BUTTON_1);
	button_state_2 = nrf_gpio_pin_read(BUTTON_2);

	tx_payload.data[0] = 0;
		
	if (button_state_1 == BTN_PRESSED) {
		tx_payload.data[0] |= 0x01;
	}
	if (button_state_2 == BTN_PRESSED) {
		 tx_payload.data[0] |= 0x02;
	}

	if (button_state_1 || button_state_2) {
		tx_payload.noack = false;
		err_code = nrf_esb_write_payload(&tx_payload);
		VERIFY_SUCCESS(err_code);
	} else {
		esb_completed = true;
	}
	
	return NRF_SUCCESS;
}


void gpio_init( void )
{
#if LEDS_NUMBER > 0
    //nrf_gpio_range_cfg_output(8, 31);
    bsp_board_leds_init();
    nrf_gpio_pin_write(BSP_LED_0, LED_OFF);
    nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
#endif
	
	nrf_gpio_cfg_sense_input(BUTTON_1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
	nrf_gpio_cfg_sense_input(BUTTON_2, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

	// Workaround for PAN_028 rev1.1 anomaly 22 - System: Issues with disable System OFF mechanism
	nrf_delay_ms(1);

}


void recover_state()
{
    uint32_t            loop_count = 0;
    if ((NRF_POWER->GPREGRET >> 4) == RESET_MEMORY_TEST_BYTE)
    {
        // Take the loop_count value.
        loop_count          = (uint8_t)(NRF_POWER->GPREGRET & 0xFUL);
        NRF_POWER->GPREGRET = 0;
    }

    loop_count++;
    NRF_POWER->GPREGRET = ( (RESET_MEMORY_TEST_BYTE << 4) | loop_count);

    tx_payload.data[1] = loop_count << 4;
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

void uart_init(void)
{
	uint32_t err_code;
	
	const app_uart_comm_params_t comm_params =
	{
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
}

int main(void)
{
	uint32_t err_code;
	// Initialize
	clocks_start();
	err_code = esb_init();
	APP_ERROR_CHECK(err_code);

	gpio_init();
	
	uart_init();
	printf("Hello World\r\n");

	// Recover state if the device was woken from System OFF.
	recover_state();

	// Check state of all buttons and send an esb packet with the button press if there is exactly one.
	err_code = gpio_check_and_esb_tx();
	APP_ERROR_CHECK(err_code);

	while (true) {

		// Wait for esb completed and all buttons released before going to system off.
		if (esb_completed) {
			#if LEDS_NUMBER > 0
			if(lastEsbEvent == NRF_ESB_EVENT_TX_SUCCESS) {
				nrf_gpio_pin_write(BSP_LED_0, LED_ON);
				nrf_gpio_pin_write(BSP_LED_1, LED_ON);
				nrf_delay_ms(100);
				nrf_gpio_pin_write(BSP_LED_0, LED_OFF);
				nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
			}else if(lastEsbEvent == NRF_ESB_EVENT_RX_RECEIVED) {
				for(int i=0; i< 2;i++) {
					nrf_gpio_pin_write(BSP_LED_0, LED_ON);
					nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
					nrf_delay_ms(100);
					nrf_gpio_pin_write(BSP_LED_0, LED_OFF);
					nrf_gpio_pin_write(BSP_LED_1, LED_ON);
					nrf_delay_ms(100);
				}
				nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
			}else if(lastEsbEvent == NRF_ESB_EVENT_TX_FAILED) {
				for(int i=0;i< 5; i++) {
					nrf_gpio_pin_write(BSP_LED_0, LED_ON);
					nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
					nrf_delay_ms(20);
					nrf_gpio_pin_write(BSP_LED_0, LED_OFF);
					nrf_gpio_pin_write(BSP_LED_1, LED_ON);
					nrf_delay_ms(20);
				}
				nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
			}
			#endif

			if (nrf_gpio_pin_read(BUTTON_1) == BTN_RELEASED &&
					nrf_gpio_pin_read(BUTTON_2) == BTN_RELEASED && 1
			//                nrf_gpio_pin_read(BUTTON_3) == BTN_RELEASED &&
			//                nrf_gpio_pin_read(BUTTON_4) == BTN_RELEASED
				) {
					system_off();
			}
		}
	}
}
/*lint -restore */
