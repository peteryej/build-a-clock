#ifndef __CLOCK_H
#define __CLOCK_H

#if defined(_SAM3XA_)

#include "DueTime.h"

class DueClock {
  int32_t _seconds;
public:
  void init();
  
  void start();
  void stop();
  void reset();
  void tick();
  
  void getTime(DueTime_t *t);
};

extern DueClock Clock;

#else

#error "Are you compiling for the PowerDue?"

#endif

#endif
