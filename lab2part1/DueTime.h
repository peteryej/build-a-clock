#ifndef __DUETIME_H
#define __DUETIME_H

#include <inttypes.h>

struct DueTime_t {
  int32_t sec;     // seconds
  uint32_t usec;    // microseconds
};

#endif //__DUETIME_H
