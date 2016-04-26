#include "CanDspic.h"

/********************************************************
*						DEFINITIONS						*
********************************************************/

//! Macro to assign a value to a CAN filter and activate it
#define _SetRXFnValue(f, val)   		\
	C1RXF##f##SID = val << 5L;			\
	C1FEN1bits.FLTEN##f = 1		

//! Macro to assign a value to a CAN mask
#define _SetRXMnValue(m, val)           \
	C1RXM##m##SID = val <<5L

//! Macro to get the current operation mode of the ECAN module 
# define CanGetOperationMode() 			C1CTRL1bits.OPMODE

// CAN Baud Rate Configuration
#define FCAN  	40000000 
#define NTQ 	20		// 20 Time Quanta in a Bit Time

/********************************************************
*						DECLARATIONS					*
********************************************************/

BUFFER_CAN receiveBuffers[7] 	__attribute__((space(dma),address(DMA_BASE_ADDRESS+0x0000)));
BUFFER_CAN transmitBuffer 		__attribute__((space(dma),address(DMA_BASE_ADDRESS+0x0070)));

void dma0init(void);
void dma1init(void);

/********************************************************
*						FUNCTIONS						*
********************************************************/

/****************** SEND ************************/

void CanSendMessage()
{
	// wait until transmit buffer is free	
	while(C1TR67CONbits.TXREQ7);

	// Request to Send
	C1TR67CONbits.TX7PRI1 = 1;
	C1TR67CONbits.TX7PRI0 = 1;	
	C1TR67CONbits.TXREQ7 = 1;
}

/****************** INITIALIZE *******************************/

void CanInitialisation(CAN_OP_MODE mode, CAN_BAUDRATE baudrate)
{
    // Put module into configuration mode
    CanSetOperationMode(CAN_OP_MODE_CONFIG);	

	// Set Bit rate values
	CanSetBaudRate(baudrate);
	
	// Configure first 7 TxRx Buffers as receive buffers, last buffer as transmit buffer
	C1TR01CON = 0x0000;
	C1TR23CON = 0x0000;
	C1TR45CON = 0x0000;
	C1TR67CON = 0x8000;
	
	// Configures the DMA buffer size to 12 buffers and makes FIFO start at buffer 16
	C1FCTRLbits.DMABS 	= 0b011;
	C1FCTRLbits.FSA 	= 0b10000;

	// Points the FIFO Write Buffer and Next FIFO Read Buffer to buffer 16
	C1FIFO = 0x1010;
		
	// Activates filter window to configure filter and mask registers
	C1CTRL1bits.WIN = 1;
	
	// Associates filters 0 -> 6 to RX buffer 0 -> 6
	C1BUFPNT1 = 0x3210;
	C1BUFPNT2 = 0xF654;
	C1BUFPNT3 = 0xFFFF;
	C1BUFPNT4 = 0xFFFF;

	// Deactivates all acceptance filters 
	C1FEN1 = 0x0000;
		
	// Associates mask 0 to all filters
	C1FMSKSEL1 = 0x0000;
	C1FMSKSEL2 = 0x0000;

	// Deactivates filter window
	C1CTRL1bits.WIN = 0;

	// Enable receive interrupt only
	C1INTEbits.RBIE = 0x1;

	// Initializes DMA channels to send and receive CAN messages
	dma0init();
	dma1init();

	// Put module into normal mode
	CanSetOperationMode(mode);
}

/****************** MASK FILTER ******************************/
void CanAssociateMaskFilter(unsigned char mask, unsigned char filter)
{
	// Saves the current operation mode
	CAN_OP_MODE currentMode = CanGetOperationMode();
	
	// Switch to configuration mode
	CanSetOperationMode(CAN_OP_MODE_CONFIG);

	// Activates filter window to configure filter and mask registers
	C1CTRL1bits.WIN = 1;
	
	if (mask >= 0 && mask <= 2)
	{
		switch(filter)
		{
			case 0:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1111111111111100) | mask;
				break;
			case 1:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1111111111110011) | (mask<<2);
				break;
			case 2:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1111111111001111) | (mask<<4);
				break;
			case 3:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1111111100111111) | (mask<<6);
				break;
			case 4:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1111110011111111) | (mask<<8);
				break;
			case 5:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1111001111111111) | (mask<<10);
				break;
			case 6:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b1100111111111111) | (mask<<12);
				break;
			case 7:
				C1FMSKSEL1 = (C1FMSKSEL1 & 0b0011111111111111) | (mask<<14);
				break;
			case 8:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1111111111111100) | mask;
				break;
			case 9:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1111111111110011) | (mask<<2);
				break;
			case 10:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1111111111001111) | (mask<<4);
				break;
			case 11:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1111111100111111) | (mask<<6);
				break;
			case 12:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1111110011111111) | (mask<<8);
				break;
			case 13:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1111001111111111) | (mask<<10);
				break;
			case 14:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b1100111111111111) | (mask<<12);
				break;
			case 15:
				C1FMSKSEL2 = (C1FMSKSEL2 & 0b0011111111111111) | (mask<<14);
				break;
			default:
				break;		
		}
	}	
	// Deactivates filter window
	C1CTRL1bits.WIN = 0;
	
	// Restores the previous operation mode
	CanSetOperationMode(currentMode);
}

