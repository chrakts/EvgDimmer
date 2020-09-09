/*
 */

#include <avr/io.h>
#include "External.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "SPI.h"
#include "RFM69.h"


enum{QUARZ,CLK2M,CLK32M};

const char RFM69Key[16] = {RFM69KEY};

#define SYSCLK QUARZ

#define PLL 0

#define myID          'L'
#define toID          'T'
#define NETWORK       '1'

void init_clock(int sysclk, int pll);

int main(void)
{
  char text[32];
  uint32_t i=0;
    // Insert code
  init_clock(SYSCLK, PLL);
  PORTD.DIRSET = PIN0_bm;
  init_mytimer();

  RFM69 myRFM(&MyTimers[RFM69_TIMER]);

  PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
  sei();


  debug.open(Serial::BAUD_57600,F_CPU);
  debug.print("\nHallo vom Sender");
  debug.print("\nHallo vom Sender\n");

  myRFM.initialize(86,myID,NETWORK);
  myRFM.encrypt(RFM69Key);
  myRFM.readAllRegsCompact();
  while(1)
  {
    sprintf(text,"Neue Nachricht %lx\n",i);
    debug.print(text);
    MyTimers[SEND_REPEAT].state = TM_START;

    while(MyTimers[SEND_REPEAT].state != TM_STOP)
      ;
    if ( myRFM.sendWithRetry(toID, text, strlen(text)) )
      debug.print(" ok!");
    else
      debug.print(" nothing...");

    PORTD.OUTTGL = PIN0_bm;
    i++;
  }

  return 0;
}

void init_clock(int sysclk, int pll)
{
	CLK_t *mein_clock;
	OSC_t *mein_osc;
	mein_clock = &CLK;
	mein_osc = &OSC;
	switch(sysclk)
	{
		case QUARZ:
			mein_osc->XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
//			mein_osc->XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCPWR_bm | OSC_XOSCSEL_XTAL_16KCLK_gc;
			mein_osc->CTRL = OSC_XOSCEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet die 32 MHz-Clock ein

			while((mein_osc->STATUS & OSC_XOSCRDY_bm) == 0)			// wartet bis diese stabil
			;
			while((mein_osc->STATUS & OSC_RC32KRDY_bm) == 0)		// wartet bis diese stabil
			;

			if ( (pll>0) & (pll<16) )
			{
				mein_osc->PLLCTRL = OSC_PLLSRC_XOSC_gc | pll;
				mein_osc->CTRL = OSC_PLLEN_bm | OSC_XOSCEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet zusätzlich die PLL ein

				while((mein_osc->STATUS & OSC_PLLRDY_bm) == 0)		// wartet bis diese stabil
				;
				CCP = CCP_IOREG_gc;										// geschuetztes Register freigeben
				mein_clock->CTRL = CLK_SCLKSEL_PLL_gc;					// umschalten auf PLL-Clock
				mein_osc->CTRL = OSC_PLLEN_bm | OSC_XOSCEN_bm | OSC_RC32KEN_bm;
			}
			else
			{
				CCP = CCP_IOREG_gc;										// geschuetztes Register freigeben
				mein_clock->CTRL = CLK_SCLKSEL_XOSC_gc;					// umschalten auf XOSC-Clock
				mein_osc->CTRL = OSC_XOSCEN_bm | OSC_RC32KEN_bm;
			}
		break; // QUARZ
		case CLK2M:
			mein_osc->CTRL = OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet die 2 MHz-Clock ein
			while((mein_osc->STATUS & OSC_RC2MRDY_bm) == 0)  // wartet bis diese stabil
			;
			while((mein_osc->STATUS & OSC_RC32KRDY_bm) == 0)  // wartet bis diese stabil
			;
			CCP = CCP_IOREG_gc;								// geschuetztes Register freigeben
			mein_clock->CTRL = CLK_SCLKSEL_RC2M_gc;		// umschalten auf 2 MHz-Clock
//			CLKSYS_AutoCalibration_Enable(OSC_RC2MCREF_RC32K_gc,false); // OSC_RC32MCREF_bm
		break;
		case CLK32M:
			mein_osc->CTRL = OSC_RC32MEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet die 32 MHz-Clock ein
			while((mein_osc->STATUS & OSC_RC32MRDY_bm) == 0)  // wartet bis diese stabil
			;
			while((mein_osc->STATUS & OSC_RC32KRDY_bm) == 0)  // wartet bis diese stabil
			;
			CCP = CCP_IOREG_gc;								// geschuetztes Register freigeben
			mein_clock->CTRL = CLK_SCLKSEL_RC32M_gc;		// umschalten auf 32 MHz-Clock
			mein_osc->CTRL = OSC_RC32MEN_bm | OSC_RC32KEN_bm;		// abschalten der 2 MHz-Clock
//			CLKSYS_AutoCalibration_Enable(OSC_RC32MCREF_RC32K_gc,false); // OSC_RC32MCREF_bm
		break;
	}
}
