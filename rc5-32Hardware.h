#ifndef RC5_32HARDWARE_H_INCLUDED
#define RC5_32HARDWARE_H_INCLUDED

#define JOIN_(X,Y) X##Y
#define JOIN(X,Y) JOIN_(X,Y)
#define JOIN3_(X,Y,Z) X##Y##Z
#define JOIN3(X,Y,Z) JOIN3_(X,Y,Z)
#define JOIN4_(W,X,Y,Z) W##X##Y##Z
#define JOIN4(W,X,Y,Z) JOIN4_(W,X,Y,Z)
#define JOIN5_(V,W,X,Y,Z) V##W##X##Y##Z
#define JOIN5(V,W,X,Y,Z) JOIN5_(V,W,X,Y,Z)


// -----------------------------------------------------------
// -----------------------------------------------------------
//                variable Eingaben
// -----------------------------------------------------------
// -----------------------------------------------------------
//Definition des rc5-32-Pins
#define RC5_PORT_         B
#define RC5_PIN_NUM       1
#define RC5_INT_NUM       1
//LO | MED | HI       PORT_INT1LVL_HI_gc
#define RC5_INT_LVL       LO
//Definition des Timers
#define RC5_TIMER_PORT C
#define RC5_TIMER_NUM  1
// -----------------------------------------------------------
// -----------------------------------------------------------
// -----------------------------------------------------------
// -----------------------------------------------------------


#define RC5_PORT      JOIN(PORT,RC5_PORT_)
#define RC5_PIN       JOIN3(PIN,RC5_PIN_NUM,_bm)
#define RC5_PINCTRL   JOIN3( PIN,RC5_PIN_NUM,CTRL )
#define RC5_INTVEC    JOIN5(PORT,RC5_PORT_,_INT,RC5_INT_NUM,_vect)
#define RC5_INTMASK   JOIN3( INT,RC5_INT_NUM,MASK )
#define RC5_INT_LEVEL JOIN5(PORT_INT,RC5_INT_NUM,LVL_,RC5_INT_LVL,_gc)

#if RC5_INT_NUM == 0
  #define RC5_INT_OFF   0b11111100
#else
  #define RC5_INT_OFF   0b11110011
#endif // RC5_INT_NUM


#define RC5_TIMER      JOIN3(TC,RC5_TIMER_PORT,RC5_TIMER_NUM)
#define RC5_SIGNAL_OVF JOIN4(TC,RC5_TIMER_PORT,RC5_TIMER_NUM,_OVF_vect)

#define ZEIT_START_LOW  800 // (uint16_t)((800*F_CPU)/32000000)  // 6,4ms
#define ZEIT_START_HIGH ZEIT_START_LOW/2  // 3,2ms
#define ZEIT_BIT_LANG   ZEIT_START_HIGH/2  // 1,6ms


#endif // RC5_32HARDWARE_H_INCLUDED  _OVF_vect
