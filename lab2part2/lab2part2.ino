//#include <PowerDue.h>
#include "DueClock.h" 
#include <IPAddress.h>
#include "ntp.h"
#include <PowerDueWiFi.h>


#define WAITIME_MS 5000
#define WIFI_SSID "PowerDue"
#define WIFI_PASS "powerdue"


diffTime_t offset;
diffTime_t roundtripDelay;

void computeOffset(DueTime_t *t0, DueTime_t *t1, DueTime_t *t2, DueTime_t *t3) {
  
  double diffSec1, diffSec2, diffuSec1, diffuSec2;
  diffSec1 = (double)(t1->sec) - (double)(t0->sec);
  diffSec2 = (double)(t2->sec) - (double)(t3->sec);
  offset.sec = (diffSec1+diffSec2)/2;
  
  diffuSec1 = (double)(t1->usec) - (double)(t0->usec);
  diffuSec2 = (double)(t2->usec) - (double)(t3->usec);
  if(diffuSec1<0){
    diffuSec1 = 1000000+ diffuSec1;
  }
  if(diffuSec2<0){
    diffuSec1 = 1000000+ diffuSec1;
  }
  offset.usec = (diffuSec2-diffuSec1)/2;
  //return offset;
}

void computeDelay(DueTime_t *t0, DueTime_t *t1, DueTime_t *t2, DueTime_t *t3) {
  double diffSec1, diffSec2, diffuSec1, diffuSec2;
  diffSec1 = (double)(t3->sec) - (double)(t0->sec);
  diffuSec1 = (double)(t3->usec) - (double)(t0->usec);

  if(diffuSec1<0){
    diffuSec1 = 1000000+ diffuSec1;
    diffSec1 -= 1;
  }

  diffSec2 = (double)(t2->sec) - (double)(t1->sec);
  diffuSec2 = (double)(t2->usec) - (double)(t1->usec); 
  
  if(diffuSec2<0){
    diffuSec2 = 1000000+ diffuSec2;
    diffSec2 -= 1;
  }

  
  roundtripDelay.sec = diffSec1-diffSec2;
  roundtripDelay.usec = diffuSec1-diffuSec2;

  if(roundtripDelay.usec<0){
    roundtripDelay.usec = 1000000+ roundtripDelay.usec;
    roundtripDelay.sec -= 1;
  }  
  //return roundtripDelay;
}

int ntp_build_socket(void){ 
  struct sockaddr_in addr; 
  memset(&addr, 0, sizeof(addr)); 
  addr.sin_len = sizeof(addr); 
  addr.sin_family = AF_INET; 
  addr.sin_port = htons(NTP_PORT);   // defined in ntp.h as 123 
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 

  int sock = lwip_socket(AF_INET, SOCK_DGRAM, 0); 
  if(lwip_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0){ 
    SerialUSB.println("Failed to bind ntp socket :("); 
    while(1); 
  } 
  return sock; 
}


void ntp_send_task(void *arg){ 
  int socket = (int)arg; 
  NtpPacket_t packet; 
  DueTime_t t0; 
  struct sockaddr_in serverAddr; 
  memset(&serverAddr, 0, sizeof(serverAddr)); 
  serverAddr.sin_len = sizeof(serverAddr); 
  serverAddr.sin_family = AF_INET; 
  serverAddr.sin_port = htons(NTP_PORT); // defined as 123 
  // NTP_SERVER defined as "10.230.0.1" 
  inet_pton(AF_INET, NTP_SERVER, &(serverAddr.sin_addr)); 
  
  while(1){ 
    /* Sending NTP Request */ 
    Clock.getTime(&t0); // get current time 
    ntp_get_request(&t0, &packet); // build a packet 
    lwip_sendto(socket, 
                &packet, 
                SNTP_MSG_LEN, // defined in ntp.h 
                0, 
                (const sockaddr *)&serverAddr, 
                sizeof(serverAddr) 
                ); 
    // wait for some amount of time before sending next request 
    // define your own WAITTIME_MS to set an interval 
    vTaskDelay(pdMS_TO_TICKS(WAITIME_MS)); 
  }
}

