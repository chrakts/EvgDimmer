/*
 * MyTimer.h
 *
 * Created: 11.02.2016 20:20:03
 *  Author: Christof
 */


#ifndef MYTIMER_H_
#define MYTIMER_H_

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#define MYTIMER_NUM	5

enum{TM_START,TM_RESET,TM_STOP,TM_RUN};
enum{RESTART_YES,RESTART_NO};

enum{RFM69_TIMER,SEND_REPEAT_TIMER,SEND_UPDATE_TIMER,LED_BLINK_TIMER,STATUS_SPECIAL_TIMER};

struct Timer
{
	uint8_t		state;
	uint8_t		restart;
	uint16_t	value;
	uint16_t	actual;
	void  (*OnReady)  (uint8_t num);
};

typedef struct Timer TIMER;


void init_mytimer( void );

// Callback-Funktionen
void clear2Send( uint8_t num );
void clear2Update( uint8_t num );
void ledToggle( uint8_t num );
void stopSpecialTim( uint8_t num );

#endif /* MYTIMER_H_ */
