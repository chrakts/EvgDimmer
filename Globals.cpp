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
  {TM_STOP,RESTART_NO,4,0,NULL},              // RFM-Timer
  {TM_START,RESTART_YES,400,0,clear2Send},    // regelmäßige ereignisunabhängige Aktualisierung
  {TM_START,RESTART_YES,50,0,clear2Update},   // Aktualisierung, falls Statusänderung
  {TM_STOP,RESTART_NO,30,0,ledToggle},		    // LED-Blinken
  {TM_STOP,RESTART_NO,1000,0,stopSpecialTim }     // Spezialmodus beenden
};

Serial debug(0);
//Communication cmulti(1,"DF",8);

volatile uint8_t IR_Remote,Taste_Neu;

LAMP_STATUS actualStatus,oldStatus,lastUpdateStatus;

const  LAMP_STATUS backupStatus PROGMEM = {1,{254,254,5},146};

uint8_t dimmerMin[3] EEMEM={50,50,50};
uint8_t dimmerMax[3] EEMEM={254,254,254};


LAMP_STATUS saveStatus[10] EEMEM= {  {1,{254,254,5},146},{2,{254,254,5},252},{4,{254,254,5},250},{8,{254,254,5},146},{2,{254,254,5},146},{1,{254,254,5},146},{1,{254,254,5},146},{3,{50,50,5},146},{8,{254,254,5},146},{16,{254,254,5},146}  };




RFM69 *globRFM;

// Zuordnung zwischen logischer Nummer (Lampe und Dimmer) zu Hardware
uint8_t LAMP_MAP[4] = {PIN4_bm,PIN5_bm,PIN6_bm,PIN7_bm};
register8_t DIMMER_MAP[3] = {(TCC0.CCDL),(TCC0.CCCL),(TCC0.CCBL)};

