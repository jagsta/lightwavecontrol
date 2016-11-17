#include "config.h"
#include <LwRx.h>
#include <LwTx.h>
#include <RCSwitch.h>
#include <Ethernet.h>
#include <SPI.h>
#include <PubSubClient.h>

#define LWrxPin 2
#define LWtxPin 3
#define RCSrxPin 4
#define RCStxPin 5
#define txMultiplier 3
#define invert 0
#define uSecTick 140
#define RCSuSec 326
#define RCSProtocol 1
#define RCSRepeat 10
#define noSerial 1

// pointer to the debug message
const char* debugmessage;

//433 data
byte msg[10];
byte msglen = 10;

//MQTT message
byte mqttmsg[11];

// Transmitter data
//byte transmitter[6];
byte room;
byte command;
byte dimlevel[2];
byte subunit;
char message_buff[11];

// declare the callback then setup the MQTT client, we'll be referencing soon
void processMQTT(char* topic, byte* payload, unsigned int length); 
EthernetClient ethClient;
PubSubClient client(server, 1883, processMQTT, ethClient);
RCSwitch mySwitch = RCSwitch();

/* 
  function to convert ascii to true hex values
*/
byte h2d(byte hex)
{
        if(hex > 0x39) hex -= 7; // adjust for hex letters upper or lower case
        return(hex & 0xf);
}

void sendDebug(char const * message) {
  if (noSerial)
  {
    client.publish(debugtopic, message);
  }
  else
  {
    Serial.println(message);
  };
};

void processMQTT(char* topic, byte* payload, unsigned int length) {
  byte temp[5];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  for (int i=0;i<length;i++) {
    temp[i] = h2d(payload[i]);
  }
  if (strcmp(topic, LWsubtopic) == 0)
  {
    sendDebug("Lightwave command received");
  }
  else if (strcmp(topic, RCSsubtopic) == 0)
  {
    sendDebug("RC Switch command received");
  };

//  if (length == 11) {
    byte sendMsg[] = {temp[3], temp[4], temp[1], temp[2], id[0], id[1], id[2], id[3], id[4], temp[0]};
//    Serial.println("message prepped");
    if (lwtx_free()) {
//      Serial.println("sending message");
      lwtx_send(sendMsg);
    }
//  }
  // Send an acknowledge response to client

  // else message invalid, send error response to client
}

void setup() {
   // set up with rx into pin 2, tx into pin 3
   Serial.begin(9600);
   Serial.println("Initialising Ethernet Stack and connecting to MQTT");
   Ethernet.begin(mac, ip, gateway, gateway, netmask);
   client.connect(clientId, username, password);
   client.subscribe(LWsubtopic);
   client.subscribe(RCSsubtopic);
   Serial.println("Initialsing Lightwave 433 RX Module");
   lwrx_setup(LWrxPin);
   Serial.println("Initialsing Lightwave 433 TX Module");
   lwtx_setup(LWtxPin, txMultiplier, invert, uSecTick);
   Serial.println("Initialsing Generic 433 TX Module");
   mySwitch.enableTransmit(RCStxPin);
   mySwitch.setProtocol(RCSProtocol);
   mySwitch.setPulseLength(RCSuSec);
   mySwitch.setRepeatTransmit(RCSRepeat);
   Serial.println("Set up completed");
}

/**
   bit maths
**/
int NibbleCombine( byte msb, byte lsb ) {
   int sum;
//   sum = msb;
   sum = msb<<4;
   sum |= lsb;
   return sum;
}

/**
   Retrieve and print out received message
**/
void printMsg(byte *msg, byte len) {
   for(int i=0;i<len;i++) {
      Serial.print(msg[i],HEX);
      Serial.print(" ");
   }
/*   for (int i=4;i<len;i++) {
      transmitter[i-4] = msg[i];
   }
*/
   subunit = msg[2];
   command = msg[3];
   for (int i=0;i<2;i++) {
      dimlevel[i] = msg[i];
   }
/*
*/
   Serial.print("dimlevel is ");
   int dim_as_int=NibbleCombine(dimlevel[0],dimlevel[1]);
   Serial.print(dim_as_int);
   Serial.print("command is ");
   if (command == 0) {
     Serial.println("off");
   }
   else if (command == 1) {
     Serial.println("on");
   };
   Serial.print("subunit is ");
   Serial.println(subunit);

}

String test;
void loop() {
   //collect any incoming command message and execute when complete
   if (lwrx_message()) {
     test = "";
      lwrx_getmessage(msg, msglen);
      for (int i=0;i<msglen;i++){
        test += String(msg[i],HEX);
      }
      test.toCharArray(message_buff, test.length()+1);
      client.publish(LWsnooptopic,message_buff);
      printMsg(msg, msglen);
   }
   client.loop();
}