void ntp_receive_task(void *arg){ 
  /* Build a UDP socket */ 
  int socket = ntp_build_socket(); 

  /* Kick off an NTP sending task and pass the socket as an arg*/ 
  xTaskCreate(ntp_send_task, "ntp_send", configMINIMAL_STACK_SIZE * 5, (void *)socket, 2, NULL); 

  NtpPacket_t packet; 
  DueTime_t t0, t1, t2, t3; 
  struct sockaddr addr; 
  socklen_t socklen; 
  
  while(1){ 
    memset(&packet, 0, SNTP_MSG_LEN); 
    lwip_recvfrom(socket, &packet, SNTP_MSG_LEN, 0, &addr, &socklen); 
    // message received from addr 
 
    // get receive time: t3 
    Clock.getTime(&t3); 
    
    // extract tO, t1 and t2 from packet 
    ntp_get_time_from_packet(&packet, &t0, &t1, &t2); 

    
    // now what? can you apply the NTP formula to compute offset and delay? 
    computeOffset(&t0, &t1, &t2, &t3); 

    SerialUSB.print("offset: ");
    SerialUSB.print(offset.sec);
    SerialUSB.print(" s ");
    SerialUSB.print(offset.usec);
    SerialUSB.println(" us");

    computeDelay(&t0, &t1, &t2, &t3); 
    SerialUSB.print("delay: ");
    SerialUSB.print(roundtripDelay.sec);
    SerialUSB.print(" s ");
    SerialUSB.print(roundtripDelay.usec);
    SerialUSB.println(" us");
    // What should be the criteria for adjusting our clock? 
    //Clock.addOffset(&offset);
    
    print_time("t0", &t0);
    print_time("t1", &t1);
    print_time("t2", &t2);
    print_time("t3", &t3);
  }
}



void onReady(){
  SerialUSB.println("Device ready");  
  SerialUSB.print("Device IP: ");
  SerialUSB.println(IPAddress(PowerDueWiFi.getDeviceIP()));  
  
  xTaskCreate(ntp_receive_task, "ntp_receive_task", configMINIMAL_STACK_SIZE*5, NULL, 4, NULL);
}

void onError(int errorCode){
  SerialUSB.print("Error received: ");
  SerialUSB.println(errorCode);
}

void print_time(String tag, DueTime_t *t){ 
  SerialUSB.print(tag); 
  SerialUSB.print(": "); 
  SerialUSB.print(t->sec); 
  SerialUSB.print("s "); 
  SerialUSB.print(t->usec); 
  SerialUSB.print("us"); 
  SerialUSB.println(); 
} 

void initLEDPins(){
  // turn off LEDs
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  turnOffLED();
}

void turnOffLED(){
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);
  digitalWrite(8,LOW);
}

void setup() { 
  //PowerDue.LED(); 
  //PowerDue.LED(PD_OFF); 
  initLEDPins();
  
  SerialUSB.begin(0); 
  while(!SerialUSB); // wait for serial port to open 
  SerialUSB.println("SerialUSB is ready");

  Clock.init(); 
  Clock.start(); 
 //enable export of the clock on A4
  PIO_Configure(PIOA, PIO_PERIPH_A, PIO_PA2A_TIOA1, PIO_DEFAULT);

  PowerDueWiFi.init(WIFI_SSID, WIFI_PASS);
  PowerDueWiFi.setCallbacks(onReady, onError);

  vTaskStartScheduler();
  SerialUSB.println("Insufficient RAM");
  while(1);
} 

void loop() { 
//  delay(1000); 
//  DueTime_t t; 
//  Clock.getTime(&t); 
//  SerialUSB.print("Time now is: "); 
//  SerialUSB.println(t.sec); 
 } 
 
