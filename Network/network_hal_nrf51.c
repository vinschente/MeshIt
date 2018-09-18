#include "network_hal.h"

#include "boards.h"

#include "nrf_esb.h"
#include "nrf_log.h"

#define LED_ON          0
#define LED_OFF         1

typedef enum {
	NRF51_IDLE,
	NRF51_RX,
	NRF51_STATE_BUSY_TX,
	NRF51_STATE_TX_SUCCESS,
	NRF51_STATE_TX_FAILED,
	NRF51_STATE_RX_RECEIVED,
}NRF51_HAL_State_t;


static nrf_esb_payload_t tx_payload;
static nrf_esb_payload_t rx_payload;

volatile static NRF51_HAL_State_t State = NRF51_IDLE;

Status_t NRF51_RF_Init(void);
void NRF51_RF_Start_RX(void);
void NRF51_RF_Stop_RX(void);
Status_t NRF51_RF_Send(uint8_t *pBuffer, uint16_t size, bool ack);
nrf_esb_payload_t* NRF51_RF_hasReceived(void);

static void NRF51_HAL_LowPower(void);

Network_Driver_t NetDriver = {
	.Init        = NRF51_RF_Init,
	.Start_RX    = NRF51_RF_Start_RX,
	.Stop_RX     = NRF51_RF_Stop_RX,
	.Send        = NRF51_RF_Send,
	.hasReceived = NRF51_RF_hasReceived,
};

static void nrf_esb_event_handler(nrf_esb_evt_t const * p_event);

Status_t NRF51_RF_Init(void)
{
    uint32_t err_code;
	
	State = NRF51_IDLE;
	
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

#ifndef NRF_ESB_LEGACY
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
#else // NRF_ESB_LEGACY
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_LEGACY_CONFIG;
#endif // NRF_ESB_LEGACY
    nrf_esb_config.selective_auto_ack       = 0;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
		nrf_esb_config.selective_auto_ack       = true;

    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_1(base_addr_1);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}


void NRF51_RF_Start_RX(void)
{
	nrf_esb_start_rx();
}

void NRF51_RF_Stop_RX(void)
{
	nrf_esb_stop_rx();
}

Status_t NRF51_RF_Send(uint8_t *pBuffer, uint16_t size, bool ack)
{
	uint32_t errorcode;
	
	tx_payload.length = size;
	tx_payload.noack = ack?0:1;
	memcpy(tx_payload.data, pBuffer, size);
	errorcode = nrf_esb_write_payload(&tx_payload);
	if(errorcode != NRF_SUCCESS) {
		// Clean up complete TX FIFO
		nrf_esb_flush_tx();
		printf("[ESB HAL] Send nrf_esb_write_payload error\r\n");
		return NET_ERROR_HAL;
	}
	
	if(tx_payload.noack == true) {
		// Remove payload from TX FIFO
		nrf_esb_pop_tx();
		return NET_SEND_OK_NOACK;
	}
	
	while(State == NRF51_STATE_BUSY_TX) {
		NRF51_HAL_LowPower();
	}
	
	if(State == NRF51_STATE_TX_SUCCESS || State == NRF51_STATE_RX_RECEIVED) {
		return NET_SEND_OK;
	}else {
		nrf_esb_pop_tx();
		return NET_SEND_ERROR_NOACK;
	}
}

nrf_esb_payload_t* NRF51_RF_hasReceived(void)
{
	if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) {
//		tx_payload.length = 1;
//		tx_payload.data[0]++; 
//		nrf_esb_write_payload(&tx_payload);
		
		#if defined(BSP_LED_1)
			nrf_gpio_pin_write(BSP_LED_1, LED_ON);
		#endif
		printf("[ESB HAL] Receiving packet\r\n");
		printf("\t length:%d\r\n", rx_payload.length);
		printf("\t pipe:%d\r\n", rx_payload.pipe);
		printf("\t rssi:%d\r\n", rx_payload.rssi);
		printf("\t noack:%d\r\n", rx_payload.noack);
		printf("\t pid:%d\r\n", rx_payload.pid);
		printf(":");
		for(int i=0; i<rx_payload.length; i++) {
			printf(" 0x%02X", rx_payload.data[i]);
		}
		printf("\r\n");
		#if defined(BSP_LED_1)
			nrf_gpio_pin_write(BSP_LED_1, LED_OFF);
		#endif
		return &rx_payload;
	}
	return NULL;
}

static void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
	printf("[ESB Handle] ID:%d TX_attempts:%d\r\n", p_event->evt_id, p_event->tx_attempts);
	
	switch (p_event->evt_id) {
		case NRF_ESB_EVENT_TX_SUCCESS:
			NRF_LOG_DEBUG("SUCCESS\r\n");
			State = NRF51_STATE_TX_SUCCESS;
			break;
		case NRF_ESB_EVENT_TX_FAILED:
			NRF_LOG_DEBUG("FAILED\r\n");
			(void) nrf_esb_flush_tx();
			State = NRF51_STATE_TX_FAILED;
			break;
		case NRF_ESB_EVENT_RX_RECEIVED:
			NRF_LOG_DEBUG("RX RECEIVED\r\n");
			State = NRF51_STATE_RX_RECEIVED;
			break;
	}
}

static void NRF51_HAL_LowPower(void)
{
	__WFE();
	__SEV();
	__WFE();
}
