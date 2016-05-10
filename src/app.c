
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

#define  HEARTBEAT_CHECK_TASK_STK_SIZE		768
OS_STK	HeartbeatCheckTaskStk[HEARTBEAT_CHECK_TASK_STK_SIZE];

#define  HEARTBEAT_TASK_STK_SIZE			128
OS_STK	HeartbeatTaskStk[HEARTBEAT_TASK_STK_SIZE];

#define  TAKE_ID_TASK_STK_SIZE				256
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

#define  DAC_TASK_STK_SIZE					128
OS_STK	DacTaskStk[DAC_TASK_STK_SIZE];

#define  SPEEX_TASK_STK_SIZE				512
OS_STK	SpeexTaskStk[SPEEX_TASK_STK_SIZE];

// Semaphores
OS_EVENT *HBMissmatchSM;
OS_EVENT *buttonPressedSM;
OS_EVENT *intrusionTmrSM;
OS_EVENT *soundBuf0SM;
OS_EVENT *soundBuf1SM;

// Mailboxes
OS_EVENT *idListMB;
OS_EVENT *charTypedMB;
OS_EVENT *validationResultMB;
OS_EVENT *pwdValidityMB;

// Queues
OS_EVENT *allCANMsgInQueue;
INT8U *allCANMsgInArray[200];

// Mutex
OS_EVENT *netBufMutex;
OS_EVENT *idMutex;
OS_EVENT *stateMutex;

// Timer
OS_TMR *intrusionTmr;

// Global variables
INT8U id;
INT8U state;

// Global without Mutex
INT8U CANSendState;
INT8U showPWD;
INT8U playSound;

