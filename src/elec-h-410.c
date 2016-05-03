/*
 *  elec-h-410.c
 *  
 *
 *  Created by Geoffrey on 31/01/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "elec-h-410.h"
#include <includes.h>


void init_elec_h_410(void)
{
	DIO1_TRIS = 0;
	DIO2_TRIS = 0;
	DIO3_TRIS = 0;
	DIO4_TRIS = 0;
	DIO5_TRIS = 0;
	DIO6_TRIS = 0;
	DIO7_TRIS = 0;
	DIO8_TRIS = 0;

	TASK_ENABLE_TRIS1 = 0;
	TASK_ENABLE_TRIS2 = 0;
	TASK_ENABLE_TRIS3 = 0;
	TASK_ENABLE_TRIS4 = 0;
	TASK_ENABLE_TRIS5 = 0;
	TASK_ENABLE_TRIS6 = 0;
	TASK_ENABLE_TRIS7 = 0;
	TASK_ENABLE_TRIS8 =	0;
	//TASK_ENABLE_TRIS9 = 0;
	//TASK_ENABLE_TRIS10 = 0;
}

