#include "p33FJ256GP710.h"
#include "speex.h"

int audio[]={
	#include "sound.audio"
};

void sendToDac(int echant)
{
	int j;
	int data = 0x3000 | (echant & 0x0FFF);	
	PORTBbits.RB2 = 0; // lach enable low -> pas de données dans le dac
	for(j=0; j<16; j++) //ecriture d'un bit
	{
		LATFbits.LATF6 = 0;
		if ( data & 0x8000 )
			LATFbits.LATF8 = 1;
        else
			LATFbits.LATF8 = 0;
		data = data<<1;
		LATFbits.LATF6 = 1;//coup d'horloge sur la patte Clk du DAC
	}
	PORTBbits.RB2 = 1; // latch enable 1, on prend en compte le DAC
}

void decodeSoundFrame(int* buf, int size)
{
	int i;
	
	for(i=0;i<size;i++)
	{
		buf[i] = audio[i] & 0x0FFF;
	}
}
