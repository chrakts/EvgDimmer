/*
 */
#include "evgDimmer.h"

const char RFM69Key[16] = {RFM69KEY};

int main(void)
{
  char  job,function,address;
  char  command[23];

    // Insert code
  init_clock(SYSCLK, PLL,true,CLOCK_CALIBRATION);

 	PORTA_DIRSET = PIN2_bm | PIN3_bm;
	PORTA_OUTSET = 0xff;

	PORTB_DIRCLR = 0xff;

	PORTC_DIRSET = 0xff;

	PORTD_DIRSET = PIN0_bm | PIN1_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN7_bm;
	PORTD_DIRCLR = PIN2_bm | PIN6_bm;
	PORTD_OUTCLR = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm;

	PORTE_DIRSET = 0xff;
	PORTE.OUTSET = 1;
  LEDROTSETUP;
  LEDGRUENSETUP;


  _delay_ms(1000);
  init_mytimer();
  rc5_init();

  SPI_Master_t spiRFM69;
  SPI_MasterInit(&spiRFM69,&(SPID),&(PORTD),false,SPI_MODE_0_gc,SPI_INTLVL_LO_gc,false,SPI_PRESCALER_DIV128_gc);

  RFM69 myRFM(&MyTimers[RFM69_TIMER],&spiRFM69,true);
  globRFM = &myRFM;

  PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;

  sei();
  LEDROT_ON;
  debug.open(Serial::BAUD_57600,F_CPU);
  debug.print("\nHallo vom rc5-32 Empfaenger");
  initStatus();
  myRFM.initialize(86,myID,NETWORK);
  myRFM.encrypt(RFM69Key);

  debug.print("\nRFM-Init fertig\n");
  LEDROT_OFF;

  nextSendReady=true;
  while(1)
  {
    if( IR_Remote > 0)
    {
      interpreteKey();
    }
    if(nextSendReady)
    {
      LEDROT_TOGGLE;
      sendStatus(nextStatus2Send);
      nextSendReady = false;
    }
    if(nextUpdateReady)
    {
      nextUpdateReady=false;
      updateStatus();
    }
    //check for any received packets
    if (myRFM.receiveDone())
    {
      LEDROT_ON;
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

      LEDROT_OFF;
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
      jobState = JOB_CHOICE;
    break;
    case T_STOP:			/* Taste Stop -> nimmt auf */
      jobState = JOB_PROGRAMM;
    break;
    case T_LETZTER:
      jobState = JOB_NORMAL;
    break;
    case T_PAUSE:
      jobState = JOB_NORMAL;
    break;
    case T_KANAL_P: // Kanal 0 dimmen
      increaseDimmer(0);
    break;
    case T_KANAL_M:
      decreaseDimmer(0);
    break;
    case T_FF:   // Kanal 1 dimmen
      increaseDimmer(1);
    break;
    case T_REW:
      decreaseDimmer(1);
    break;
/*    case T_START_PL:
      if( (PORTC & 0b00000010)==0 )    // Dimmen, nur wenn eingeschaltet
      {
        if( job == JOB_PROGRAMM )
        {
          eeprom_write_byte ( (uint8_t *) (DIMMER2_START_PL), Dim[1] );
        }
        else
        {
          Dim[1] = eeprom_read_byte ( (uint8_t *) (DIMMER2_START_PL) );
          DIMMER_2 = Dim[1];
        }
      }
      Taste_Neu = false;
      job = JOB_NORMAL;
    break;
    case T_START_MI:
      if( (PORTC & 0b00000010)==0 )    // Dimmen, nur wenn eingeschaltet
      {
        if( job == JOB_PROGRAMM )
        {
          eeprom_write_byte ( (uint8_t *) (DIMMER2_START_MI), Dim[1] );
        }
        else
        {
          Dim[1] = eeprom_read_byte ( (uint8_t *) (DIMMER2_START_MI) );
          DIMMER_2 = Dim[1];
        }
      }
      Taste_Neu = false;
      job = JOB_NORMAL;
    break;
    case T_STOP_PL:
      if( (PORTC & 0b00000100)==0 )    // Dimmen, nur wenn eingeschaltet
      {
        if( job == JOB_PROGRAMM )
        {
          eeprom_write_byte ( (uint8_t *) (DIMMER3_STOP_PL), Dim[2] );
        }
        else
        {
          Dim[2] = eeprom_read_byte ( (uint8_t *) (DIMMER3_STOP_PL) );
          DIMMER_3 = Dim[2];
        }
      }
      Taste_Neu = false;
      job = JOB_NORMAL;
    break;
    case T_STOP_MI:
      if( (PORTC & 0b00000100)==0 )    // Dimmen, nur wenn eingeschaltet
      {
        if( job == JOB_PROGRAMM )
        {
          eeprom_write_byte ( (uint8_t *) (DIMMER3_STOP_MI), Dim[2] );
        }
        else
        {
          Dim[2] = eeprom_read_byte ( (uint8_t *) (DIMMER3_STOP_MI) );
          DIMMER_3 = Dim[2];
        }
      }
      Taste_Neu = false;
      job = JOB_NORMAL;
    break;
    case T_DATE_PL:
      Dim[1]=eeprom_read_byte ( (uint8_t *) (DIMMER2_START_PL) );
      Dim[2]=eeprom_read_byte ( (uint8_t *) (DIMMER3_STOP_PL) );
      DIMMER_2 = Dim[1];
      DIMMER_3 = Dim[2];
    break;
    case T_DATE_MI:
      Dim[1]=eeprom_read_byte ( (uint8_t *) (DIMMER2_START_MI) );
      Dim[2]=eeprom_read_byte ( (uint8_t *) (DIMMER3_STOP_MI) );
      DIMMER_2 = Dim[1];
      DIMMER_3 = Dim[2];
    break;*/
  }
  IR_Remote = 0;
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
				Taste_Neu = false;
			}
			else
			{
				if( (Taste_Neu == true) && (Taste < 10) )
				{
/*					PORTC = eeprom_read_byte ( (uint8_t *) (4*Taste) );
					Dim[0]  = eeprom_read_byte ( (uint8_t *) (4*Taste+1) );
					Dim[1]  = eeprom_read_byte ( (uint8_t *) (4*Taste+2) );
					Dim[2]  = eeprom_read_byte ( (uint8_t *) (4*Taste+3) );
					DIMMER_2 =  Dim[1];
					DIMMER_3 =  Dim[2];
					Taste_Neu = false;*/
					jobState = JOB_NORMAL;
				}
			}
			IR_Remote = 0;
		break;
		case JOB_CHOICE:
/*			PORTC = eeprom_read_byte ( (uint8_t *) (4*Taste) );
			Dim[0]  = eeprom_read_byte ( (uint8_t *) (4*Taste+1) );
			Dim[1]  = eeprom_read_byte ( (uint8_t *) (4*Taste+2) );
			Dim[2]  = eeprom_read_byte ( (uint8_t *) (4*Taste+3) );
			DIMMER_2 =  Dim[1];
			DIMMER_3 =  Dim[2];
			Taste_Neu = false;*/
			jobState = JOB_NORMAL;
		break;
		case JOB_PROGRAMM:
/*			eeprom_write_byte ( (uint8_t *) (4*Taste), PORTC );
			eeprom_write_byte ( (uint8_t *) (4*Taste+1), Dim[0] );
			eeprom_write_byte ( (uint8_t *) (4*Taste+2), Dim[1] );
			eeprom_write_byte ( (uint8_t *) (4*Taste+3), Dim[2] );
			Taste_Neu = false;*/
			jobState = JOB_NORMAL;
		break;
	}
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

          globRFM->sendWithRetry('T', toSend, 15, 10, 100);
          debug.println(toSend);

        break;
        case 4 ... 6:
          sprintf(toSend,"BRK1SD%dST%d",type-4,actualStatus.dimmer[type-4]);
          globRFM->sendWithRetry('T', toSend, 15, 10, 100);
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
  debug.pformat("\nStatus: %d - %d %d %d",actualStatus.lamps,actualStatus.dimmer[0],actualStatus.dimmer[1],actualStatus.dimmer[2]);
  debug.pformat("\n      :     - %d %d %d",actualStatus.minDimmer[0],actualStatus.minDimmer[1],actualStatus.minDimmer[2]);
  debug.pformat("\n      :     - %d %d %d",actualStatus.maxDimmer[0],actualStatus.maxDimmer[1],actualStatus.maxDimmer[2]);
}

