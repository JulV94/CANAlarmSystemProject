#include <includes.h>

extern OS_EVENT *stateMutex;
extern INT8U state;

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    INT8U err;
    IFS1bits.CNIF = 0;
    OSMutexPend(stateMutex, 0, &err);
    state=ALARM_STATE;
    OSMutexPost(stateMutex);
    LATAbits.LATA3 = !LATAbits.LATA3; // led test
}
