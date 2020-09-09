/*
 * External.h
 *
 * Created: 03.04.2017 21:04:41
 *  Author: Christof
 */



#ifndef EXTERNAL_H_
#define EXTERNAL_H_

#include "CRC_Calc.h"
#include "Serial.h"
#include "MyTimer.h"
#include "spi_driver.h"
#include "lampStatus.h"
#include "RFM69.h"

extern volatile uint8_t UART0_ring_received;
//extern volatile  char UART0_ring_buffer[UART0_RING_BUFFER_SIZE];
extern volatile  char UART0_ring_buffer[];
extern volatile uint8_t UART1_ring_received;
extern volatile  char UART1_ring_buffer[];
extern volatile  uint8_t sendFree;
extern volatile TIMER MyTimers[];
extern volatile bool nextSendReady;
extern volatile uint8_t nextStatus2Send;
extern volatile uint8_t jobState;
extern volatile bool nextUpdateReady;

/*  */
extern Serial debug;
extern volatile uint8_t IR_Remote,Taste_Neu;

extern LAMP_STATUS actualStatus,oldStatus,lastUpdateStatus;
extern const  LAMP_STATUS backupStatus PROGMEM ;
extern LAMP_STATUS saveStatus[];

extern RFM69 *globRFM;
#endif /* EXTERNAL_H_ */
