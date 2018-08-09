#include "network.h"

void Network_Init(NetworkHandler_t *hNet)
{
	hNet->Driver = &NetDriver;
	hNet->Driver->Init();
	
	
	if(hNet->Init.DeviceType != DEVICE_NODE) {
		hNet->Driver->Start_RX();
		hNet->State = NET_STATE_BUSY_RX;
	}
}

void Network_SendBuffer(NetworkHandler_t *hNet, uint16_t dstAddr, uint8_t buffer, uint16_t size)
{
	
	
}


void Network_Manage(NetworkHandler_t *hNet)
{
	nrf_esb_payload_t* payload = hNet->Driver->hasReceived();
	if(payload) {
		
	}
}