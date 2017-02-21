#include "config.h"
#include <LightwaveRF.h>
#include <RCSwitch.h>
#include <Ethernet.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define LWrxPin 2
#define LWtxPin 3
#define LWInterrupt 0
#define RCSrxPin 4
#define RCStxPin 5
#define txMultiplier 3
#define invert 0
#define uSecTick 140
#define RCSuSec 326
#define RCSProtocol 1
#define RCSRepeat 10
#define useSerial 1


// char array to build debug messages
char debugmessage[100];

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

/*
  function to take a string and return in a format client.publish will accept
*/



/*
  function to either log to serial or MQTT debug channel if headless
*/
void sendDebug(char const * message) {
  if (useSerial)
  {
    Serial.println(message);
  }
  else
  {
    client.publish(debugtopic, message);
  };
};

void processMQTT(char* topic, byte* payload, unsigned int length) {
  // first copy the payload to a temp byte array as any MQTT operations will overwrite the payload
  // lw payloads are 5 bytes, RCswitch payloads are 3 bytes padded
  // format is room, device, command, dim, dim (2 bytes)
  // plus 6 bytes of requestid
  byte req[5];
  for (int i=0;i<5;i++) {
    req[i] = h2d(payload[i]);
  }
  byte reqId[6];
  for (unsigned int i=5;i<length;i++) {
    reqId[i] = h2d(payload[i]);
  }
  if (strcmp(topic, RCSsubtopic) == 0)
  {
    sendDebug("RC Switch command received:");
    sendDebug((char *)req);
  }
  else if (strcmp(topic, LWsubtopic) == 0)
  {
    sendDebug("Lightwave command received:");
    sendDebug((char *)req);
//  if (length == 11) {
    byte sendMsg[] = {req[3], req[4], req[1], req[2], id[0], id[1], id[2], id[3], id[4], req[0]};
    lw_send(sendMsg);
//  }
  // Send an acknowledge response to client
  client.publish(LWpubtopic, "holding comment - fix me ");

  // else message invalid, send error response to client
  };
}

void setup() {
   // set up with rx into pin 2, tx into pin 3
   if (useSerial) {
     Serial.begin(9600);
     Serial.println("Initialising Ethernet Stack and connecting to MQTT");
   };
   Serial.print("Connecting to ");
   Serial.println(wifissid);

//   WiFi.config(ip, gateway, subnet);
   WiFi.begin(wifissid, wifipass);

   while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
   }
   Serial.println("");
   Serial.println("WiFi connected");
   client.connect(clientId, username, password);
   client.subscribe(LWsubtopic);
   client.subscribe(RCSsubtopic);
   sendDebug("Initialsing Lightwave 433 RX Module");
   lw_rx_setup(LWrxPin, LWInterrupt);
   sendDebug("Initialsing Lightwave 433 TX Module");
   lw_tx_setup(LWtxPin);
   sendDebug("Initialsing Generic 433 TX Module");
   mySwitch.enableTransmit(RCStxPin);
   mySwitch.setProtocol(RCSProtocol);
   mySwitch.setPulseLength(RCSuSec);
   mySwitch.setRepeatTransmit(RCSRepeat);
   sendDebug("Set up completed");
}

/**
   bit maths
**/
int NibbleCombine( byte msb, byte lsb ) {
   int sum;
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
   if (lw_have_message()) {
     test = "";
      lw_get_message(msg, &msglen);
      for (int i=0;i<msglen;i++){
        test += String(msg[i],HEX);
      }
      test.toCharArray(message_buff, test.length()+1);
      client.publish(LWsnooptopic,message_buff);
//      printMsg(msg, msglen);
   }
   client.loop();
}

