#ifndef MAIN_H
#define MAIN_H

/* comment the below line for release */
//#define TESTING

#include "includes.h"

#ifdef TESTING
	#define PhNum	 "9980237552"
#else
	#define PhNum	 "9742449675"
#endif

#define CONSTANT_GAIN			0.0131
#define REF_VALUE				670

#define MSG_WAIT_MSG			1
#define MSG_PH_NUM				2
#define MSG_COLL_MSG			3
#define MSG_RCV_MSG				4

#define LINE_FEED				0x0A

struct  {
	volatile int8u Fuel:1;
	volatile int8u Meas:1;
	volatile int8u msg:1;
}AppFlags;


//DEFINE MACROS
#define StartTmr()			TCCR0  	|= _BV(CS01)
#define StopTmr()			TCCR0  	&= ~_BV(CS01)

#define EnUARTInt()					UCSRB |= _BV(RXCIE); UCSRA |= _BV(RXC)
#define DisUARTInt()				UCSRB &= ~_BV(RXCIE); UCSRA &= ~_BV(RXC)

//FUNCTION PROTOTYPES
static void		init		(void);
static void 	disptitl 	(void);
static void 	tmr1init	(void);
static void		EXTINTinit	(void);
static void		sendmsg		(void);
static float	readfuel	(void);
static void		FillFuel	(void);
static int8u	checkmsg	(void);

#endif
