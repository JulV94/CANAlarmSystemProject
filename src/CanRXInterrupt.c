#include "includes.h"
#include "CanDspic.h"

extern OS_EVENT *allCANMsgInQueue, *statusMsgInMB;
INT8U netState;

void __attribute__((interrupt, no_auto_psv))_C1Interrupt(void)
{
	if (CAN_RX_BUFFER_IF){
			OSQPost(allCANMsgInQueue, (void*)&receiveBuffers[0].DATA[0]); // send for heartbeat
			if(CAN_RX_BUFFER_0){ // heartbeat
				//memcpy(mess, receiveBuffers[0].DATA, 1);
				CAN_RX_BUFFER_0 = 0;
			}
			if(CAN_RX_BUFFER_1){ // intrusion
				netState = INTRUSION_STATE;
				CAN_RX_BUFFER_1 = 0;
			}
			if(CAN_RX_BUFFER_2){ // disarmed
				netState = DISARMED_STATE;
				CAN_RX_BUFFER_2 = 0;
			}
			if(CAN_RX_BUFFER_3){  // armed
				netState = ARMED_STATE;
				CAN_RX_BUFFER_3 = 0;
			}
			if(CAN_RX_BUFFER_4){  // alarm
				netState = ALARM_STATE;
				CAN_RX_BUFFER_4 = 0;
			}
			if(CAN_RX_BUFFER_5){  // pwd chg
				netState = PWD_CHG_STATE;
				CAN_RX_BUFFER_5 = 0;
			}
			OSMboxPost(statusMsgInMB, (void*)&netState);
			CAN_RX_BUFFER_IF = 0;
		}
		CAN_INTERRUPT_FLAG = 0;
}
