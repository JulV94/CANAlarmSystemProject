#ifndef CONSTANTS_H
#define CONSTANTS_H

/////////////////////////////////////////////////////////////////////////////
//                              TASK PRIORITIES                            //
/////////////////////////////////////////////////////////////////////////////

#define  APP_TASK_START_PRIO              9
#define  HEARTBEAT_CHECK_PRIO             11
#define  HEARTBEAT_PRIO							      12
#define  TAKE_ID_PRIO						          13
#define  STATE_MACHINE_PRIO               15
#define  ALARM_PRIO                   	  20
#define  SEND_PRIO                     	  25
#define  READ_KB_PRIO                     28
#define  PWD_CHECK_PRIO                   30
#define  DISP_STATE_PRIO                 	40
#define  DISP_KB_PRIO                  		41
#define  DISP_CHG_PWD_PRIO               	42

/////////////////////////////////////////////////////////////////////////////
//                            NETWORK PRIORITIES                           //
/////////////////////////////////////////////////////////////////////////////

#define HEARTBEAT_NET_PRIO              	0
#define INTRUSION_NET_PRIO                1
#define DISARMED_NET_PRIO	                4
#define ARMED_NET_PRIO  		              6
#define ALARM_NET_PRIO  		              8
#define PWD_CHG_NET_PRIO                  16

/////////////////////////////////////////////////////////////////////////////
//                                  FILTERS                                //
/////////////////////////////////////////////////////////////////////////////
#define HEARTBEAT_FILTER                  0
#define INTRUSION_FILTER                  1
#define DISARMED_FILTER                   2
#define ARMED_FILTER                      3
#define ALARM_FILTER                      4
#define PWD_CHG_FILTER                    5


// Periods
#define HEARTBEAT_PERIOD                  4000
#define READ_KB_PERIOD                    100

/////////////////////////////////////////////////////////////////////////////
//                                 PROTOCOLS                               //
/////////////////////////////////////////////////////////////////////////////
#define INIT_STATE                        0
#define DISARMED_STATE                    1
#define PWD_CHG_STATE                     2
#define ARMED_STATE                       3
#define INTRUSION_STATE                   4
#define ALARM_STATE                       5


#define HEARTBEAT_MSG                     0
#define INTRUSION_MSG                     1
#define ARMED_MSG                         2
#define DISARMED_MSG                      3
#define ALARM_MSG                         4
#define PWD_CHG_MSG                       5

#endif
