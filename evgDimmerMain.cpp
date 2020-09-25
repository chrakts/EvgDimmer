/*
 */
#include "evgDimmer.h"

const char RFM69Key[16] = {RFM69KEY};

int main(void)
{
  char  job,function,address;
  char  command[23];
  uint8_t resetStatus;

  resetStatus = RST.STATUS;
  RST.STATUS = 0xff;
  WDT_EnableAndSetTimeout(WDT_SHORT);
  WDT_Reset();
  init_clock(SYSCLK, PLL,true,CLOCK_CALIBRATION);

 	PORTA_DIRSET = PIN2_bm | PIN3_bm;
	PORTA_OUTSET = 0xff;

	PORTB_DIRCLR = 0xff;

	PORTC_DIRSET    = 0xff;
	PORTC_OUTSET = 0xff;

	PORTD_DIRSET = PIN0_bm | PIN1_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN7_bm;
	PORTD_DIRCLR = PIN2_bm | PIN6_bm;
	PORTD_OUTCLR = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;

	PORTE_DIRSET = 0xff;
	PORTE_OUTSET = 1;
  LEDROTSETUP;
  LEDGRUENSETUP;

  LEDROT_ON;
  LEDGRUEN_ON;

  init_mytimer();
  rc5_init();

  WDT_Reset();

  SPI_Master_t spiRFM69;
  SPI_MasterInit(&spiRFM69,&(SPID),&(PORTD),false,SPI_MODE_0_gc,SPI_INTLVL_LO_gc,false,SPI_PRESCALER_DIV128_gc);

  RFM69 myRFM(&MyTimers[RFM69_TIMER],&spiRFM69,true);
  globRFM = &myRFM;
  WDT_Reset();

  PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;

  sei();
  LEDROT_ON;
  debug.open(Serial::BAUD_57600,F_CPU);
  debug.println("Hallo vom rc5-32 Empfaenger");
  debug.pformat("Reset-Status: %x",resetStatus);
  initStatus();
  myRFM.initialize(86,myID,NETWORK);
  myRFM.encrypt(RFM69Key);
  WDT_Reset();

  debug.println("RFM-Init fertig\n");
  LEDROT_OFF;
  initPWM();
  nextSendReady=true;

  while(1)
  {
    WDT_Reset();
    // setzt Lampenausgänge und Dimmer-PWMs auf die aktuellen Werte
    updateHardware();
    // Neuer IR-Befehl ?
    if( IR_Remote > 0)
    {
      interpreteKey();
    }
    // regelmäßiges unbedingtes Update über Funk
    if(nextSendReady)
    {
      sendStatus(nextStatus2Send);
      nextSendReady = false;
    }
    // bei Änderung wird das Update gesendet über Funk
    if(nextUpdateReady)
    {
      nextUpdateReady=false;
      updateStatus();
    }
    // neuer Funk-Befehl?
    if (myRFM.receiveDone())
    {
      debug.pformat("[%d]-[%d]-[RX_RSSI: %d]:",myRFM.SENDERID,myRFM.DATALEN,myRFM.RSSI);
      debug.print((char *)myRFM.DATA);

      if (myRFM.ACKRequested())
      {
        myRFM.sendACK();
        debug.print(" - ACK sent\n");
      }
      if( interpreteData((char *)myRFM.DATA,&function,&address,&job,command) )
      {
        if(command != NULL)
           doJob(function,address,job,command);
      }
      else
        debug.print("Fehler\n");
    }

  }

  return 0;
}

void doJob(char function,char address, char job, char *command)
{

  switch(job)
  {
    case 'w':
      switch(function)
      {
        case 'L':
          setLamp(address,command);
        break;
        case 'D':
          setDimmer(address,command);
        break;
      }

    break;
  }

}

void startSpecial(uint8_t state)
{
  jobState=state;
	MyTimers[LED_BLINK_TIMER].restart = RESTART_YES;
	MyTimers[LED_BLINK_TIMER].state = TM_START;
	MyTimers[STATUS_SPECIAL_TIMER].state = TM_START;
}

void stopSpecial(void)
{
  jobState=JOB_NORMAL;
	MyTimers[LED_BLINK_TIMER].restart = RESTART_NO;
	MyTimers[LED_BLINK_TIMER].state = TM_STOP;
	LEDROT_OFF;
}

// R1XXSL0wToff
// R1XXSL0wToff
bool interpreteData(char *data,char *function,char *address, char *job, char *command)
{
  bool noError = false;
  uint8_t l = strlen(data);
  if( (l>8) && l<30 )
  {
    if( (data[0]==myID) && (data[1]==NETWORK) && (data[4]=='S') && (data[8]=='T') )
    {
      noError = true;
      *function = data[5];
      *address  = data[6];
      *job      = data[7];
      if(l>9)
        strcpy(command,&(data[9]));
      else
        command = NULL;
    }
  }
  return(noError);
}