// Buffers
int	soundBuf0[SOUND_BUF_SIZE];
int	soundBuf1[SOUND_BUF_SIZE];


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
static  void  DacTask(void *p_arg);
static  void  SpeexTask(void *p_arg);
static  void  IntrusionTmrFct(void *p_tmr, void *p_arg);
static  int   sameArrays(int a[], int b[], int size);

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
	HBMissmatchSM = OSSemCreate(0);
	buttonPressedSM = OSSemCreate(0);
	intrusionTmrSM = OSSemCreate(0);
	soundBuf0SM = OSSemCreate(0);
	soundBuf1SM = OSSemCreate(0);

	// create mailbox
	idListMB = OSMboxCreate((void *)0);
	charTypedMB = OSMboxCreate((void *)0);
	validationResultMB = OSMboxCreate((void *)0);
	pwdValidityMB = OSMboxCreate((void *)0);

	// create queues
	allCANMsgInQueue = OSQCreate((void*)&allCANMsgInArray[0], 200);

	// create Mutex
	netBufMutex = OSMutexCreate(11, &err); // All task created can use the mutex
	idMutex = OSMutexCreate(11, &err);
	stateMutex = OSMutexCreate(11, &err);

	// enable interruptions button
	_CN15IE = 1;		//enable alarm switch
	TRISAbits.TRISA3 = 0; //led
	TRISAbits.TRISA2 = 0;
	LATAbits.LATA2 = 1;
	TRISDbits.TRISD6 = 1; //alarm switch interrupt
	IFS1bits.CNIF = 0;
	IEC1bits.CNIE = 1;            // Enable CN interrupts

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

		OSTaskCreateExt(
			DacTask,
			(void *)0,
			(OS_STK *)&DacTaskStk[0],
			DAC_PRIO,
			DAC_PRIO,
			(OS_STK *)&DacTaskStk[DAC_TASK_STK_SIZE-1],
			DAC_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

		// defines the App Name (for debug purpose)
    OSTaskNameSet(DAC_PRIO, (CPU_INT08U *)"Dac Decode Task", &err);

		OSTaskCreateExt(
			SpeexTask,
			(void *)0,
			(OS_STK *)&SpeexTaskStk[0],
			SPEEX_PRIO,
			SPEEX_PRIO,
			(OS_STK *)&SpeexTaskStk[SPEEX_TASK_STK_SIZE-1],
			SPEEX_TASK_STK_SIZE,
			(void *)0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

		// defines the App Name (for debug purpose)
    OSTaskNameSet(SPEEX_PRIO, (CPU_INT08U *)"Speex Task", &err);

	intrusionTmr = OSTmrCreate( 0,INTRUSION_TMR_TIME,  
									 //0, 
									 OS_TMR_OPT_PERIODIC, 
									 IntrusionTmrFct, 
									 (void *)0, 
									 "Intrusion timer", 
									 &err);

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
	int nodesTTL[MAX_NODES] = {0};
	int fixedIdList[MAX_NODES];
	INT8U *msgInQueue;
	INT32U endTime=OSTimeGet();
	INT32U time;
	int i, j, sizeFixedList, size, multiple=0;
	static INT8U HBMissmatchBuf;

//	Infinite loop
/////////////////
    while (42)
		{
			time = /*4294967295-*/abs(OSTimeGet()-endTime);
			for (i=0; i<MAX_NODES; i++)
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
		if (stateCpy == INIT_STATE || stateCpy == DISARMED_STATE || stateCpy == PWD_CHG_STATE)
		{
			// update fixed list
			Mem_Set((void*)&fixedIdList[0], -1, sizeof(int)*MAX_NODES);
			sizeFixedList=0;
			for (i=0; i<MAX_NODES; i++)
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
			for (i=0; i<MAX_NODES; i++)
			{
				size=0;
				if (nodesTTL[i] > 0)
				{
					OSMutexPend(idMutex, 0, &err);
					if (size > sizeFixedList || fixedIdList[size] != i || nodesTTL[i] == id)
					{
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
			OSTimeDly(HEARTBEAT_PERIOD-(magicNumber-abs(OSTimeGet()-timeStart))); // 2^32-1
   	}
}

static  void  TakeIdTask(void *p_arg)
{
	INT8U err;

	int *idList;
	int oldIdList[MAX_NODES] = {-1};
	int i;

  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	CANSendState=INIT_STATE;
	OSMutexPend(stateMutex, 0, &err);
	state=INIT_STATE;
	OSMutexPend(idMutex, 0, &err);
	id = rand()%MAX_NODES;
	OSMutexPost(idMutex);
	idList = (int*)OSMboxPend(idListMB, 0, &err);
	do {
		i=0;
		while (idList[i] != -1)
		{
			if (idList[i] == id)
			{
				OSMutexPend(idMutex, 0, &err);
				id = rand()%MAX_NODES;
				OSMutexPost(idMutex);
				i=-1;
			}
			i++;
		}
		memcpy(oldIdList, idList, sizeof(int)*MAX_NODES);
		idList = (int*)OSMboxPend(idListMB, 0, &err);
	}while (!sameArrays(idList, oldIdList, MAX_NODES));
  	state = DISARMED_STATE;
	OSMutexPost(stateMutex);
	CANSendState=DISARMED_STATE;

//	Infinite loop
/////////////////
    while (1)
		{
			OSTaskSuspend(OS_PRIO_SELF);
   	}
}

static  void  StateMachineTask(void *p_arg)
{
	INT8U err, previousState, netState, wrongPwdCount, tmpState, pushStateToNet;
	INT8U *pwdResult;

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
	//ACTIVATE_CAN_INTERRUPTS = 1;

	wrongPwdCount = 0;

//	Infinite loop
/////////////////
  	while (1)
		{
				OSMutexPend(stateMutex, 0, &err);
				pushStateToNet = 0;
				if (OSSemAccept(buttonPressedSM) && state == ARMED_STATE)
				{
					state = INTRUSION_STATE;
					pushStateToNet = 1;
				}
				pwdResult = (INT8U*)OSMboxAccept(pwdValidityMB);
				if (*pwdResult == PWD_OK)
				{
					tmpState = state;
					switch (tmpState)
					{
						case DISARMED_STATE:
							state = ARMED_STATE;
							break;
						case ARMED_STATE:
						case INTRUSION_STATE:
						case ALARM_STATE:
							state = DISARMED_STATE;
							break;
					}
					pushStateToNet = 1;
				}
				else if (*pwdResult == PWD_WRONG && (state == ARMED_STATE || state == INTRUSION_STATE || state == ALARM_STATE))
				{
					wrongPwdCount++;
				}
				if (wrongPwdCount > 2 && (state == ARMED_STATE || state == INTRUSION_STATE))
				{
					state = ALARM_STATE;
					pushStateToNet = 1;
				}
				netState = CANSendState; // to avoid Mutex and interruptions together
                if (OSSemAccept(HBMissmatchSM) && (state == ARMED_STATE || state == INTRUSION_STATE))
                {
                    pushStateToNet = 1;
					state = ALARM_STATE;
                }
				if (netState != state && !pushStateToNet && !OSSemAccept(intrusionTmrSM))
                {
                    state = netState;
                }
                if (state != previousState)
                {
                    switch (state)
                    {
                    case INIT_STATE:
						// nothing to do (§never happens normally)
                        break;
                    case DISARMED_STATE:
						wrongPwdCount = 0;
                        break;
                    case PWD_CHG_STATE:
						// not implemented
                        break;
                    case ARMED_STATE:
						// nothing to do
                        break;
                    case INTRUSION_STATE:
						OSTmrStart(intrusionTmr, &err);
                        break;
                    case ALARM_STATE:
						// nothing to do
                        break;
                    }
                }
                if (state != netState)
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
										CANSendState = state;
									}
		previousState = state;
		OSMutexPost(stateMutex);
		OSTimeDly(500);
   	}
}

static  void  AlarmTask(void *p_arg)
{
	INT8U err;

	INT8U stateCpy;

  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	playSound = 0;

//	Infinite loop
/////////////////
  	while (1)
		{
								OSMutexPend(stateMutex, 0, &err);
								stateCpy=state;
								OSMutexPost(stateMutex);
								playSound = (stateCpy == ALARM_STATE);
								OSTimeDly(ALARM_PERIOD);

   	}
}

static  void  ReadKbTask(void *p_arg)

{
	INT8U err;
	INT8U keyDetected, keyPressed, keyTooLong, charTyed;


  (void)p_arg;	// to avoid a warning message

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
	INT8U result;
	int charCount, dispMsg;
	INT8U currentPwd[4] = {0};


  (void)p_arg;	// to avoid a warning message


//	Initialisations
///////////////////
	charCount = 0;
	dispMsg = 0;
	showPWD = 0;

//	Infinite loop
/////////////////
    while (1)
		{
			newChar = (INT8U*)OSMboxPend(charTypedMB, 0, &err);
			showPWD = 1;
			currentPwd[charCount] = *newChar;
			charCount++;
			if (charCount < 4) // password not completely typed
			{
				dispMsg = charCount;
				OSMboxPost(validationResultMB, (void*)&dispMsg);
			}
			else
			{
				showPWD = 0;
				if (currentPwd[0] == PASSWD0 && currentPwd[1] == PASSWD1 && currentPwd[2] == PASSWD2 && currentPwd[3] == PASSWD3) // password correct
				{
					charCount=0;
					dispMsg = 0;
					result = PWD_OK;
					OSMboxPost(validationResultMB, (void*)&dispMsg);
					OSMboxPost(pwdValidityMB, (void*)&result);
				}
				else // wrong password
				{
					charCount=-1;
					dispMsg = -1;
					result = PWD_WRONG;
					OSMboxPost(validationResultMB, (void*)&dispMsg);
					OSMboxPost(pwdValidityMB, (void*)&result);
				}
				charCount=0;
			}
   	}
}

static  void  DispStateTask(void *p_arg)
{
	INT8U err;

  (void)p_arg;	// to avoid a warning message
	
	char idHexStr[3];


//	Initialisations
///////////////////

	DispInit(2, 16);
	DispClrScr();

//	Infinite loop
/////////////////
    while (1)
		{
				if (!showPWD)
				{
					OSTimeDly(PWD_RESULT_DISP_TIME);
					DispClrScr();
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
					sprintf(&idHexStr[0], "%02X", id);
					DispStr(1, 14, &idHexStr);
				}
				OSTimeDly(STATE_DISP_REFRESH_PERIOD-PWD_RESULT_DISP_TIME);
   	}
}

static  void  DispKbTask(void *p_arg)
{
	INT8U err;

	int *msg;

  (void)p_arg;	// to avoid a warning message

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

static  void  DacTask(void *p_arg)
{
	int i,j;
	int data;
	INT8U err;
	

	// Initialisation du DAC
	TRISBbits.TRISB2=0; //SPI
	TRISFbits.TRISF6=0;
	TRISFbits.TRISF8=0;
	
	OSSemPend(soundBuf0SM, 0, &err); /// pend from alarm task
	while(1)
	{
		if (playSound)
		{
			for (i=0; i<SOUND_BUF_SIZE; i++)
			{
				sendToDac(soundBuf0[i]);
				OSTimeDlyHMSM(0, 0, 0, 10);
			}
			OSSemPend(soundBuf0SM, 0, &err);
			OSSemPost(soundBuf1SM);
		
			for (i=0; i<SOUND_BUF_SIZE; i++)
			{
				TASK_ENABLE1 = 0;
				sendToDac(soundBuf1[i]);
				TASK_ENABLE1 = 1;
				OSTimeDlyHMSM(0, 0, 0, 10);
			}
			TASK_ENABLE1 = 1;
			OSSemPend(soundBuf0SM, 0, &err);
			OSSemPost(soundBuf1SM);
		}
	else
		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}

static  void  SpeexTask(void *p_arg)
{	 
	INT8U err;
	INT32U	clkValue;

   (void)p_arg;			// to avoid a warning message


//	Initialisations
///////////////////
	decodeSoundFrame(&soundBuf0[0], SOUND_BUF_SIZE);
	OSSemPost(soundBuf0SM);

//	Infinite loop
/////////////////
	while(1)
	{
		decodeSoundFrame(&soundBuf1[0], SOUND_BUF_SIZE);
		OSSemPost(soundBuf0SM);
		OSSemPend(soundBuf1SM, 0, &err);
		decodeSoundFrame(&soundBuf0[0], SOUND_BUF_SIZE);
		OSSemPost(soundBuf0SM);
		OSSemPend(soundBuf1SM, 0, &err);
	}
}

static void IntrusionTmrFct(void *p_tmr, void *p_arg)
{
	INT8U err;
	OSMutexPend(stateMutex, 0, &err);
	state=ALARM_STATE;
	err = OSSemPost(intrusionTmrSM);
	OSMutexPost(stateMutex);
	OSTmrStop(intrusionTmr, OS_TMR_OPT_NONE, (void *)0, &err);
}

static  int  sameArrays(int a[], int b[], int size)
{
	int i;
	for (i=0; i<size; i++)
	{
		if (a[i] != b[i]) return 0;
	}
	return 1;
}