//////////////////////////////////////////////////////////////////////////////
//																INCLUDES																	//
//////////////////////////////////////////////////////////////////////////////
#include <includes.h>	// uC/OSII includes
#include "Keyboard.h"	// Keyboard functions library
#include "CanDspic.h"

//////////////////////////////////////////////////////////////////////////////
//																	CONSTANTS																//
//////////////////////////////////////////////////////////////////////////////
const INT8U hex2ASCII[16] = {	'0', '1', '2', '3', '4', '5', '6', '7',
								'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//////////////////////////////////////////////////////////////////////////////
//																VARIABLES																	//
//////////////////////////////////////////////////////////////////////////////
// Tasks stack
#define  APP_TASK_START_STK_SIZE			128
OS_STK  AppStartTaskStk[APP_TASK_START_STK_SIZE];

#define  HEARTBEAT_CHECK_TASK_STK_SIZE		1024
OS_STK	HeartbeatCheckTaskStk[HEARTBEAT_CHECK_TASK_STK_SIZE];

#define  HEARTBEAT_TASK_STK_SIZE			128
OS_STK	HeartbeatTaskStk[HEARTBEAT_TASK_STK_SIZE];

#define  TAKE_ID_TASK_STK_SIZE				128
OS_STK	TakeIdTaskStk[TAKE_ID_TASK_STK_SIZE];

#define  STATE_MACHINE_TASK_STK_SIZE		128
OS_STK	StateMachineTaskStk[STATE_MACHINE_TASK_STK_SIZE];

#define  ALARM_TASK_STK_SIZE				128
OS_STK	AlarmTaskStk[ALARM_TASK_STK_SIZE];

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

// Semaphores
OS_EVENT *validPwdOkSM;
OS_EVENT *HBMissmatchSM;

// Mailboxes
OS_EVENT *idListMB;
OS_EVENT *statusMsgInMB;
OS_EVENT *charTypedMB;
OS_EVENT *KBMsgMB;
OS_EVENT *charTypedMB;
OS_EVENT *validationResultMB;
OS_EVENT *CANMsgOutMB;

// Queues
OS_EVENT *allCANMsgInQueue;
INT8U *allCANMsgInArray[200];

// Mutex
OS_EVENT *netBufMutex;
OS_EVENT *idMutex;
OS_EVENT *stateMutex;

// Global variables
INT8U id;
INT8U state;


//////////////////////////////////////////////////////////////////////////////
//														FUNCTION PROTOTYPES														//
//////////////////////////////////////////////////////////////////////////////
static  void  AppStartTask(void *p_arg);
static  void  HeartbeatCheckTask(void *p_arg);
static  void  HeartbeatTask(void *p_arg);
static  void  TakeIdTask(void *p_arg);
static  void  StateMachineTask(void *p_arg);
static  void  AlarmTask(void *p_arg);
static  void  ReadKbTask(void *p_arg);
static  void  PwdCheckTask(void *p_arg);
static  void  DispStateTask(void *p_arg);
static  void  DispKbTask(void *p_arg);
static  void  DispChgPwdTask(void *p_arg);

#if (uC_PROBE_OS_PLUGIN > 0) || (uC_PROBE_COM_MODULE > 0)
extern  void  AppProbeInit(void);
#endif

//////////////////////////////////////////////////////////////////////////////
//															MAIN FUNCTION																//
//////////////////////////////////////////////////////////////////////////////
CPU_INT16S  main (void)
{
	CPU_INT08U  err;


	BSP_IntDisAll();	// Disable all interrupts until we are ready to accept them
	init_elec_h_410();
	OSInit();			// Initialize "uC/OS-II, The Real-Time Kernel"

	// create semaphores
	validPwdOkSM = OSSemCreate(0);
	HBMissmatchSM = OSSemCreate(0);

	// create mailbox
	idListMB = OSMboxCreate((void *)0);
	statusMsgInMB = OSMboxCreate((void *)0);
	charTypedMB = OSMboxCreate((void *)0);
	KBMsgMB = OSMboxCreate((void *)0);
	charTypedMB = OSMboxCreate((void *)0);
	validationResultMB = OSMboxCreate((void *)0);
	CANMsgOutMB = OSMboxCreate((void *)0);

	// create queues
	allCANMsgInQueue = OSQCreate((void*)&allCANMsgInArray[0], 200);

	// create Mutex
	netBufMutex = OSMutexCreate(11, &err); // All task created can use the mutex
	idMutex = OSMutexCreate(11, &err);
	stateMutex = OSMutexCreate(11, &err);

	// enable interruptions button
	IEC1bits.CNIE = 1;
	CNEN1bits.CN15IE = 1;

	R6 input
	CN15 activate
	activate change notification block interrupt

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

	return (-1);	// Return an error - This line of code is unreachable
}



/////////////////////////////////////////////////////////////////////////////
//								AppStartTask								//
/////////////////////////////////////////////////////////////////////////////
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

		TASK_ENABLE1=0;

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
			DispStateTask,
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
    while (1)
		{
			OSTaskSuspend(OS_PRIO_SELF);
   	}
}