void interpreteKey()
{
  switch(IR_Remote)					/* wurde eine Taste gedrueckt  */
  {
    case T_1:				/* Taste 1 */
      NummernTaste( 0 );
    break;
    case T_2:				/* Taste 2 */
      NummernTaste( 1 );
    break;
    case T_3:				/* Taste 3		*/
      NummernTaste( 2 );
    break;
    case T_4:				/* Taste 4 */
      NummernTaste( 3 );
    break;
    case T_5:				/* Taste 5		*/
      NummernTaste( 4 );
    break;
    case T_6:				/* Taste 6		*/
      NummernTaste( 5 );
    break;
    case T_7:				/* Taste 7		*/
      NummernTaste( 6 );
    break;
    case T_8:				/* Taste 8		*/
      NummernTaste( 7 );
    break;
    case T_9:				/* Taste 9		*/
      NummernTaste( 8 );
    break;
    case T_0:				/* Taste 0		*/
      NummernTaste( 9 );
    break;
    case T_ONOFF:			/* Taste ONOFF	*/
      actualStatus.lamps = 0;
    break;
    case T_PLAY:			/* Taste PLAY */
      startSpecial(JOB_CHOICE);
    break;
    case T_STOP:			/* Taste Stop -> nimmt auf */
    case T_PROG_MENU:
      startSpecial(JOB_PROGRAMM);
    break;
    case T_OK:
      /* ************************************************************************* */
      /* ******************* Löst absichtlich Watchdog-Reset aus ***************** */
      /* ************************************************************************* */
      while(1);
      stopSpecial();
    break;
    case T_LETZTER:
      jobState = JOB_NORMAL;
    break;
    case T_PAUSE:
      jobState = JOB_NORMAL;
    break;
    case T_KANAL_P: // Kanal 0 dimmen
      increaseDimmer(1);
    break;
    case T_KANAL_M:
      decreaseDimmer(1);
    break;
    case T_FF:   // Kanal 1 dimmen
      increaseDimmer(0);
    break;
    case T_REW:
      decreaseDimmer(0);
    break;
    // Dimmer 0 maximale Helligkeit
    case T_START_PL:
      if( actualStatus.lamps & 0x01 )    // Dimmen, nur wenn eingeschaltet
      {
        if( jobState == JOB_PROGRAMM )
          eeprom_write_byte ( &(dimmerMax[0]), actualStatus.dimmer[0] );
        else
          actualStatus.dimmer[0] = eeprom_read_byte ( &(dimmerMax[0]) );
      }
      Taste_Neu = false;
      jobState = JOB_NORMAL;
    break;
    // Dimmer 0 mimimale Helligkeit
    case T_START_MI:
      if( actualStatus.lamps & 0x01 )    // Dimmen, nur wenn eingeschaltet
      {
        if( jobState == JOB_PROGRAMM )
          eeprom_write_byte ( &(dimmerMin[0]), actualStatus.dimmer[0] );
        else
          actualStatus.dimmer[0] = eeprom_read_byte ( &(dimmerMin[0]) );
      }
      Taste_Neu = false;
      jobState = JOB_NORMAL;
    break;
    // Dimmer 1 maximale Helligkeit
    case T_STOP_PL:
      if( actualStatus.lamps & 0x02 )    // Dimmen, nur wenn eingeschaltet
      {
        if( jobState == JOB_PROGRAMM )
          eeprom_write_byte ( &(dimmerMax[1]), actualStatus.dimmer[1] );
        else
          actualStatus.dimmer[1] = eeprom_read_byte ( &(dimmerMax[1]) );
      }
      Taste_Neu = false;
      jobState = JOB_NORMAL;
    break;
    // Dimmer 1 mimimale Helligkeit
    case T_STOP_MI:
      if( actualStatus.lamps & 0x02 )    // Dimmen, nur wenn eingeschaltet
      {
        if( jobState == JOB_PROGRAMM )
          eeprom_write_byte ( &(dimmerMin[1]), actualStatus.dimmer[1] );
        else
          actualStatus.dimmer[1] = eeprom_read_byte ( &(dimmerMin[1]) );
      }
      Taste_Neu = false;
      jobState = JOB_NORMAL;
    break;
    // Kanal 1 aufdimmen
    case T_DATE_PL:
      increaseDimmer(0);
    break;
    // Kanal 1 abdimmen
    case T_DATE_MI:
      decreaseDimmer(0);
    break;
    default:
      debug.pformat("unbelegte Taste: %d\n",IR_Remote);
    //break;
  }
  IR_Remote = 0;
}

