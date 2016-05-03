#include "includes.h"
#include "CanDspic.h"

extern OS_EVENT *allCANMsgInQueue, *stateMB;
//char mess[20] = {-1};
INT8U state[1];

void __attribute__((interrupt, no_auto_psv))_C1Interrupt(void)  
{    
	if (CAN_RX_BUFFER_IF){
			if(CAN_RX_BUFFER_0){ // heartbeat
				//memcpy(mess, receiveBuffers[0].DATA, 1);
				OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[0].DATA[0]);
				CAN_RX_BUFFER_0 = 0;
			}
			if(CAN_RX_BUFFER_1){ // intrusion
				state[0] = INTRUSION_STATE;
				OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[1].DATA[0]);
				CAN_RX_BUFFER_1 = 0;
			}
			if(CAN_RX_BUFFER_2){ // disarmed
				state[0] = DISARMED_STATE;
				OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[2].DATA[0]);
				CAN_RX_BUFFER_2 = 0;
			}
			if(CAN_RX_BUFFER_3){  // armed
				state[0] = ARMED_STATE;
				OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[3].DATA[0]);
				CAN_RX_BUFFER_3 = 0;
			}
			if(CAN_RX_BUFFER_4){  // alarm
				state[0] = ALARM_STATE;
				OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[4].DATA[0]);
				CAN_RX_BUFFER_4 = 0;
			}
			if(CAN_RX_BUFFER_5){  // pwd chg
				state[0] = PWD_CHG_STATE;
				OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[5].DATA[0]);
				CAN_RX_BUFFER_5 = 0;
			}
			OSMboxPost(stateMB, (void*)&state[0]);
			CAN_RX_BUFFER_IF = 0;
		}
		CAN_INTERRUPT_FLAG = 0;
}
