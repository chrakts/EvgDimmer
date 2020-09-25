#ifndef LAMPSTATUS_H_INCLUDED
#define LAMPSTATUS_H_INCLUDED

struct lampStatus
{
  uint8_t lamps = 0;
  uint8_t dimmer[3];
/*  uint8_t minDimmer[3];
  uint8_t maxDimmer[3];*/
  uint8_t checksum;
};

typedef struct lampStatus LAMP_STATUS;



#endif // LAMPSTATUS_H_INCLUDED