void initPWM()
{
  TCC0.CTRLA = TC_CLKSEL_DIV64_gc;  // Teiler auf 64
  TCC0.CTRLB = TC0_CCDEN_bm | TC0_CCBEN_bm | TC0_CCCEN_bm | TC_WGMODE_SINGLESLOPE_gc; // single-Slope auf 3 Kanälen
  TCC0.CTRLE = TC_BYTEM_BYTEMODE_gc; // Splitmode
  TCC0.PERL  = 254;
  TCC0.CCBL  = 128;
  TCC0.CCCL  = 128;
  TCC0.CCDL  = 128;
}

void NummernTaste( char Taste )
{
	switch( jobState )
	{
		case JOB_NORMAL:
			if( (Taste_Neu == true) && (Taste < 4) )
			{
        LEDGRUEN_TOGGLE;
				actualStatus.lamps ^=  (0b00000001<<Taste);
			}
			else
			{
				if( (Taste_Neu == true) && (Taste < 10) )
				{
          if(getStatusFromEEProm(&actualStatus,Taste)==true)
          {
            debug.println("Lade EEPROM");
          }
          else
          {
            debug.println("Lade EEPROM fehlgeschlagen");
          }
          printStatus();
				}
			}
		break;
		case JOB_CHOICE:
      if(getStatusFromEEProm(&actualStatus,Taste)==true)
      {
        debug.println("Lade EEPROM");
      }
      else
      {
        debug.println("Lade EEPROM fehlgeschlagen");
      }
      printStatus();
			stopSpecial();
		break;
		case JOB_PROGRAMM:
      setStatusToEEProm(&actualStatus,Taste);
      debug.println("Speichere ins EEPROM");
			stopSpecial();
		break;
	}
  Taste_Neu = false;
  IR_Remote = 0;

}

void updateHardware()
{
  uint8_t i,test;

  for(i=0;i<4;i++)
  {
    if( actualStatus.lamps & (1<<i) )
      PORTC.OUTCLR = LAMP_MAP[i];
    else
      PORTC.OUTSET = LAMP_MAP[i];
  }

/*
  PORTC_DIR = 0xff;
  test = actualStatus.lamps<<4;
  PORTC_OUT = test | 0b11101111;
*/

  TCC0.CCDL = 255-actualStatus.dimmer[0];
  TCC0.CCCL = 255-actualStatus.dimmer[1];
  TCC0.CCBL = 255-actualStatus.dimmer[2];
}

void updateStatus()
{
uint8_t i;
  uint8_t l = lastUpdateStatus.lamps ^ actualStatus.lamps;
  i=1;
  for(i=0;i<4;i++)
  {
    if(  ( (1<<i)&l ) > 0  )
      sendStatus(i);
  }
  for(i=0;i<3;i++)
  {
    if(lastUpdateStatus.dimmer[i]!=actualStatus.dimmer[i])
      sendStatus(i+4);
  }
  lastUpdateStatus.lamps = actualStatus.lamps;
  for(i = 0;i<3;i++)
    lastUpdateStatus.dimmer[i] = actualStatus.dimmer[i];
}

void sendStatus(uint8_t type)
{
  char toSend[25];
  switch(type)
  {
        case 0 ... 3:
          sprintf(toSend,"BRK1SL%dST",type);
          if(  (actualStatus.lamps&(1<<type)) > 0  )
            strcat(toSend,"on");
          else
            strcat(toSend,"off");

          globRFM->sendWithRetry('T', toSend, 15, 4, 20);
          debug.println(toSend);

        break;
        case 4 ... 6:
          sprintf(toSend,"BRK1SD%dST%d",type-4,actualStatus.dimmer[type-4]);
          globRFM->sendWithRetry('T', toSend, 15, 4, 20);
          debug.println(toSend);
        break;
        default:
          debug.print("\nFalscher Sendestatus");
  }
}

void setDimmer(char dimmer,char *status)
{
  uint8_t stat;
  debug.println("Dimmerstatus aendern");
  if(isdigit(dimmer))
  {
    uint8_t d=dimmer-48;
    stat = (uint8_t) atol(status);
    setDimmer(d,stat);
  }

}

void setDimmer(uint8_t dimmer,uint8_t status)
{
  if( (dimmer<3) && (status<255) )
  {
    actualStatus.dimmer[dimmer] = status;
  }
}

void setLamp(char lamp,char *status)
{
  debug.println("Lampenstatus aendern");
  if(isdigit(lamp))
  {
    uint8_t l=lamp-48;

    if(strcmp(status,"on")==0)
    {
      setLamp(l,ON);
    }
    else if(strcmp(status,"off")==0)
    {
      setLamp(l,OFF);
    }
  }
}

