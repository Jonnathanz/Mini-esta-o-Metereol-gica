#include "umidade.h"
#include "SENSOR_TEMPERATURA.h"


char __temp[4];
char __umid[6];
char __ilum[6];

#include "freeMemory.h"
#include "ENC28J60.h"
//#include "TCPserver.h"

void setup(){
  unsigned char _macAddress[] = {0x01,0x02,0x03,0x04,0x05,0x06};
  unsigned char _IPaddress[] = {192,168,100,200};
  //unsigned char _portTCP[] = {0xc3, 0x50};  // 50000
  unsigned char _portTCP[] = {0x1f,0x92};  // 8083
  Serial.begin(9600);
  init_ENC28J60(_macAddress, _IPaddress);
  init_TCPserver(_portTCP);
}

void loop() {
 // delayMicroseconds(100);
  read_bufferMemory_ENC28J60();   
  //char data_send[60];
  temperature(__temp);
  UMIDADE(__umid);
  SENSOR_LUMINOSIDADE(__ilum);
  //sSerial.println(__temp);
  //delay(1000);
  /*
  Serial.println("\n");
  SENSOR_LUMINOSIDADE(data_send+35);
  Serial.println(data_send);
  Serial.println("\n");
  temperature(data_send+16); 
  Serial.println(data_send);
  Serial.println("\n\n\n\n");
  */
}
