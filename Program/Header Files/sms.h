#ifndef SMS_H
#define SMS_H

//INCLUDE HEADER FILES
#include "includes.h"


#define GSMEn()				do {									\
								GSM_GPS_PORT &= ~_BV(GSM_GPS_PIN); 	\
								dlyms(250);							\
							} while(0)

#define GPSEn()				do {									\
								GSM_GPS_PORT |= _BV(GSM_GPS_PIN);	\
								dlyms(250);							\
							} while(0)


//FUNCTION PROTOTYPE
void GPSinit	(void);
void sendloc	(const char *PhNum, const char *GSMMsg);

#endif