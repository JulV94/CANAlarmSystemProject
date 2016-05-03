/*
 *  elec-h-410.h
 *  
 *
 *  Created by Geoffrey on 31/01/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ELEC_H_410_H
#define ELEC_H_410_H

#define DIO1_TRIS	TRISCbits.TRISC1
#define DIO2_TRIS	TRISCbits.TRISC2
#define DIO3_TRIS	TRISCbits.TRISC3
#define DIO4_TRIS	TRISCbits.TRISC4
#define DIO5_TRIS	TRISGbits.TRISG12
#define DIO6_TRIS	TRISGbits.TRISG13
#define DIO7_TRIS	TRISGbits.TRISG14
#define DIO8_TRIS	TRISGbits.TRISG15


	
#define TASK_ENABLE1		LATFbits.LATF7
#define TASK_ENABLE2		LATFbits.LATF8
#define TASK_ENABLE3		LATFbits.LATF6
#define TASK_ENABLE4		LATBbits.LATB2
#define TASK_ENABLE5		LATDbits.LATD0
#define TASK_ENABLE6		LATDbits.LATD1
#define TASK_ENABLE7		LATDbits.LATD2
#define TASK_ENABLE8		LATDbits.LATD3
#define TASK_ENABLE9		LATAbits.LATA12
#define TASK_ENABLE10		LATAbits.LATA13

#define TASK_ENABLE_TRIS1		TRISFbits.TRISF7
#define TASK_ENABLE_TRIS2		TRISFbits.TRISF8
#define TASK_ENABLE_TRIS3		TRISFbits.TRISF6
#define TASK_ENABLE_TRIS4		TRISBbits.TRISB2
#define TASK_ENABLE_TRIS5		TRISDbits.TRISD0
#define TASK_ENABLE_TRIS6		TRISDbits.TRISD1
#define TASK_ENABLE_TRIS7		TRISDbits.TRISD2
#define TASK_ENABLE_TRIS8		TRISDbits.TRISD3
//#define TASK_ENABLE_TRIS9		TRISAbits.TRISA12
//#define TASK_ENABLE_TRIS10		TRISAbits.TRISA13


void init_elec_h_410(void);

#endif
