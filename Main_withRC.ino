#include <RadioLib.h>
#include <string.h>
#include <PPMReader.h>
#include <mavlink.h>
#include <PulsePositionIMXRT.h>

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

#define RadioType 0      //1 air 0 gnd
#define Rxfreq 2415      //rx frequncy-> different from tx
#define Txfreq 2475      //select between 2400 and 2480 mhz
#define SPD 650          //speed kbps-> 325 / 650 / 1300
#define TxPower -2      //output power -2 -> 34 dB, -18 ->20 dB  you can set any power in this range (check for legality)
#define TelemID 25       //ID of he messages for the mavlink messages (0-254)
#define RCbusID 12       //-1 if not used (additional id for sbus)
#define RCbusrate 50     //delay in milliseconds for the sbus rate
#define RCbuschannels 8  //numers of ppm channels       mavlink max 8         ppm out max 16
#define RCbuspin 15      //Rx serial 4 pin    pins:   14, 15, 18 capable of ppm out precisely
#define RCoutype 2       //type of PPM output on air unit 1-> mavlink mix  2-> PPPm out on RXbuspin
////////////////////////////////////////////////

PPMReader ppm(RCbuspin, RCbuschannels);
PulsePositionOutput PPMout;

SX1280 radio = new Module(CSS1, DIO11, NRST1, BUSY1);   //tx pin 17
SX1280 radio2 = new Module(CSS2, DIO12, NRST2, BUSY2);  //rx pin 16


int transmissionState = RADIOLIB_ERR_NONE;
int num = 0;
const uint8_t MaxMessageSize = 120;
uint8_t Message[MaxMessageSize];
uint8_t MSG[MaxMessageSize];
int msgrx = 0;
int Rssi = 0;
int recivedBytes = 2;
unsigned long int RCbusTimer = 0;
unsigned long int msgtimeTX = 0;



void setup() {
  RCbusTimer = millis();
  msgtimeTX = micros();
  if (RCoutype == 2 && RadioType == 1) {
    PPMout.begin(RCbuspin);
  }
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  Serial.begin(115200);
  Serial1.begin(115200);  //set if serial output is needed at the gnd station / air port
  pinMode(LEDrx, OUTPUT);
  pinMode(LEDtx, OUTPUT);
  pinMode(RCbuspin, INPUT_PULLDOWN);
  if (RadioType == 0) {
    int state = radio.beginFLRC(Txfreq, SPD, 3, TxPower, 16, 0);
    Serial.println(state);
    state = radio2.beginFLRC(Rxfreq, SPD, 3, TxPower, 16, 0);
    Serial.println(state);
  }
  if (RadioType == 1) {
    int state = radio.beginFLRC(Rxfreq, SPD, 3, TxPower, 16, 0);
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
  while (Serial.available() > 0 && recivedBytes <= MaxMessageSize) {  //change Serial with Serial1 for the air, and vice versa
    if (recivedBytes == 2) {
      Message[0] = TelemID;
      Message[1] = 0;
    }
    Message[recivedBytes] = Serial.read();  //change Serial with Serial1 for the air, and vice versa
    recivedBytes++;
    msgtimeTX = micros();
  }

  if (Serial.available() == 0 && micros() - msgtimeTX > 5000 && recivedBytes > 2) {  //change Serial with Serial1 for the air, and vice versa (this function sends incomplete messages after 50millis, for higherd bauds lower it)
    for (int i = recivedBytes; i < MaxMessageSize; i++) {
      Message[i] = '\0';
    }
    recivedBytes = MaxMessageSize;
    msgtimeTX = micros();
  }

  if (millis() - RCbusTimer > RCbusrate && recivedBytes == 2 && RadioType == 0 && RCbusID != -1 && ppm.latestValidChannelValue(1, 0) != 0) {
    Message[0] = 254;  //specific for sbus messsage
    Message[1] = RCbusID;
    for (int i = 0; i < RCbuschannels * 2; i = i + 2) {
      Message[i + 2] = ppm.latestValidChannelValue((i + 2) / 2, 0) & 0xff;
      Message[i + 3] = (ppm.latestValidChannelValue((i + 2) / 2, 0) >> 8);
    }
    RCbusTimer = millis();
    recivedBytes = MaxMessageSize;
  }



  if (transmittedFlag == true && recivedBytes >= MaxMessageSize) {  //TX when message full and flag permits
    digitalWrite(LEDtx, HIGH);
    recivedBytes = 2;
    transmittedFlag = false;
    radio.transmit(Message, MaxMessageSize);
    //radio.finishTransmit()#include <PulsePositionIMXRT.h>;
    digitalWrite(LEDtx, LOW);
  }


  if (receivedFlag) {  //RX packet ready to serialize every time is recived something
    // reset flag
    receivedFlag = false;
    digitalWrite(LEDrx, HIGH);
    int state2 = radio2.readData(MSG, MaxMessageSize);
    //Serial.println(radio2.getRSSI());
    Rssi = map(radio2.getRSSI(), -106, 0, 0, 254);  //get an rssi to build mavlink radio message,future implementation
    if (MSG[0] == TelemID) {
      for (int i = 2; i < MaxMessageSize; i++) {
        Serial.print(char(MSG[i]));
        Serial1.print(char(MSG[i]));  //mirror msg to ext port
      }
    }
    if (MSG[0] == 254 && MSG[1] == RCbusID && RadioType == 1 && RCbusID != -1 && RCoutype == 1) {
      //syntetize PPM  in a mavlink message

      mavlink_message_t message;

      uint8_t target_system = 1;
      uint8_t target_component = 1;
      uint16_t ch1 = (MSG[3] << 8) | MSG[2];
      uint16_t ch2 = (MSG[5] << 8) | MSG[4];
      uint16_t ch3 = (MSG[7] << 8) | MSG[6];
      uint16_t ch4 = (MSG[9] << 8) | MSG[8];
      uint16_t ch5 = (MSG[11] << 8) | MSG[10];
      uint16_t ch6 = (MSG[13] << 8) | MSG[12];
      uint16_t ch7 = (MSG[15] << 8) | MSG[14];
      uint16_t ch8 = (MSG[17] << 8) | MSG[16];
      uint8_t buf[MAVLINK_MAX_PACKET_LEN];
      mavlink_msg_rc_channels_override_pack(255, 254, &message, target_system, target_component, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8);
      uint16_t len = mavlink_msg_to_send_buffer(buf, &message);
      Serial1.write(buf, len);
    }
    if (MSG[0] == 254 && MSG[1] == RCbusID && RadioType == 1 && RCbusID != -1 && RCoutype == 2) {
      //ppm synt
      for (int i = 0; i < RCbuschannels; i++) {
        PPMout.write(i + 1, ((MSG[i*2 + 3] << 8) | MSG[i*2 + 2]));
      }
    }
    radio2.startReceive();  //starat reciving new msg
    digitalWrite(LEDrx, LOW);
  }
}