static  void  HeartbeatCheckTask(void *p_arg)
{
	INT8U err;
	INT8U stateCpy;

  (void)p_arg;	// to avoid a warning message

//	Initialisations
///////////////////
	int nodesTTL[256] = {0};
	int fixedIdList[256];
	INT8U *msgInQueue;
	INT32U endTime=OSTimeGet();
	INT32U time;
	int i, j, sizeFixedList, size, multiple=0;
	static INT8U HBMissmatchBuf;

	TASK_ENABLE1=1;

//	Infinite loop
/////////////////
    while (42)
		{
			time = /*4294967295-*/abs(OSTimeGet()-endTime);
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
		msgInQueue = (INT8U*)OSQPend(allCANMsgInQueue, 100, &err);
		while (msgInQueue[j] != NULL)
		{
			nodesTTL[msgInQueue[j]]=5000;
			j++;
		}
		OSMutexPend(stateMutex, 0, &err);
		stateCpy=state;
		OSMutexPost(stateMutex);
		stateCpy=INIT_STATE;
		if (stateCpy == INIT_STATE || stateCpy == DISARMED_STATE || stateCpy == PWD_CHG_STATE)
		{
			// update fixed list
			Mem_Set((void*)&fixedIdList[0], -1, sizeof(int)*256);
			sizeFixedList=0;
			for (i=0; i<256; i++)
			{
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
					OSMutexPend(idMutex, 0, &err);
					if (size > sizeFixedList || fixedIdList[size] != i || nodesTTL[i] == id)
					{
						TASK_ENABLE1=1;
						err = OSSemPost(HBMissmatchSM);
					}
					OSMutexPost(idMutex);
					size++;
				}

			}
		}
		if (multiple >= 5000)
		{
			err = OSMboxPost(idListMB, (void *)&fixedIdList[0]);
			multiple=0;
			TASK_ENABLE1^=1;
			TASK_ENABLE1=1;
		}
		//err = OSQFlush(allCANMsgInQueue);
		multiple+=HEARTBEAT_CHECK_PERIOD;
		OSTimeDly(HEARTBEAT_CHECK_PERIOD);
   	}
}

static  void  HeartbeatTask(void *p_arg)
{
	INT8U err;


	(void)p_arg;	// to avoid a warning message

	INT32U timeStart, magicNumber;
//	Initialisations
///////////////////
	TASK_ENABLE2=1;
	magicNumber = 4294967295;

//	Infinite loop
/////////////////
    while (1)
		{
			timeStart = OSTimeGet();
			OSMutexPend(netBufMutex, 4000, &err);
			transmitBuffer.SID = HEARTBEAT_NET_PRIO;
	    transmitBuffer.DLC = 1;
			OSMutexPend(idMutex, 0, &err);
			transmitBuffer.DATA[0] = id;
			OSMutexPost(idMutex);
			CanSendMessage();
			OSMutexPost(netBufMutex);
			TASK_ENABLE2^=1;
			OSTimeDly(HEARTBEAT_PERIOD-(magicNumber-abs(OSTimeGet()-timeStart))); // 2^32-1
   	}
}

