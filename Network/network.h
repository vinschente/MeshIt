#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdint.h>

#include "network_hal.h"

typedef enum {
	NET_STATE_RESET      = 0x00,
	NET_STATE_READY      = 0x01,
	NET_STATE_BUSY       = 0x02,
	NET_STATE_BUSY_TX    = 0x12,
	NET_STATE_BUSY_RX    = 0x22,
	NET_STATE_BUSY_RX_TX = 0x32,
	NET_STATE_TIMEOUT    = 0x03,
	NET_STATE_ERROR      = 0x04,
}Network_State_t;

typedef enum {
	DEVICE_NODE        = 0,
	DEVICE_ROUTER      = 1,
	DEVICE_GATEWAY     = 2,
	DEVICE_BIG_BROTHER = 3,
}DeviceType_t;

typedef struct {
	uint16_t dstAddr;
	uint16_t srcAddr;
	uint16_t flags;
}Network_Message_t;

typedef struct {
	uint16_t myAddr;
	uint16_t hostAddr;
	DeviceType_t DeviceType;
	
	uint8_t *TxBufferPtr;
	
	void (* Message_Received_Callback)(Network_Message_t *msg);
	void (* Message_Send_Callback)(Status_t Status);
}Network_Init_t;

typedef struct {
	Network_Init_t Init;
	Network_Driver_t  *Driver;

	Network_State_t State;
	uint32_t        ErrorCode;
}NetworkHandler_t;

void Network_Init(NetworkHandler_t *hNet);

void Network_Manage(NetworkHandler_t *hNet);

void Network_SendBuffer(NetworkHandler_t *hNet, uint16_t dstAddr, uint8_t buffer, uint16_t size);


#endif /* __NETWORK_H__ */
