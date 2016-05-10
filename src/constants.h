#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PASSWD0                           '1'
#define PASSWD1                           '2'
#define PASSWD2                           '3'
#define PASSWD3                           '4'

/////////////////////////////////////////////////////////////////////////////
//                              TASK PRIORITIES                            //
/////////////////////////////////////////////////////////////////////////////

#define  APP_TASK_START_PRIO              9
#define  HEARTBEAT_CHECK_PRIO             12
#define  HEARTBEAT_PRIO							      13
#define  TAKE_ID_PRIO						          14
#define  STATE_MACHINE_PRIO               15
#define  ALARM_PRIO                   	  20
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
#define READ_KB_PERIOD                    50
#define STATE_DISP_REFRESH_PERIOD				  2000
#define PWD_RESULT_DISP_TIME			  1500
#define HEARTBEAT_CHECK_PERIOD			  500
#define ALARM_PERIOD					  500
#define INTRUSION_TMR_TIME				  30000

/////////////////////////////////////////////////////////////////////////////
//                                 PROTOCOLS                               //
/////////////////////////////////////////////////////////////////////////////
#define INIT_STATE                        6
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

// Nodes count

#define MAX_NODES 10

#define PWD_OK							  1
#define PWD_WRONG						  2

#endif