static  void  TakeIdTask(void *p_arg)
{
	INT8U err;

	int *idList;
	int i;

  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	OSMutexPend(stateMutex, 0, &err);
	state=INIT_STATE;
	OSMutexPost(stateMutex);
	OSMutexPend(idMutex, 0, &err);
	id = rand()%256;
	OSMutexPost(idMutex);
	TASK_ENABLE3=1;

	do {
		idList = (int*)OSMboxPend(idListMB, 0, &err);
		i=0;
		while (idList[i] != -1)
		{
			if (idList[i] == id)
			{
				OSMutexPend(idMutex, 0, &err);
				id = rand()%256;
				OSMutexPost(idMutex);
				i=-1;
			}
			i++;
		}
	}while (idList == (int*)OSMboxPend(idListMB, 0, &err));
	OSMutexPend(stateMutex, 0, &err);
  	state = DISARMED_STATE;
	OSMutexPost(stateMutex);
	TASK_ENABLE3=0;

//	Infinite loop
/////////////////
    while (1)
		{
			OSTaskSuspend(OS_PRIO_SELF);
   	}
}

static  void  StateMachineTask(void *p_arg)
{
	INT8U err;
  INT8U networkState, previousState;

  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	CanInitialisation(CAN_OP_MODE_NORMAL , CAN_BAUDRATE_500k);
	CanLoadFilter(HEARTBEAT_FILTER, HEARTBEAT_NET_PRIO);
	CanLoadFilter(INTRUSION_FILTER, INTRUSION_NET_PRIO);
	CanLoadFilter(DISARMED_FILTER, DISARMED_NET_PRIO);
	CanLoadFilter(ARMED_FILTER, ARMED_NET_PRIO);
	CanLoadFilter(ALARM_FILTER, ALARM_NET_PRIO);
	CanLoadFilter(PWD_CHG_FILTER, PWD_CHG_NET_PRIO);
	CanLoadMask(0, 0x7FF);
	CanAssociateMaskFilter(0, HEARTBEAT_FILTER);
	CanAssociateMaskFilter(0, INTRUSION_FILTER);
	CanAssociateMaskFilter(0, DISARMED_FILTER);
	CanAssociateMaskFilter(0, ARMED_FILTER);
	CanAssociateMaskFilter(0, ALARM_FILTER);
	CanAssociateMaskFilter(0, PWD_CHG_FILTER);
	ACTIVATE_CAN_INTERRUPTS = 1;

//	Infinite loop
/////////////////
  	while (1)
		{
                networkState = ((int*)OSMboxAccept(statusMsgInMB))[0];
				OSMutexPend(stateMutex, 0, &err);
                previousState = state;
				
                if ((/*button pressed || */OSSemAccept(HBMissmatchSM)) && state < INTRUSION_STATE)
                {
                    state = INTRUSION_STATE;
                }
				TASK_ENABLE1=0;
                if (networkState > state)
                {
                    state = networkState;
                }
				TASK_ENABLE1=0;
                if (state != previousState)
                {
                    switch (state)
                    {
                    case INIT_STATE:

                        break;
                    case DISARMED_STATE:

                        break;
                    case PWD_CHG_STATE:

                        break;
                    case ARMED_STATE:

                        break;
                    case INTRUSION_STATE:

                        break;
                    case ALARM_STATE:

                        break;
                    }
                }
				TASK_ENABLE1=1;
                if (state != networkState)
                {
									OSMutexPend(netBufMutex, 0, &err);
									transmitBuffer.DLC = 1;
									OSMutexPend(idMutex, 0, &err);
									transmitBuffer.DATA[0] = id;
									OSMutexPost(idMutex);
									switch (state)
									{
										case INIT_STATE:
												// Don't send anything, not handled by the CAN network
                        break;
                    case DISARMED_STATE:
												transmitBuffer.SID = DISARMED_NET_PRIO;
												CanSendMessage();
                        break;
                    case PWD_CHG_STATE:
												transmitBuffer.SID = PWD_CHG_NET_PRIO;
												CanSendMessage();
                        break;
                    case ARMED_STATE:
												transmitBuffer.SID = ARMED_NET_PRIO;
												CanSendMessage();
                        break;
                    case INTRUSION_STATE:
												transmitBuffer.SID = INTRUSION_NET_PRIO;
												CanSendMessage();
                        break;
                    case ALARM_STATE:
												transmitBuffer.SID = ALARM_NET_PRIO;
												CanSendMessage();
                        break;
                    }
										OSMutexPost(netBufMutex);
									}
		OSMutexPost(stateMutex);
		OSTimeDly(500);
   	}
}