uint8_t getChecksum(LAMP_STATUS *toTest)
{
  uint8_t s = toTest->lamps;
  for(uint8_t i=0;i<3;i++)
    s += toTest->dimmer[i];
  for(uint8_t i=0;i<3;i++)
    s += toTest->minDimmer[i];
  for(uint8_t i=0;i<3;i++)
    s += toTest->maxDimmer[i];
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
  toSet->lamps = eeprom_read_byte(&(saveStatus[sceneNum].lamps));
  for(uint8_t i=0;i<3;i++)
    toSet->dimmer[i] = eeprom_read_byte( &(saveStatus[sceneNum].dimmer[i]) );
  for(uint8_t i=0;i<3;i++)
    toSet->minDimmer[i] = eeprom_read_byte( &(saveStatus[sceneNum].minDimmer[i]) );
  for(uint8_t i=0;i<3;i++)
    toSet->maxDimmer[i] = eeprom_read_byte( &(saveStatus[sceneNum].maxDimmer[i]) );
  toSet->checksum = eeprom_read_byte( &(saveStatus[sceneNum].checksum) );
  return(proveChecksum(toSet));
}

void setStatusToEEProm(LAMP_STATUS *toSet,uint8_t sceneNum)
{
uint8_t i;
  debug.print("\nSichere Status");
  eeprom_write_byte(&(saveStatus[sceneNum].lamps),toSet->lamps );
  for(i=0;i<3;i++)
    eeprom_write_byte( &(saveStatus[sceneNum].dimmer[i]) ,toSet->dimmer[i]);
  for(i=0;i<3;i++)
    eeprom_write_byte( &(saveStatus[sceneNum].minDimmer[i]) ,toSet->minDimmer[i]);
  for(i=0;i<3;i++)
    eeprom_write_byte( &(saveStatus[sceneNum].maxDimmer[i]) ,toSet->maxDimmer[i]);
  uint8_t c = getChecksum(toSet);
  eeprom_write_byte( &(saveStatus[sceneNum].checksum),c );
}

void getStatusFromPGM(LAMP_STATUS *toSet)
{
  toSet->lamps = pgm_read_byte(&(backupStatus.lamps));
  for(uint8_t i=0;i<3;i++)
    toSet->dimmer[i] = pgm_read_byte( &(backupStatus.dimmer[i]) );
  for(uint8_t i=0;i<3;i++)
    toSet->minDimmer[i] = pgm_read_byte( &(backupStatus.minDimmer[i]) );
  for(uint8_t i=0;i<3;i++)
    toSet->maxDimmer[i] = pgm_read_byte( &(backupStatus.maxDimmer[i]) );
  toSet->checksum = pgm_read_byte( &(backupStatus.checksum) );
}

