#ifndef __NETWORK_PHY_H__
#define __NETWORK_PHY_H__

#include <stdint.h>

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
	
	void (* Start_RX) (void);
	void (* Stop_RX) (void);
}Network_Phy_t;

extern Network_Phy_t NetPhy;

#endif /* __NETWORK_PHY_H__ */
