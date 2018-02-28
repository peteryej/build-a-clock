#include "ntp.h"

#include <Arduino.h>
#include <assert.h>

// stuff due time into request packet
void ntp_get_request(DueTime_t *t0, NtpPacket_t *packet){
  memset(packet, 0, SNTP_MSG_LEN);
  
  packet->li_vn_mode = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;
  
  // would you want to add in other info here?
  
  // place t0 into transmit_timestamp
  packet->transmit_timestamp[0] = lwip_htonl(t0->sec + DIFF_SEC_1900_1970);
  packet->transmit_timestamp[1] = lwip_htonl(USEC_TO_FRAC(t0->usec));
}

void ntp_get_time_from_packet(NtpPacket_t *packet, DueTime_t *t0, DueTime_t *t1, DueTime_t *t2){
  t0->sec = lwip_ntohl(packet->originate_timestamp[0]) - DIFF_SEC_1900_1970;
  t0->usec = FRAC_TO_USEC(lwip_ntohl(packet->originate_timestamp[1]));
  
  t1->sec = lwip_ntohl(packet->receive_timestamp[0]) - DIFF_SEC_1900_1970;
  t1->usec = FRAC_TO_USEC(lwip_ntohl(packet->receive_timestamp[1]));
  
  t2->sec = lwip_ntohl(packet->transmit_timestamp[0]) - DIFF_SEC_1900_1970;
  t2->usec = FRAC_TO_USEC(lwip_ntohl(packet->transmit_timestamp[1]));
}
