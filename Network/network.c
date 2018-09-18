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

void Network_SendBuffer(NetworkHandler_t *hNet, uint16_t dstAddr, uint8_t *buffer, uint16_t size)
{
	memset(hNet->Init.TxBufferPtr, 0, MESSAGE_SIZE);

	hNet->Init.TxBufferPtr[0] = hNet->Init.myAddr >> 8;
	hNet->Init.TxBufferPtr[1] = hNet->Init.myAddr & 0xFF;

	hNet->Init.TxBufferPtr[2] = dstAddr >> 8;
	hNet->Init.TxBufferPtr[3] = dstAddr & 0xFF;

	hNet->Init.TxBufferPtr[4] = 0;
	hNet->Init.TxBufferPtr[5] = 0;
	
	
	if(size < MESSAGE_SIZE-6) {
		memcpy(&hNet->Init.TxBufferPtr[6], buffer, size);
		Status_t status = hNet->Driver->Send(hNet->Init.TxBufferPtr, 6+size, true);
		if(status == NET_SEND_OK) {
			printf("Send OK\r\n");
			
			return;
		}else if(){
			printf("Send Error: No ACK\r\n");
			// TODO try to find a different route
		}
		
	}else {
		// TODO send fragmented messages
		printf("Send Error: Buffer to big\r\n");
	}
}


void Network_Manage(NetworkHandler_t *hNet)
{
	nrf_esb_payload_t* payload = hNet->Driver->hasReceived();
	if(payload) {
		
	}
}