static  void  AlarmTask(void *p_arg)
{
	INT8U err;

	INT8U stateCpy;

	TASK_ENABLE1=0;

  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////


//	Infinite loop
/////////////////
  	while (1)
		{
								OSMutexPend(stateMutex, 0, &err);
								stateCpy=state;
								OSMutexPost(stateMutex);
								if (stateCpy == ALARM_STATE)
								{
									// launch the alarm code
								}
								else
								{
									// stop alarm code
								}
								OSTimeDly(ALARM_PERIOD);

   	}
}

static  void  ReadKbTask(void *p_arg)

{
	INT8U err;
	INT8U keyDetected, keyPressed, keyTooLong, charTyed;


  (void)p_arg;	// to avoid a warning message

	TASK_ENABLE1=0;

//	Initialisations
///////////////////
	KeyboardInit();
	keyDetected=255;
	keyPressed=255;

//	Infinite loop
/////////////////
    while (1)
		{
			keyTooLong=keyPressed;
			keyPressed=keyDetected;
			keyDetected=KeyboardScan();
			if ( (keyDetected == keyPressed) && (keyDetected != keyTooLong) && (keyDetected != 255) )
			{
				charTyed = hex2ASCII[keyDetected];
				err = OSMboxPost(charTypedMB, &charTyed);
			}

            OSTimeDly(READ_KB_PERIOD);
   	}
}

static  void  PwdCheckTask(void *p_arg)
{
	INT8U err;
	INT8U *newChar;
	int charCount;
	INT8U currentPwd[4] = {0};


  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
charCount=0;
TASK_ENABLE4=0;

//	Infinite loop
/////////////////
    while (1)
		{
			newChar = (INT8U*)OSMboxPend(charTypedMB, 0, &err);
			currentPwd[charCount] = *newChar;
			charCount++;
			if (charCount < 4) // password not completely typed
			{
				OSMboxPost(validationResultMB, (void*)&charCount);
			}
			else
			{
				if (currentPwd[0] == PASSWD0 && currentPwd[1] == PASSWD1 && currentPwd[2] == PASSWD2 && currentPwd[3] == PASSWD3) // password correct
				{
					charCount=0;
					OSMboxPost(validationResultMB, (void*)&charCount);
					// add MB to state machine validation
				}
				else // wrong password
				{
					charCount=-1;
					OSMboxPost(validationResultMB, (void*)&charCount);
					// add MB to state machine validation
				}
				charCount=0;
			}
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
                DispClrScr();
				OSMutexPend(stateMutex, 0, &err);
								switch (state)
								{
								case INIT_STATE:
										DispStr(0, 0, (CPU_INT08U*)"Initializing...");
										break;
								case DISARMED_STATE:
										DispStr(0, 0, (CPU_INT08U*)"Disarmed");
										break;
								case PWD_CHG_STATE:
										DispStr(0, 0, (CPU_INT08U*)"Insert a new");
										DispStr(1, 0, (CPU_INT08U*)"password");
										break;
								case ARMED_STATE:
										DispStr(0, 0, (CPU_INT08U*)"Armed");
										break;
								case INTRUSION_STATE:
										DispStr(0, 0, (CPU_INT08U*)"Intrusion");
										DispStr(1, 0, (CPU_INT08U*)"detected !");
										break;
								case ALARM_STATE:
										DispStr(0, 0, (CPU_INT08U*)"Alarm ringing");
										break;
								}
				OSMutexPost(stateMutex);
				OSTimeDly(STATE_DISP_REFRESH_PERIOD);
   	}
}

static  void  DispKbTask(void *p_arg)
{
	INT8U err;

	int *msg;

  (void)p_arg;	// to avoid a warning message

	TASK_ENABLE1=0;

//	Initialisations
///////////////////
        // Done in DispStateTask()


//	Infinite loop
/////////////////
    while (1)
		{
                msg = (int*)OSMboxPend(validationResultMB, 0, &err);
                DispClrScr();
								switch (*msg)
								{
								case -1:
										DispStr(0, 0, (CPU_INT08U*)"Wrong password");
										break;
								case 0:
										DispStr(0, 0, (CPU_INT08U*)"Password OK");
										break;
								case 1:
										DispStr(0, 0, (CPU_INT08U*)"*");
										break;
								case 2:
										DispStr(0, 0, (CPU_INT08U*)"**");
										break;
								case 3:
										DispStr(0, 0, (CPU_INT08U*)"***");
										break;
								}
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
		OSTaskSuspend(OS_PRIO_SELF);
   	}
}
