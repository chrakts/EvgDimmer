#ifndef EVGDIMMER_H_INCLUDED
#define EVGDIMMER_H_INCLUDED

#include <ctype.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "../rc5-32/rc5-32.h"
#include "avr/eeprom.h"
#include "avr/pgmspace.h"
#include "xmegaClocks.h"
#include "SPI.h"
#include "RFM69.h"
#include "secrets.h"
#include "ledHardware.h"

#include "External.h"

#define SYSCLK CLK32M
#define PLL 0
#define CLOCK_CALIBRATION 0x83

#define myID          'R'
//#define toID          33
#define NETWORK       '1'

enum{OFF=0,ON,TOGGLE};

enum{JOB_NORMAL=0,JOB_CHOICE,JOB_PROGRAMM};
enum{LAMP0=0,LAMP1,LAMP2,LAMP3,DIMMER0,DIMMER1,DIMMER2};

void interpreteKey();
bool interpreteData(char *data,char *function,char *address, char *job, char *command);
void doJob(char function,char address, char job, char *command);
void NummernTaste( char Taste );
void increaseDimmer(uint8_t num);
void decreaseDimmer(uint8_t num);
void sendStatus(uint8_t type);
void initStatus();
void updateStatus();
void printStatus();
void setDimmer(char dimmer,char *status);
void setDimmer(uint8_t dimmer,uint8_t status);
void setLamp(char lamp,char *status);
void setLamp(uint8_t lamp,uint8_t status);

uint8_t getChecksum(LAMP_STATUS *toTest);
uint8_t proveChecksum(LAMP_STATUS *toTest);
void setChecksum(LAMP_STATUS *toTest);
bool getStatusFromEEProm(LAMP_STATUS *toSet,uint8_t sceneNum);
void setStatusToEEProm(LAMP_STATUS *toSet,uint8_t sceneNum);
void getStatusFromPGM(LAMP_STATUS *toSet);
#endif // EVGDIMMER_H_INCLUDED
