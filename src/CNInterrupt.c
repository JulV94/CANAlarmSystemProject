#include <includes.h>

extern OS_EVENT* buttonPressedSM;

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    INT8U err;
    IFS1bits.CNIF = 0;
	OSSemPost(buttonPressedSM);
    LATAbits.LATA3 = !LATAbits.LATA3; // led test
}
