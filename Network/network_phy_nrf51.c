#include "network_phy.h"

#include "nrf_esb.h"
#include "nrf_log.h"

static nrf_esb_payload_t tx_payload;
static nrf_esb_payload_t rx_payload;

Status_t NRF51_RF_Init(void);
void NRF51_RF_Start_RX(void);
void NRF51_RF_Stop_RX(void);

Network_Phy_t NetPhy = {
	.Init = NRF51_RF_Init,
	.Start_RX = NRF51_RF_Start_RX,
	.Stop_RX  = NRF51_RF_Stop_RX,
};

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event);

Status_t NRF51_RF_Init(void)
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


void NRF51_RF_Start_RX(void)
{
	nrf_esb_start_rx();
}

void NRF51_RF_Stop_RX(void)
{
	nrf_esb_stop_rx();
}


void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
            NRF_LOG_DEBUG("SUCCESS\r\n");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            NRF_LOG_DEBUG("FAILED\r\n");
            (void) nrf_esb_flush_tx();
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            while (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS) {
							NRF_LOG_DEBUG("Receiving packet: %x\r\n", rx_payload.data[0]);
						}
//            (void) nrf_esb_write_payload(&tx_payload);

            NRF_LOG_DEBUG("Queue transmitt packet: %02x\r\n", tx_payload.data[0]);
            break;
    }
}

