#ifndef _CANDSPIC_H
#define _CANDSPIC_H
/********************************************************
*						HEADERS							*
********************************************************/

#include "p33fxxxx.h"

/********************************************************
*						DEFINITIONS						*
********************************************************/

#define		ACTIVATE_CAN_INTERRUPTS			IEC2bits.C1IE			// Defines the bit that activates interrupts for Can
#define		CAN_INTERRUPT_FLAG				IFS2bits.C1IF			// Defines the bit containing the flag of any CAN interrupt
#define 	CAN_RX_BUFFER_IF				C1INTFbits.RBIF			// Defines the bit containing the reception flag of any CAN message
#define		CAN_RX_BUFFER_0					C1RXFUL1bits.RXFUL0		// Defines the bit containing the reception flag of a CAN message in buffer 0
#define		CAN_RX_BUFFER_1					C1RXFUL1bits.RXFUL1		// Defines the bit containing the reception flag of a CAN message in buffer 1
#define		CAN_RX_BUFFER_2					C1RXFUL1bits.RXFUL2		// Defines the bit containing the reception flag of a CAN message in buffer 2
#define		CAN_RX_BUFFER_3					C1RXFUL1bits.RXFUL3		// Defines the bit containing the reception flag of a CAN message in buffer 3
#define		CAN_RX_BUFFER_4					C1RXFUL1bits.RXFUL4		// Defines the bit containing the reception flag of a CAN message in buffer 4
#define		CAN_RX_BUFFER_5					C1RXFUL1bits.RXFUL5		// Defines the bit containing the reception flag of a CAN message in buffer 5
#define		CAN_RX_BUFFER_6					C1RXFUL1bits.RXFUL6		// Defines the bit containing the reception flag of a CAN message in buffer 6

#define 	DMA_BASE_ADDRESS				0x7800

/********************************************************
*						VARIABLES						*
********************************************************/

//! CAN Module baudrates
typedef enum _CAN_BAUDRATE
{
	CAN_BAUDRATE_1M		= 1000000,		/*!< 1Mbps	*/
    CAN_BAUDRATE_500k	= 500000,		/*!< 500kpbs */
	CAN_BAUDRATE_250k	= 250000,		/*!< 250kbps */
	CAN_BAUDRATE_125k	= 125000		/*!< 125kbps */
} CAN_BAUDRATE;

//! CAN Module operation modes
typedef enum _CAN_OP_MODE
{
    CAN_OP_MODE_NORMAL  	= 0x00,		/*!< Normal mode 				*/
    CAN_OP_MODE_DISABLE 	= 0x01,		/*!< Disable mode 				*/
    CAN_OP_MODE_LOOP    	= 0x02,		/*!< Loopback mode				*/ 
    CAN_OP_MODE_LISTEN_ONLY = 0x03,		/*!< Listen-only mode			*/
    CAN_OP_MODE_CONFIG  	= 0x04,		/*!< Configuration mode			*/
    CAN_OP_MODE_LISTEN_ALL 	= 0x07		/*!< Listen all messages mode	*/
} CAN_OP_MODE;

//! Message CAN structure
typedef union _BUFFER_CAN
{
    unsigned int u16Words [8];
    struct{
        unsigned IDE:1;
        unsigned SRR:1;        
        unsigned SID:11;
        unsigned UNUSED0:3;
        unsigned EID17_6:12;
        unsigned UNUSED1:4;
        unsigned DLC:4;
        unsigned RB0:1;
        unsigned UNUSED2:3;
        unsigned RB1:1;
        unsigned RTR:1;
        unsigned EID5_0:6;
        unsigned char DATA[8];
        unsigned char UNUSED4;
        unsigned FILHIT:5;  
        unsigned UNUSED3:3;        
    };
} BUFFER_CAN;

//! Transmission and reception buffers
extern BUFFER_CAN receiveBuffers[7] 	__attribute__((space(dma),address(DMA_BASE_ADDRESS+0x0000)));
extern BUFFER_CAN transmitBuffer 		__attribute__((space(dma),address(DMA_BASE_ADDRESS+0x0070)));

/********************************************************
*						PROTOTYPES						*
********************************************************/

//! Can initialisation : requires operation mode and baudrate desired at the end of initialisation
void CanInitialisation(CAN_OP_MODE mode, CAN_BAUDRATE baudrate);

//! Changes the Can Baudrate
void CanSetBaudRate(CAN_BAUDRATE baudrate);

//! Changes the operation mode of the CAN Module
void CanSetOperationMode(CAN_OP_MODE mode);

//! Activates a filter with the corresponding id
void CanLoadFilter(unsigned char numero, unsigned int id);

//! Sends the message stored in the transmit buffer
void CanSendMessage();

//! Loads a mask with the corresponding id
void CanLoadMask(unsigned char number, unsigned int mask);

//! Associates a mask to a filter
void CanAssociateMaskFilter(unsigned char mask, unsigned char filter);

#endif



