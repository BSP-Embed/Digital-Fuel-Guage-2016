#include "main.h"

extern int8u lcdptr;

int8u phnum[15];
int8u sbuf[100];
float FuelAdded;
int Amount;

int main(void)
{
	char tmpstr[10];
	
	init();
	
	while (TRUE) {
		
		 set_sleep_mode(SLEEP_MODE_IDLE);
		 sleep_mode();
			
		if (AppFlags.Meas == TRUE) {
			FuelAdded = readfuel();
			ftoa(FuelAdded, tmpstr);
			lcdptr = 0xc7;
			lcdws("     ");
			lcdptr = 0xc7;
			lcdws(tmpstr);
			AppFlags.Meas = FALSE;
		}
		
		if (AppFlags.Fuel == TRUE) {
			GICR &=  ~_BV(INT1);
			MCUCR &= ~_BV(ISC11);
			FillFuel();
			GICR |=  _BV(INT1);
			MCUCR |= _BV(ISC11);
			AppFlags.Fuel = FALSE;
		}
		
		if (AppFlags.msg) {
			DisUARTInt();
			AppFlags.msg = 0;
			lcdclrr(1);
			lcdws("Message Receiv'd");
			beep(1,250);
			switch (checkmsg()){
				case 1:	sendloc(PhNum,"Your Vehicle is Located"); break;
				default: beep(1,500); lcdclrr(1); break;
			}
			disptitl();
			EnUARTInt();
		}
	}
	return 0;
}
static void init(void)
{
	AppFlags.Fuel = FALSE;
	AppFlags.Meas = FALSE;
	AppFlags.msg = FALSE;
	
	buzinit();
	ledinit();
	beep(2,100);
	motorinit();
	lcdinit();
	adcinit();
	uartinit();
	EXTINTinit();
	EXTINTinit();
	tmr1init();
	GPSinit();
	GSMEn();
	GSMinit();
	disptitl();
	beep(1,100);
	sei();
	StartTmr();
	EnUARTInt();

}
static void disptitl(void)
{
	lcdclr();
	lcdws("Digital FuelGuag");
	lcdr2();
	lcdws("  Fuel:     Lts");
}
static void tmr1init(void)
{
	TCNT1H   = 0xD3;
	TCNT1L   = 0x00;
	TIMSK   |= _BV(TOIE1);			//ENABLE OVERFLOW INTERRUPT
	TCCR1A   = 0x00;					
	TCCR1B  |= _BV(CS10) | _BV(CS11); /* PRESCALAR BY 16 */
}
static void EXTINTinit(void)
{
	INTDDR 	&= ~_BV(INT0_PIN);
	INTPORT |= _BV(INT0_PIN);

	INTDDR 	&= ~_BV(INT1_PIN);
	INTPORT |= _BV(INT1_PIN);

	GICR |= _BV(INT0) | _BV(INT1);			//ENABLE EXTERNAL INTERRUPT
	MCUCR |= _BV(ISC01) | _BV(ISC11);		//FALLING EDGE INTERRUPT

}
/* overflows at every 100msec */
ISR(TIMER1_OVF_vect) 
{ 
	static int8u i, j;

	TCNT1H = 0xD3;
	TCNT1L = 0x00;
	
	if (AppFlags.Meas == FALSE && ++j >= 10) {
		AppFlags.Meas = TRUE;
		j = 0;
	}
	if (++i >= 50) 
		 i = 0;
			
	switch(i) {
		case 0: case 2: ledon(); break;
		case 1: case 3: ledoff(); break;
	} 
	/* put in sleep */
	
}
ISR(INT1_vect)
{

	AppFlags.Fuel = 1;
	GICR |= _BV(INT1);
}

static void FillFuel(void)
{
	float NewFuelRd;
	float OldFuelRd;
	char tmpstr[10];
	
	
	beep(1,100);
	
	OldFuelRd = readfuel() ;
	lcdclr();
	lcdws(" FILL THE FUEL");
	dlyms(2000);
	lcdr2();
	lcdws( "PRESS SW A'COMPL");
	
	while (INTPIN & _BV(INT1_PIN));
	
	beep(1,100);
	NewFuelRd = readfuel() ;
	lcdclr();
	FuelAdded = NewFuelRd - OldFuelRd;
	
	if (FuelAdded > 0.1) {
		lcdws("Fuel Added:      Lts");
		ftoa(FuelAdded, tmpstr);
		lcdptr = 0x8b;
		lcdws(tmpstr);
		lcdr2();
		lcdws( "Amount:      Rs");
		Amount = FuelAdded * 72;
		itoa(Amount, tmpstr);
		lcdptr = 0xc7;
		lcdws(tmpstr);
		beep(1,100);
		dlyms(2000);
		lcdclrr(1);
		sendmsg();
	} else {
		lcdws ( "No Fuel Added! " );
		beep(1,250);
		dlyms(2000);
	}
	disptitl();

}
static void sendmsg (void)
{
		
		int8u gsmmsg[150];
		char tmpstr[10];
		gsmmsg[0] = '\0';

		ftoa(FuelAdded, tmpstr);
		strcat(gsmmsg, tmpstr);
		strcat(gsmmsg,"Lts & ");
		itoa(Amount,tmpstr);
		strcat(gsmmsg,tmpstr);
		strcat (gsmmsg, "Rs of fuel filled");
		
		DisUARTInt();
		sendloc(PhNum, gsmmsg);
		EnUARTInt();
		
}

float readfuel(void)
{
	return ((adcget(0) - REF_VALUE) * CONSTANT_GAIN);
//	return adcget(0);
}

static int8u checkmsg(void)
{
	if (!strcmp(sbuf, "track"))
		return 1;
	else
		return 0;
}
ISR (USART_RXC_vect)
{
	static int8u i;
	static int8u msgcnt,phcnt;
	static int8u state = MSG_WAIT_MSG;

	switch (state) {
		case MSG_WAIT_MSG:
		if ( UDR == '\"') state = MSG_PH_NUM;
		break;
		case MSG_PH_NUM:
		if (phcnt++ < 13)
		phnum[phcnt-1] = UDR;
		else
		state = MSG_COLL_MSG;
		break;
		case MSG_COLL_MSG:
		if (UDR == LINE_FEED)
		state = MSG_RCV_MSG;
		break;
		case MSG_RCV_MSG:
		if ((sbuf[msgcnt++] = UDR) == LINE_FEED) {
			sbuf[msgcnt-2] = '\0';
			for (i = 0 ; i < 10; i++)	/* eliminate +91 */
			phnum[i] = phnum[i+3];
			phnum[i] = '\0';
			state = MSG_WAIT_MSG;
			msgcnt = 0;
			phcnt = 0;
			AppFlags.msg = 1;
			DisUARTInt();
		}
	}
}