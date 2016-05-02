//////////////////////////////////////////////////////////////////////////////
//									INCLUDES								//
//////////////////////////////////////////////////////////////////////////////
#include <includes.h>	// uC/OSII includes
#include "Keyboard.h"	// Keyboard functions library
#include "CanDspic.h"


/*
*********************************************************************************************************
*                                       TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                    9
#define  HEARTBEAT_CHECK_PRIO                   11 
#define  HEARTBEAT_PRIO							12
#define  TAKE_ID_PRIO							13
#define  STATE_MACHINE_PRIO                     15
#define  ALARM_PRIO                   			20
#define  SEND_PRIO                     			25
#define  READ_KB_PRIO                     		28
#define  PWD_CHECK_PRIO                     	30
#define  DISP_STATE_PRIO                     	40
#define  DISP_KB_PRIO                     		41
#define  DISP_CHG_PWD_PRIO                     	42

//////////////////////////////////////////////////////////////////////////////
//								STRUCTURES									//
//////////////////////////////////////////////////////////////////////////////

struct CANMsg {
	INT8U id;
	INT8U msg;
};

//////////////////////////////////////////////////////////////////////////////
//									CONSTANTES								//
//////////////////////////////////////////////////////////////////////////////
const INT8U hex2ASCII[16] = {	'0', '1', '2', '3', '4', '5', '6', '7',
								'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//////////////////////////////////////////////////////////////////////////////
//									VARIABLES								//
//////////////////////////////////////////////////////////////////////////////
// Tasks stack
#define  APP_TASK_START_STK_SIZE			128
OS_STK  AppStartTaskStk[APP_TASK_START_STK_SIZE];
#define  HEARTBEAT_CHECK_TASK_STK_SIZE		128
OS_STK	HeartbeatCheckTaskStk[HEARTBEAT_CHECK_TASK_STK_SIZE];
#define  HEARTBEAT_TASK_STK_SIZE			128
OS_STK	HeartbeatTaskStk[HEARTBEAT_TASK_STK_SIZE];
#define  TAKE_ID_TASK_STK_SIZE				128
OS_STK	TakeIdTaskStk[TAKE_ID_TASK_STK_SIZE];
#define  STATE_MACHINE_TASK_STK_SIZE		128
OS_STK	StateMachineTaskStk[STATE_MACHINE_TASK_STK_SIZE];
#define  ALARM_TASK_STK_SIZE				128
OS_STK	AlarmTaskStk[ALARM_TASK_STK_SIZE];
#define  SEND_TASK_STK_SIZE					128
OS_STK	SendTaskStk[SEND_TASK_STK_SIZE];
#define  READ_KB_TASK_STK_SIZE				128
OS_STK	ReadKbTaskStk[READ_KB_TASK_STK_SIZE];
#define  PWD_CHECK_TASK_STK_SIZE			128
OS_STK	PwdCheckTaskStk[PWD_CHECK_TASK_STK_SIZE];
#define  DISP_STATE_TASK_STK_SIZE			128
OS_STK	DispStateTaskStk[DISP_STATE_TASK_STK_SIZE];
#define  DISP_KB_TASK_STK_SIZE				128
OS_STK	DispKbTaskStk[DISP_KB_TASK_STK_SIZE];
#define  DISP_CHG_PWD_TASK_STK_SIZE			128
OS_STK	DispChgPwdTaskStk[DISP_CHG_PWD_TASK_STK_SIZE];


// Periods
#define HEARTBEAT_PERIOD				4000
#define READ_KB_PERIOD					100

// Semaphores
OS_EVENT *alarmSM;
OS_EVENT *validPwdOkSM;
OS_EVENT *HBMissmatchSM;

// Mailboxes
OS_EVENT *idListMB;
OS_EVENT *idMB;
OS_EVENT *heartbeatMB;
OS_EVENT *statusMsgInMB;
OS_EVENT *charTypedMB;
OS_EVENT *KBMsgMB;
OS_EVENT *charTypedMB;
OS_EVENT *stateMB;
OS_EVENT *validationResultMB;
OS_EVENT *CANMsgOutMB;

// Queues
OS_EVENT *allCANMsgInQueue;
CANMsg *allCANMsgInArray[30];


//////////////////////////////////////////////////////////////////////////////
//							FUNCTION PROTOTYPES								//
//////////////////////////////////////////////////////////////////////////////
static  void  AppStartTask(void *p_arg);
static  void  HeartbeatCheckTask(void *p_arg);
static  void  HeartbeatTask(void *p_arg);
static  void  TakeIdTask(void *p_arg);
static  void  StateMachineTask(void *p_arg);
static  void  AlarmTask(void *p_arg);
static  void  SendTask(void *p_arg);
static  void  ReadKbTask(void *p_arg);
static  void  PwdCheckTask(void *p_arg);
static  void  DispStateTask(void *p_arg);
static  void  DispKbTask(void *p_arg);
static  void  DispChgPwdTask(void *p_arg);

#if (uC_PROBE_OS_PLUGIN > 0) || (uC_PROBE_COM_MODULE > 0)
extern  void  AppProbeInit(void);
#endif

//////////////////////////////////////////////////////////////////////////////
//						  		PROTOCOLS									//
//////////////////////////////////////////////////////////////////////////////
#define INIT_STATE		0
#define DISARMED_STATE	1
#define ARMED_STATE		2
#define INTRUSION_STATE	3
#define ALARM_STATE		4
#define PWD_CHG_STATE	5

#define HEARTBEAT_MSG	0
#define INTRUSION_MSG	1
#define ARMED_MSG		2
#define DISARMED_MSG	3
#define ALARM_MSG		4
#define PWD_CHG_MSG		5


//////////////////////////////////////////////////////////////////////////////
//							MAIN FUNCTION									//
//////////////////////////////////////////////////////////////////////////////
CPU_INT16S  main (void)
{
	CPU_INT08U  err;


	BSP_IntDisAll();	// Disable all interrupts until we are ready to accept them

	OSInit();			// Initialize "uC/OS-II, The Real-Time Kernel"

	// create semaphores
	alarmSM = OSSemCreate(1);
	validPwdOkSM = OSSemCreate(2);
	HBMissmatchSM = OSSemCreate(3);
	
	// create mailbox
	idListMB = OSMboxCreate((void *)0);
	idMB = OSMboxCreate((void *)0);
	heartbeatMB = OSMboxCreate((void *)0);
	statusMsgInMB = OSMboxCreate((void *)0);
	charTypedMB = OSMboxCreate((void *)0);
	KBMsgMB = OSMboxCreate((void *)0);
	charTypedMB = OSMboxCreate((void *)0);
	stateMB = OSMboxCreate((void *)0);
	validationResultMB = OSMboxCreate((void *)0);
	CANMsgOutMB = OSMboxCreate((void *)0);

	// create queues
	allCANMsgInQueue = OSQCreate(&allCANMsgInArray[0], 20);

	OSTaskCreateExt(
			AppStartTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&AppStartTaskStk[0],
			APP_TASK_START_PRIO,
			APP_TASK_START_PRIO,
			(OS_STK *)&AppStartTaskStk[APP_TASK_START_STK_SIZE-1],
			APP_TASK_START_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *)"Start Tasks", &err);
    
    
	//DispInitOS();
	OSStart();		// Start multitasking (i.e. give control to uC/OS-II)

	
	//LedTask();

	return (-1);	// Return an error - This line of code is unreachable
}



//////////////////////////////////////////////////////////////////////////////
//								AppStartTask								//
//////////////////////////////////////////////////////////////////////////////
static  void  AppStartTask (void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////

    BSP_Init();		// Initialize BSP (Board Support Package) functions

#if OS_TASK_STAT_EN > 0
    OSStatInit();	// Determine CPU capacity
#endif

#if (uC_PROBE_OS_PLUGIN > 0) || (uC_PROBE_COM_MODULE > 0)
    AppProbeInit();	// Initialize uC/Probe modules
#endif

    LED_Off(0);		// Turn OFF all the LEDs


//	Tasks creation
//////////////////

OSTaskCreateExt(
			HeartbeatCheckTask,
			(void *)0,
			(OS_STK *)&HeartbeatCheckTaskStk[0],
			HEARTBEAT_CHECK_PRIO,
			HEARTBEAT_CHECK_PRIO,
			(OS_STK *)&HeartbeatCheckTaskStk[HEARTBEAT_CHECK_TASK_STK_SIZE-1],
			HEARTBEAT_CHECK_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(HEARTBEAT_CHECK_PRIO, (CPU_INT08U *)"Heartbeat check Task", &err);

OSTaskCreateExt(
			HeartbeatTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&HeartbeatTaskStk[0],
			HEARTBEAT_PRIO,
			HEARTBEAT_PRIO,
			(OS_STK *)&HeartbeatTaskStk[HEARTBEAT_TASK_STK_SIZE-1],
			HEARTBEAT_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(HEARTBEAT_PRIO, (CPU_INT08U *)"heartbeat Task", &err);

OSTaskCreateExt(
			TakeIdTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&TakeIdTaskStk[0],
			TAKE_ID_PRIO,
			TAKE_ID_PRIO,
			(OS_STK *)&TakeIdTaskStk[TAKE_ID_TASK_STK_SIZE-1],
			TAKE_ID_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(TAKE_ID_PRIO, (CPU_INT08U *)"Take Id Task", &err);

OSTaskCreateExt(
			StateMachineTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&StateMachineTaskStk[0],
			STATE_MACHINE_PRIO,
			STATE_MACHINE_PRIO,
			(OS_STK *)&StateMachineTaskStk[STATE_MACHINE_TASK_STK_SIZE-1],
			STATE_MACHINE_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(STATE_MACHINE_PRIO, (CPU_INT08U *)"STATE MACHINE", &err);

OSTaskCreateExt(
			AlarmTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&AlarmTaskStk[0],
			ALARM_PRIO,
			ALARM_PRIO,
			(OS_STK *)&AlarmTaskStk[ALARM_TASK_STK_SIZE-1],
			ALARM_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(ALARM_PRIO, (CPU_INT08U *)"Alarm Task", &err);

OSTaskCreateExt(
			SendTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&SendTaskStk[0],
			SEND_PRIO,
			SEND_PRIO,
			(OS_STK *)&SendTaskStk[SEND_TASK_STK_SIZE-1],
			SEND_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(SEND_PRIO, (CPU_INT08U *)"Send Task", &err);

OSTaskCreateExt(
			ReadKbTask,		// creates AppStartTask
			(void *)0,
			(OS_STK *)&ReadKbTaskStk[0],
			READ_KB_PRIO,
			READ_KB_PRIO,
			(OS_STK *)&ReadKbTaskStk[READ_KB_TASK_STK_SIZE-1],
			READ_KB_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(READ_KB_PRIO, (CPU_INT08U *)"Read Kb Task", &err);

OSTaskCreateExt(
			PwdCheckTask,
			(void *)0,
			(OS_STK *)&PwdCheckTaskStk[0],
			PWD_CHECK_PRIO,
			PWD_CHECK_PRIO,
			(OS_STK *)&PwdCheckTaskStk[PWD_CHECK_TASK_STK_SIZE-1],
			PWD_CHECK_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(PWD_CHECK_PRIO, (CPU_INT08U *)"Pwd Check Task", &err);

OSTaskCreateExt(
			DispChgPwdTask,
			(void *)0,
			(OS_STK *)&DispChgPwdTaskStk[0],
			DISP_CHG_PWD_PRIO,
			DISP_CHG_PWD_PRIO,
			(OS_STK *)&DispChgPwdTaskStk[DISP_CHG_PWD_TASK_STK_SIZE-1],
			DISP_CHG_PWD_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(DISP_CHG_PWD_PRIO, (CPU_INT08U *)"Disp Chg Pwd Task", &err);


OSTaskCreateExt(
			DispStatetTask,
			(void *)0,
			(OS_STK *)&DispStateTaskStk[0],
			DISP_STATE_PRIO,
			DISP_STATE_PRIO,
			(OS_STK *)&DispStateTaskStk[DISP_STATE_TASK_STK_SIZE-1],
			DISP_STATE_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(DISP_STATE_PRIO, (CPU_INT08U *)"Disp State Task", &err);

OSTaskCreateExt(
			DispKbTask,
			(void *)0,
			(OS_STK *)&DispKbTaskStk[0],
			DISP_KB_PRIO,
			DISP_KB_PRIO,
			(OS_STK *)&DispKbTaskStk[DISP_KB_TASK_STK_SIZE-1],
			DISP_KB_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// defines the App Name (for debug purpose)
    OSTaskNameSet(DISP_KB_PRIO, (CPU_INT08U *)"Disp Kb Task", &err);	



//	Infinite loop
/////////////////
    while (42)
	{
		OSTaskSuspend(OS_PRIO_SELF);
   	} 	
}

static  void  HeartbeatCheckTask(void *p_arg)
{
	INT8U err;

   (void)p_arg;	// to avoid a warning message

	INT8U *id;
	INT8U *state;

//	Initialisations
///////////////////
	INT16U nodesTTL[256] = {0};
	INT8U fixedIdList[256];
	static CANMsg msgInQueue[20];
	INT32U endTime=OSTimeGet();
	INT32U time;
	int i, j, sizeFixedList, size;
	static INT8U HBMissmatchBuf;

	
	id = (INT8U*)OSMboxPend(idMB, 0, &err);

//	Infinite loop
/////////////////
    while (42)
	{
		id
		msgInQueue = {null};
		msgInQueue = (CANMsg*)OSQPend(allCANMsgInQueue, 100, &err);
		time = (2e32-1)-abs(OSTimeGet()-endTime);
		for (i=0; i<256; i++)
		{
			nodesTTL[i]-=time;
			if (nodesTTL[i] <= 0)
			{
				nodesTTL[i]=0;
			}
		}
		endTime=OSTimeGet();
		j=0;
		while (msgInQueue[j] != null)
		{
			nodesTTL[msgInQueue[j].id]=5000;
			j++;
		}
		if (state[0] == INIT_STATE || state[0] == DISARMED_STATE || state[0] == PWD_CHG_STATE)
		{
			id = (INT8U*)OSMboxAccept(idMB);  // for when takeID changes id when already taken
			// update fixed list
			fixedIdList = {null};
			for (i=0; i<256; i++)
			{
				int sizeFixedList=0;
				if (nodesTTL[i] > 0)
				{
					fixedIdList[sizeFixedList]=i;
					sizeFixedList++;
				}
			}
		}
		else
		{
			// compare with fixed list
			for (i=0; i<256; i++)
			{
				size=0;
				if (nodesTTL[i] > 0)
				{
					if (size > sizeFixedList || fixedIdList[size] != i || nodesTTL[i] == id)
					{
						err = OSSemPost(HBMissmatchSM);
					}
					size++;
				}
				
			}
		}
		err = OSMboxPost(idListMB, (void *)&fixedIdList[0]);
		err = OSQFlush(allCANMsgInQueue);
		OSTimeDly(50);
   	} 	
}

static  void  HeartbeatTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message

	INT8U *id;
	INT32U timeStart;
	CANMsg heartbeat;

//	Initialisations
///////////////////
	id = (INT8U*)OSMboxPend(idMB, 0, &err);
	heartbeat.id = id;
	heartbeat.msg = HEARTBEAT_MSG;

//	Infinite loop
/////////////////
    while (1)
	{
		timeStart = OSTimeGet();
		id = (INT8U*)OSMboxAccept(idMB);
		err = OSMboxPost(heartbeatMB, (void *)&heartbeat);
		OSTimeDly(HEARTBEAT_PERIOD-((2e32-1)-abs(OSTimeGet()-timeStart)));
   	} 	
}

static  void  TakeIdTask(void *p_arg)
{
	INT8U err;

	INT8U *id;
	INT8U idList[256];
	int i;

   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	id = rand()*256;
	
	do {
		err = OSMboxPost(idMB, (void *)&id);
		idList = (INT8U*)OSMboxPend(idListMB, 0, &err);
		i=0;
		while (idList[i] != null)
		{
			if (idList[i] == id)
			{
				id = rand()*256;
				i=-1;
			}
			i++;
		}
	}while (idList == (INT8U*)OSMboxPend(idListMB, 0, &err));

//	Infinite loop
/////////////////
    while (1)
	{
		
   	} 	
}

static  void  StateMachineTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	
	

//	Infinite loop
/////////////////
    while (1)
	{
		
   	} 	
}

static  void  AlarmTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////


//	Infinite loop
/////////////////
    while (1)
	{
                OSSemPend(AlarmSM, 0, &err);
                // launch the alarm code
		
   	} 	
}

static  void  SendTask(void *p_arg)

{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
        CanInitialisation(CAN_OP_MODE_NORMAL , CAN_BAUDRATE_500k);
        transmitBuffer.SID = 0x189;
        transmitBuffer.DLC = 6;
        transmitBuffer.DATA[0] = 'F';
        transmitBuffer.DATA[1] = 'N';
        transmitBuffer.DATA[2] = 'J';
        transmitBuffer.DATA[3] = 'V';
        transmitBuffer.DATA[4] = 'V';
        transmitBuffer.DATA[5] = 'D';

        CanLoadFilter(0, 0x2A);
        CanLoadFilter(1, 0x2B);
        CanLoadFilter(2, 0x2C);
        CanLoadFilter(3, 0x42);
        CanLoadMask(0, 0x7FF);
        CanAssociateMaskFilter(0, 0);
        CanAssociateMaskFilter(0, 1);
        CanAssociateMaskFilter(0, 2);
        CanAssociateMaskFilter(0, 3);
        ACTIVATE_CAN_INTERRUPTS = 1;


//	Infinite loop
/////////////////
    while (1)
	{

   	} 	
}

static  void  ReadKbTask(void *p_arg)

{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
        INT32U timeStart;


//	Infinite loop
/////////////////
    while (1)
	{
            timeStart = OSTimeGet();


            OSTimeDly(READ_KB_PERIOD-((2e32-1)-abs(OSTimeGet()-timeStart)));
   	} 	
}

static  void  PwdCheckTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	

//	Infinite loop
/////////////////
    while (1)
	{

   	} 	
}

static  void  DispStateTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	
	DispInit(2, 16);
	DispClrScr();

//	Infinite loop
/////////////////
    while (1)
	{
                msg = (INT8U*)OSMboxPend(stateMB, 0, &err);
                DispClrScr();
                DispStr(0, 0, msg);
   	} 	
}

static  void  DispKbTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
        // Done in DispStateTask()
	

//	Infinite loop
/////////////////
    while (1)
	{
                msg = (INT8U*)OSMboxPend(validationResultMB, 0, &err);
                DispClrScr();
                DispStr(0, 0, msg);
   	} 	
}

static  void  DispChgPwdTask(void *p_arg)
{
	INT8U err;


   (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	


//	Infinite loop
/////////////////
    while (1)
	{

   	} 	
}