/****************** CHANGE MASK ******************************/
void CanLoadMask(unsigned char number, unsigned int mask)
{
	// Saves the current operation mode
	CAN_OP_MODE currentMode = CanGetOperationMode();
	
	// Switch to configuration mode
	CanSetOperationMode(CAN_OP_MODE_CONFIG);

	// Activates filter window to configure filter and mask registers
	C1CTRL1bits.WIN = 1;
	
	switch(number)
	{
		case 0: 
			C1RXM0SID = (mask << 5) | 0b0000000000001000;
			break;
		case 1:
			C1RXM1SID = (mask << 5) | 0b0000000000001000;
			break;
		case 2:
			C1RXM2SID = (mask << 5) | 0b0000000000001000;
			break;
		default:
			break;
	}

	// Deactivates filter window
	C1CTRL1bits.WIN = 0;
	
	// Restores the previous operation mode
	CanSetOperationMode(currentMode);
}


/****************** SET OPERATION MODE ***********************/

void CanSetOperationMode(CAN_OP_MODE mode)
{
	// Clear all pending transmissions
    C1CTRL1bits.ABAT = 1;    

	// Request Configuration Mode
	C1CTRL1bits.REQOP = mode;
	                
	// Wait till desired mode is set
	while(C1CTRL1bits.OPMODE != mode);  
}

/****************** SET BAUDRATE *****************************/

void CanSetBaudRate(CAN_BAUDRATE baudrate)
{
	// Saves the current operation mode
	CAN_OP_MODE currentMode = CanGetOperationMode();	
	
	// Switch to configuration mode
	CanSetOperationMode(CAN_OP_MODE_CONFIG);

	// FCAN is selected to be FCY = 40MHz
	C1CTRL1bits.CANCKS = 0x1;
		
	/*
	Bit Time = (Sync Segment + Propagation Delay + Phase Segment 1 + Phase Segment 2)=20*TQ
	Phase Segment 1 = 8TQ
	Phase Segment 2 = 6Tq
	Propagation Delay = 5Tq
	Sync Segment = 1TQ
	CiCFG1<BRP> =(FCAN /(2 ×N×FBAUD))– 1
	*/
		
	/* Synchronization Jump Width set to 4 TQ */
	C1CFG1bits.SJW = 0x3;
	
	/* Baud Rate Prescaler */
	C1CFG1bits.BRP = ((FCAN/(2*NTQ*baudrate))-1);
	
	/* Phase Segment 1 time is 8 TQ */
	C1CFG2bits.SEG1PH=0x7;
	/* Phase Segment 2 time is set to be programmable */
	C1CFG2bits.SEG2PHTS = 0x1;
	/* Phase Segment 2 time is 6 TQ */
	C1CFG2bits.SEG2PH = 0x5;
	/* Propagation Segment time is 5 TQ */
	C1CFG2bits.PRSEG = 0x4;
	/* Bus line is sampled three times at the sample point */
	C1CFG2bits.SAM = 0x1;

	// Restores the previous operation mode
	CanSetOperationMode(currentMode);					
}

/****************** LOAD FILTER ***************************/

