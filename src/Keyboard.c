#include <includes.h>

void KeyboardInit(void)
{
	TRISD &= 0xF0FF;//colonnes du clavier en output
	//lignes du clavier en input
	AD1PCFGL |= 0x0F00;
	AD2PCFGL |= 0x0F00;
	TRISB |=0x0F00;
}

INT8U KeyboardScan( void )
{
	INT8U res;
	INT8U tmp;
	
	res = 255;
	//colonne 1
	LATD = (LATD & 0xF0FF)| 0x0100;
	//OSTimeDly (1);					//mise en sommeil pendant 1 coup d'horloge
	for (tmp=0;tmp<255;tmp++);
	switch(PORTB & 0x0F00){
		case 0x0100 : res = 1; break;
		case 0x0200 : res = 4; break;
		case 0x0400 : res = 7; break;
		case 0x0800 : res = 10; break;
	}
	//colonne 2
	LATD = (LATD & 0xF0FF)| 0x0200;
	//OSTimeDly (1);					//mise en sommeil pendant 1 coup d'horloge
	for (tmp=0;tmp<255;tmp++);
	switch(PORTB & 0x0F00){
		case 0x0100 : res = 2; break;
		case 0x0200 : res = 5; break;
		case 0x0400 : res = 8; break;
		case 0x0800 : res = 0; break;
	}
	//colonne 3
	LATD = (LATD & 0xF0FF)| 0x0400;
	//OSTimeDly (1);					//mise en sommeil pendant 1 coup d'horloge
	for (tmp=0;tmp<255;tmp++);
	switch(PORTB & 0x0F00){
		case 0x0100 : res = 3; break;
		case 0x0200 : res = 6; break;
		case 0x0400 : res = 9; break;
		case 0x0800 : res = 11; break;
	}
	//colonne 4
	LATD = (LATD & 0xF0FF)| 0x0800;
	//OSTimeDly (1);					//mise en sommeil pendant 1 coup d'horloge
	for (tmp=0;tmp<255;tmp++);
	switch(PORTB & 0x0F00){
		case 0x0100 : res = 15; break;
		case 0x0200 : res = 14; break;
		case 0x0400 : res = 13; break;
		case 0x0800 : res = 12; break;
	}
	return res;
}
