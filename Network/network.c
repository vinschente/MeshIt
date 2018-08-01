#include "network.h"

void Network_Init(NetworkHandler_t *hNet)
{
	hNet->Phy->Init();
	
	if(hNet->Init.DeviceType != DEVICE_NODE) {
		hNet->Phy->Start_RX();
	}
}

void Network_SendBuffer(NetworkHandler_t *hNet, uint16_t dstAddr, uint8_t buffer, uint16_t size)
{
	
	
}


