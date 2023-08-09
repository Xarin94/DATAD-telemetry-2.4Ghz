#include <RadioLib.h>
#include <string.h>

#define LEDtx 9
#define LEDrx 2
#define CSS1 3
#define DIO11 6
#define NRST1 4
#define BUSY1 5
#define CSS2 10
#define DIO12 19
#define NRST2 21
#define BUSY2 20

SX1280 radio = new Module(CSS1, DIO11, NRST1, BUSY1);   //tx pin 17
SX1280 radio2 = new Module(CSS2, DIO12, NRST2, BUSY2);  //rx pin 16
//EasyNex myNex(Serial1);


int transmissionState = RADIOLIB_ERR_NONE;
int num = 0;
const uint8_t MaxMessageSize = 112 ;
uint8_t Message[MaxMessageSize];
uint8_t MSG[MaxMessageSize];
uint8_t MessageCR[16];
uint8_t MessageCRX[16];
uint8_t MSGCR[16];
uint8_t MSGCRX[16];
int msgrx = 0;
int Rssi = 0;
int recivedBytes = 0;
unsigned long int Time = 0;
unsigned long int msgtimeTX = 0;
int kbps = 0;



void setup() {
  Time = millis();
  msgtimeTX = millis();
  Serial.begin(115200);
 Serial1.begin(115200);              //set if serial output is needed at the gnd station / air port
    pinMode(LEDrx, OUTPUT);
    pinMode(LEDtx, OUTPUT);
  // initialize SX1280 with default settings
  int state=radio.beginFLRC(2455.0, 325, 3, 0, 16, 0);    //TX AIR  rx gnd    2415.0
  Serial.println(state);
  state = radio2.beginFLRC(2455.0, 325, 3, 0, 16, 0);  // RX AIR  tx gnd
  Serial.println(state);
  uint8_t syncWord[] = { 0x01, 0x23, 0x45, 0x67 };
  radio.setSyncWord(syncWord, 4);
  uint8_t syncWord2[] = { 0x11, 0x53, 0x44, 0x97 };
  radio2.setSyncWord(syncWord, 4);
  radio.setDio1Action(setFlagTX);
  radio2.setDio1Action(setFlagRX);
  radio2.setHighSensitivityMode(true);
  radio2.startReceive();
}

volatile bool transmittedFlag = true;
volatile bool receivedFlag = false;

void setFlagTX(void) {

  transmittedFlag = true;
}

void setFlagRX(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void loop() {
  while (Serial.available() > 0 && recivedBytes < MaxMessageSize) {
    
      Message[recivedBytes] = Serial.read();
      recivedBytes++;
      msgtimeTX=millis();
    
  }
  if(Serial.available()==0 && millis()-msgtimeTX>50 && recivedBytes!=0){
    for(int i=recivedBytes;i<MaxMessageSize;i++){
      Message[i]=0;}
      recivedBytes=MaxMessageSize;
      msgtimeTX=millis();
    }
  
  if (transmittedFlag == true && recivedBytes >= MaxMessageSize) {  //TX when message full and flag permits
    digitalWrite(LEDtx, HIGH);
    recivedBytes = 0;
    transmittedFlag = false;
    radio.transmit(Message, MaxMessageSize);
    //radio.finishTransmit();
    digitalWrite(LEDtx, LOW);
  }


  if (receivedFlag) {  //RX packet ready to serialize every time is recived something
    // reset flag
    receivedFlag = false;
    digitalWrite(LEDrx, HIGH);
    int state2 = radio2.readData(MSG, MaxMessageSize);
    //Serial.println(radio2.getRSSI());
    Rssi = map(radio2.getRSSI(),-106,0,0,254);
      for (int i = 0; i < MaxMessageSize; i++) {
        //Serial.print(char(MSG[i]));
        Serial.print(char(MSG[i]));
        Serial1.print(char(MSG[i]));//mirror msg to ext port
      }                     
    radio2.startReceive();  //starat reciving new msg
    digitalWrite(LEDrx, LOW);
    
  }
 
}
