#ifndef __ntp_h
#define __ntp_h

#include <lwip.h>

#include "DueTime.h"

#define NTP_SERVER    "10.230.0.1"
#define NTP_PORT      123

/* SNTP protocol defines */
#define SNTP_MSG_LEN                48

#define SNTP_OFFSET_LI_VN_MODE      0
#define SNTP_LI_MASK                0xC0
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_LI_LAST_MINUTE_61_SEC  0x01
#define SNTP_LI_LAST_MINUTE_59_SEC  0x02
#define SNTP_LI_ALARM_CONDITION     0x03 /* (clock not synchronized) */

#define SNTP_VERSION_MASK           0x38
#define SNTP_VERSION                (4/* NTP Version 4*/<<3)

#define SNTP_MODE_MASK              0x07
#define SNTP_MODE_CLIENT            0x03
#define SNTP_MODE_SERVER            0x04
#define SNTP_MODE_BROADCAST         0x05

#define SNTP_OFFSET_STRATUM         1
#define SNTP_STRATUM_KOD            0x00

#define SNTP_OFFSET_ORIGINATE_TIME  24
#define SNTP_OFFSET_RECEIVE_TIME    32
#define SNTP_OFFSET_TRANSMIT_TIME   40

/* number of seconds between 1900 and 1970 (MSB=1)*/
#define DIFF_SEC_1900_1970         (2208988800UL)
/* number of seconds between 1970 and Feb 7, 2036 (6:28:16 UTC) (MSB=0) */
#define DIFF_SEC_1970_2036         (2085978496UL)

/* Fractions to microseconds conversion */
/* microseconds per second */
#define MICROSECONDS 1000000
#define FRAC    4294967296.0    /* 2^32 as a double */
#define USECFRAC	(FRAC / MICROSECONDS)
#define FRAC_TO_USEC(tsf)						\
	((int32_t)((tsf) / USECFRAC + 0.5))
#define USEC_TO_FRAC(tvu)						\
	((uint32_t)((tvu) * USECFRAC + 0.5))

typedef struct NtpTimestamp_t {
  uint32_t seconds;
  uint32_t frac;      // 32bit-fraction of a second
} __attribute__ ((packed));

typedef struct NtpPacket_t {
  uint8_t li_vn_mode;
  uint8_t stratum;
  uint8_t poll;
  uint8_t precision;
  uint32_t root_delay;
  uint32_t root_dispersion;
  uint32_t reference_identifier;      // client clock identifier
  uint32_t reference_timestamp[2];    // last time the clock was synchronized
  uint32_t originate_timestamp[2];    // t0 - from client
  uint32_t receive_timestamp[2];      // t1 - from server
  uint32_t transmit_timestamp[2];     // t2 - from server
} __attribute__ ((packed)); 

void ntp_get_request(DueTime_t *t0, NtpPacket_t *packet);
void ntp_get_time_from_packet(NtpPacket_t *packet, DueTime_t *t0, DueTime_t *t1, DueTime_t *t2);

#endif  //__ntp_h
