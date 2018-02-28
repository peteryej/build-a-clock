#ifndef __CLOCK_H
#define __CLOCK_H

#if defined(_SAM3XA_)

#include "DueTime.h"
//#include <PowerDue.h>

struct diffTime_t{
  double sec;
  double usec;   //different from DueTime_t
};


class DueClock {
  uint32_t _seconds;
  diffTime_t _offset;
public:
  void init();
  
  void start();
  void stop();
  void reset();
  void tick();
  void addOffset(diffTime_t *offset);
  void getTime(DueTime_t *t);
};

extern DueClock Clock;

#else

#error "Are you compiling for the PowerDue?"

#endif

#endif
