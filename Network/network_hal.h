#ifndef __NETWORK_HAL_H__
#define __NETWORK_HAL_H__

#include <stdint.h>
#include "nrf_esb.h"

typedef enum {
	NET_OK = 0,
	NET_ERROR_PARAM,
	NET_ERROR_NAK,
	NET_NO_DATA,
}Status_t;

typedef struct {
	Status_t (* Init) (void);
	
	Status_t (* Send) (uint8_t *buffer, uint16_t size);
	uint8_t  (* Receive) (uint8_t *buffer, uint16_t size);
	
	nrf_esb_payload_t* (* hasReceived) (void);
	
	void (* Start_RX) (void);
	void (* Stop_RX) (void);
}Network_Driver_t;

extern Network_Driver_t NetDriver;

#endif /* __NETWORK_HAL_H__ */