void setLamp(uint8_t lamp,uint8_t status)
{
  if(lamp <4)
  {
    uint8_t l = 1<<lamp;
    switch(status)
    {
      case OFF:
        actualStatus.lamps &= ~l;
      break;
      case ON:
        actualStatus.lamps |= l;
      break;
      case TOGGLE:
        actualStatus.lamps ^= l;
      break;
    }
  }
}


void increaseDimmer(uint8_t num)
{
  if(  (actualStatus.lamps & (1<<num)) > 0 )
  {
    if(actualStatus.dimmer[num]<254)
    {
      actualStatus.dimmer[num] +=2;
    }
  }
}

void decreaseDimmer(uint8_t num)
{
  if(  (actualStatus.lamps & (1<<num)) > 0 )
  {
    if(actualStatus.dimmer[num]>1)
    {
      actualStatus.dimmer[num] -=2;
    }
  }
}

void initStatus()
{
  if(getStatusFromEEProm(&actualStatus,0)==true)
  {
    debug.print("\nLade EEPROM");
  }
  else
  {
    getStatusFromPGM(&actualStatus);
    debug.print("\nLade Backup");
    setStatusToEEProm(&actualStatus,0);
  }
  printStatus();
}

void printStatus()
{
  debug.pformat("Status: %d - %d %d %d\n",actualStatus.lamps,actualStatus.dimmer[0],actualStatus.dimmer[1],actualStatus.dimmer[2]);
/*  debug.pformat("\n      :     - %d %d %d",actualStatus.minDimmer[0],actualStatus.minDimmer[1],actualStatus.minDimmer[2]);
  debug.pformat("\n      :     - %d %d %d",actualStatus.maxDimmer[0],actualStatus.maxDimmer[1],actualStatus.maxDimmer[2]);*/
}

uint8_t getChecksum(LAMP_STATUS *toTest)
{
  uint8_t s = toTest->lamps;
  for(uint8_t i=0;i<3;i++)
    s += toTest->dimmer[i];
/*  for(uint8_t i=0;i<3;i++)
    s += toTest->minDimmer[i];
  for(uint8_t i=0;i<3;i++)
    s += toTest->maxDimmer[i];*/
  return(!s);
}

uint8_t proveChecksum(LAMP_STATUS *toTest)
{
  uint8_t s = getChecksum(toTest);
  if(toTest->checksum==s)
    return(true);
  else
    return(false);
}

void setChecksum(LAMP_STATUS *toTest)
{
  uint8_t s = getChecksum(toTest);
  toTest->checksum=s;
}

bool getStatusFromEEProm(LAMP_STATUS *toSet,uint8_t sceneNum)
{
  LAMP_STATUS tempStatus;

  tempStatus.lamps = eeprom_read_byte(&(saveStatus[sceneNum].lamps));
  for(uint8_t i=0;i<3;i++)
    tempStatus.dimmer[i] = eeprom_read_byte( &(saveStatus[sceneNum].dimmer[i]) );
  tempStatus.checksum = eeprom_read_byte( &(saveStatus[sceneNum].checksum) );

  if( proveChecksum(&tempStatus) )
  {
    toSet->lamps = tempStatus.lamps;
    for(uint8_t i=0;i<3;i++)
      toSet->dimmer[i] = tempStatus.dimmer[i];
    return( true );

  }
  else
    return(false);
}

void setStatusToEEProm(LAMP_STATUS *toSet,uint8_t sceneNum)
{
uint8_t i;
  debug.println("Sichere Status");
  eeprom_write_byte(&(saveStatus[sceneNum].lamps),toSet->lamps );
  for(i=0;i<3;i++)
    eeprom_write_byte( &(saveStatus[sceneNum].dimmer[i]) ,toSet->dimmer[i]);
  uint8_t c = getChecksum(toSet);
  eeprom_write_byte( &(saveStatus[sceneNum].checksum),c );
}

void getStatusFromPGM(LAMP_STATUS *toSet)
{
  toSet->lamps = pgm_read_byte(&(backupStatus.lamps));
  for(uint8_t i=0;i<3;i++)
    toSet->dimmer[i] = pgm_read_byte( &(backupStatus.dimmer[i]) );
/*  for(uint8_t i=0;i<3;i++)
    toSet->minDimmer[i] = pgm_read_byte( &(backupStatus.minDimmer[i]) );
  for(uint8_t i=0;i<3;i++)
    toSet->maxDimmer[i] = pgm_read_byte( &(backupStatus.maxDimmer[i]) );*/
  toSet->checksum = pgm_read_byte( &(backupStatus.checksum) );
}