void CanLoadFilter(unsigned char numero, unsigned int id)
{
	// Saves the current operation mode
	CAN_OP_MODE currentMode = CanGetOperationMode();	
	
	// Switch to configuration mode
	CanSetOperationMode(CAN_OP_MODE_CONFIG);

	// Activates filter window to configure filter and mask registers
	C1CTRL1bits.WIN = 1;

	switch (numero)
	{
		case 0 : _SetRXFnValue(0, id); C1FEN1 |= 0x0001; break;
		case 1 : _SetRXFnValue(1, id); C1FEN1 |= 0x0002; break;
		case 2 : _SetRXFnValue(2, id); C1FEN1 |= 0x0004; break;
		case 3 : _SetRXFnValue(3, id); C1FEN1 |= 0x0008; break;
		case 4 : _SetRXFnValue(4, id); C1FEN1 |= 0x0010; break;
		case 5 : _SetRXFnValue(5, id); C1FEN1 |= 0x0020; break;
		case 6 : _SetRXFnValue(6, id); C1FEN1 |= 0x0040; break;
		case 7 : _SetRXFnValue(7, id); C1FEN1 |= 0x0080; break;
		case 8 : _SetRXFnValue(8, id); C1FEN1 |= 0x0100; break;
		case 9 : _SetRXFnValue(9, id); C1FEN1 |= 0x0200; break;
		case 10 : _SetRXFnValue(10, id); C1FEN1 |= 0x0400; break;
		case 11 : _SetRXFnValue(11, id); C1FEN1 |= 0x0800; break;
		case 12 : _SetRXFnValue(12, id); C1FEN1 |= 0x1000; break;
		case 13 : _SetRXFnValue(13, id); C1FEN1 |= 0x2000; break;
		case 14 : _SetRXFnValue(14, id); C1FEN1 |= 0x4000; break;			
		case 15 : _SetRXFnValue(15, id); C1FEN1 |= 0x8000; break;
		default : break;
	}
	
	// Deactivates filter window
	C1CTRL1bits.WIN = 0;

	// Restores the previous operation mode
	CanSetOperationMode(currentMode);	
}

/****************** Dma Initialization for TX ****************/

void dma0init(void){

	DMACS0=0;						// Clear DMA Status
    
	DMA0CONbits.SIZE = 0x0; 		// Data Transfer Size: Word Transfer Mode
	DMA0CONbits.DIR = 0x1; 			// Data Transfer Direction: DMA RAM to Peripheral  
	DMA0CONbits.HALF = 0x0;			// Initiate interrupt when all data has been transfered	 
	DMA0CONbits.NULLW = 0x0;		// Normal operation (no NULL data write)
	DMA0CONbits.AMODE = 0x2; 		// DMA Addressing Mode: Peripheral Indirect Addressing mode
	DMA0CONbits.MODE = 0x0; 		// Operating Mode: Continuous, Ping-Pong modes disabled
	
	DMA0REQ = 70; 					// Assign ECAN1 Transmit event for DMA Channel 0 
	
	DMA0PAD = &C1TXD; 				// Peripheral Address: ECAN1 Transmit Register 
	
	DMA0CNT = 7; 					// Set Number of DMA Transfer per ECAN message to 8 words  
 	 
 	DMA0STA = 0x0070;  				// Start Address Offset for ECAN1 Message Buffer 0x0000 
	//DMA0STA=  __builtin_dmaoffset(ecan1msgBuf);	
	
	DMA0CONbits.CHEN = 0x1; 		// Channel Enable: Enable DMA Channel 0 
	IEC0bits.DMA0IE = 1;			// Channel Interrupt Enable: Enable DMA Channel 0 Interrupt  

}


/****************** Dma Initialization for RX ****************/

void dma1init(void){

	DMACS1=0;						// Clear DMA Status
    
	DMA1CONbits.SIZE = 0x0; 		// Data Transfer Size: Word Transfer Mode
	DMA1CONbits.DIR = 0x0; 			// Data Transfer Direction: Peripheral to DMA RAM
	DMA1CONbits.HALF = 0x0;			// Initiate interrupt when all data has been transfered	 
	DMA1CONbits.NULLW = 0x0;		// Normal operation (no NULL data write)
	DMA1CONbits.AMODE = 0x2; 		// DMA Addressing Mode: Peripheral Indirect Addressing mode
	DMA1CONbits.MODE = 0x0; 		// Operating Mode: Continuous, Ping-Pong modes disabled
	
	DMA1REQ = 34; 					// Assign ECAN1 Receive event for DMA Channel 1 
	
	DMA1PAD = &C1RXD; 				// Peripheral Address: ECAN1 Transmit Register 
	
	DMA1CNT = 7; 					// Set Number of DMA Transfer per ECAN message to 8 words  
 	 
 	DMA1STA = 0x0000;  				// Start Address Offset for ECAN1 Message Buffer 0x0000 
	// ATTENTION 80 ???
	//DMA0STA=  __builtin_dmaoffset(ecan1msgBuf);	
	
	DMA1CONbits.CHEN = 0x1; 		// Channel Enable: Enable DMA Channel 0 
	IEC0bits.DMA1IE = 1;			// Channel Interrupt Enable: Enable DMA Channel 0 Interrupt  

}

/****************** Dma Interrupt handlers *******************/

void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void)
{
   IFS0bits.DMA0IF = 0;          // Clear the DMA0 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void)
{
   IFS0bits.DMA1IF = 0;          // Clear the DMA1 Interrupt Flag;
}
