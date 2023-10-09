#include <RadioLib.h>
#include <string.h>
#include "sbus.h"

bfs::SbusRx sbus_rx(&Serial4);             //use SERIAL4 for Sbus RX pin for the ground trasmitter / TX for the air unit
bfs::SbusTx sbus_tx(&Serial4);
bfs::SbusData data;


//////////////pinout of the board//////////////
//////////////////DO NOT CHANGE////////////////

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
////////////////////////////////////////////////

///////////Radio params/////////////////////////
#define RadioType 1                          //1 air 0 gnd
#define Rxfreq 2425                     //rx frequncy-> different from tx
#define Txfreq 2475                     //select between 2400 and 2480 mhz
#define SPD 325                         //speed kbps-> 325 / 650 / 1300
#define TxPower -2                      //output power -2 -> 34 dB, -18 ->20 dB  you can set any power in this range (check for legality)
#define TelemID 1                       //ID of he messages for the mavlink messages (0-254)
#define SbusID 12                       //-1 if not used (additional id for sbus)
#define Sbusrate 100                    //delay in milliseconds for the sbus rate
////////////////////////////////////////////////


SX1280 radio = new Module(CSS1, DIO11, NRST1, BUSY1);   //tx pin 17
SX1280 radio2 = new Module(CSS2, DIO12, NRST2, BUSY2);  //rx pin 16


int transmissionState = RADIOLIB_ERR_NONE;
int num = 0;
const uint8_t MaxMessageSize = 112+2 ;
uint8_t Message[MaxMessageSize];
uint8_t MSG[MaxMessageSize];
int msgrx = 0;
int Rssi = 0;
int recivedBytes = 0;
unsigned long int SbusTimer = 0;
unsigned long int msgtimeTX = 0;



void setup() {
  SbusTimer = millis();
  msgtimeTX = micros();
  Serial.begin(115200);
  Serial1.begin(115200);              //set if serial output is needed at the gnd station / air port
  if(SbusID>=1){
    sbus_rx.Begin();
    sbus_tx.Begin();
    }
  pinMode(LEDrx, OUTPUT);
  pinMode(LEDtx, OUTPUT);
  if(RadioType==0){
   int state=radio.beginFLRC(Txfreq, SPD, 3, TxPower, 16, 0);    
   Serial.println(state);
   state = radio2.beginFLRC(Rxfreq, SPD, 3, TxPower, 16, 0);  
   Serial.println(state);
  }
   if(RadioType==1){
   int state=radio.beginFLRC(Rxfreq, SPD, 3, TxPower, 16, 0);    
   Serial.println(state);
   state = radio2.beginFLRC(Txfreq, SPD, 3, TxPower, 16, 0);  
   Serial.println(state);
  }
  uint8_t syncWord[] = { 0x01, 0x23, 0x45, 0x67 };
  radio.setSyncWord(syncWord, 4);
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
  if (Serial1.available() > 0 && recivedBytes < MaxMessageSize) {                //change Serial with Serial1 for the air, and vice versa
    if(recivedBytes==3){
      Message[0]=TelemID;
      Message[1]=0;
      }     
      Message[recivedBytes] = Serial1.read();                                    //change Serial with Serial1 for the air, and vice versa
      recivedBytes++;
      msgtimeTX=millis();  
  }
  
  if(Serial1.available()==0 && micros()-msgtimeTX>500 && recivedBytes!=0){       //change Serial with Serial1 for the air, and vice versa (this function sends incomplete messages after 50millis, for higherd bauds lower it)
    for(int i=recivedBytes;i<MaxMessageSize;i++){
      Message[i]=0;}
      recivedBytes=MaxMessageSize;
      msgtimeTX=millis();
  }

  if(millis()-SbusTimer>= Sbusrate && recivedBytes==0){
    Message[0]=255;          //specific for sbus messsage
    Message[1]=SbusID;

  }


  
  if (transmittedFlag == true && recivedBytes >= MaxMessageSize) {            //TX when message full and flag permits
    digitalWrite(LEDtx, HIGH);
    recivedBytes = 2;
    transmittedFlag = false;
    radio.transmit(Message, MaxMessageSize);
    //radio.finishTransmit();
    digitalWrite(LEDtx, LOW);
  }


  if (receivedFlag) {                                                        //RX packet ready to serialize every time is recived something
    // reset flag
    receivedFlag = false;
    digitalWrite(LEDrx, HIGH);
    int state2 = radio2.readData(MSG, MaxMessageSize);
    //Serial.println(radio2.getRSSI());
    Rssi = map(radio2.getRSSI(),-106,0,0,254);                              //get an rssi to build mavlink radio message,future implementation
    if(MSG[0]==TelemID){
      for (int i = 2; i < MaxMessageSize; i++) {
        Serial.print(char(MSG[i]));
        Serial1.print(char(MSG[i]));                                        //mirror msg to ext port
      }
    }
    if(MSG[0]==255 && MSG[1]==SbusID){
                                          //Sbus out
      }                       
    radio2.startReceive();  //starat reciving new msg
    digitalWrite(LEDrx, LOW);
  }
}
