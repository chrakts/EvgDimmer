/*
 * Globals.cpp
 *
 * Created: 19.03.2017 09:24:37
 *  Author: Christof
 */
#define EXTERNALS_H_

#include <stdint.h>


#include "MyTimer.h"
#include "Serial.h"
#include "CRC_Calc.h"
#include "evgDimmer.h"
#include "avr/eeprom.h"
#include "RFM69.h"


char Compilation_Date[] = __DATE__;
char Compilation_Time[] = __TIME__;

volatile bool    nextSendReady=false;
volatile uint8_t nextStatus2Send=0;
volatile uint8_t jobState=JOB_NORMAL;
volatile bool    nextUpdateReady=false;

// Timerwert 16 entspricht 100ms
volatile TIMER MyTimers[MYTIMER_NUM]= {
  {TM_STOP,RESTART_NO,4,0,NULL},
  {TM_START,RESTART_YES,400,0,clear2Send},    // regelmäßige ereignisunabhängige Aktualisierung
  {TM_START,RESTART_YES,50,0,clear2Update},  // Aktualisierung, falls Statusänderung
  {TM_STOP,RESTART_NO,100,0,NULL}		// Timeout-Timer
};

Serial debug(0);
//Communication cmulti(1,"DF",8);

volatile uint8_t IR_Remote,Taste_Neu;

LAMP_STATUS actualStatus,oldStatus,lastUpdateStatus;
const  LAMP_STATUS backupStatus PROGMEM = {1,{254,254,5},{50,50,50},{254,254,254},146};
LAMP_STATUS EEMEM saveStatus[10];

RFM69 *globRFM;

// const unsigned char imageButterfly[IMAGEBUTTERFLY_SIZE] PROGMEM = {};
