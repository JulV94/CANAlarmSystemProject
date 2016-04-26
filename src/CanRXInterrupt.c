#include "includes.h"
#include "CanDspic.h"

extern OS_EVENT *CANReceiveSem;
extern char mess[4][8];

void __attribute__((interrupt, no_auto_psv))_C1Interrupt(void)  
{    
	if (CAN_RX_BUFFER_IF){
			if(CAN_RX_BUFFER_0){
				memcpy(mess[0], receiveBuffers[0].DATA, receiveBuffers[0].DLC);
				CAN_RX_BUFFER_0 = 0;
			}
			if(CAN_RX_BUFFER_1){
				memcpy(mess[1], receiveBuffers[1].DATA, receiveBuffers[1].DLC);
				CAN_RX_BUFFER_1 = 0;
			}
			if(CAN_RX_BUFFER_2){
				memcpy(mess[2], receiveBuffers[2].DATA, receiveBuffers[2].DLC);
				CAN_RX_BUFFER_2 = 0;
			}
			if(CAN_RX_BUFFER_3){
				memcpy(mess[3], receiveBuffers[3].DATA, receiveBuffers[3].DLC);
				CAN_RX_BUFFER_3 = 0;
			}
			CAN_RX_BUFFER_IF = 0;
			OSSemPost(CANReceiveSem);
		}
		CAN_INTERRUPT_FLAG = 0;
}
