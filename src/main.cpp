#include "config.h"
#include <LwRx.h>
#include <LwTx.h>
#include <Ethernet.h>
#include <SPI.h>
#include <PubSubClient.h>

#define rxPin 2
#define txPin 3
#define txMultiplier 3
#define invert 0
#define uSecTick 140

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

/* 
  function to convert ascii to true hex values
*/
byte h2d(byte hex)
{
        if(hex > 0x39) hex -= 7; // adjust for hex letters upper or lower case
        return(hex & 0xf);
}

void processMQTT(char* topic, byte* payload, unsigned int length) {
  byte temp[5];
/*
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
*/
  for (int i=0;i<length;i++) {
    temp[i] = h2d(payload[i]);
  }
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

EthernetClient ethClient;
PubSubClient client(server, 1883, processMQTT, ethClient);

void setup() {
   // set up with rx into pin 2, tx into pin 3
   Serial.begin(9600);
   Serial.println("Initialising Ethernet Stack and connecting to MQTT");
   Ethernet.begin(mac, ip, gateway, gateway, netmask);
   client.connect(clientId, username, password);
   client.subscribe(subtopic);
   Serial.println("Initialsing 433 Module");
   lwrx_setup(rxPin);
   lwtx_setup(txPin, txMultiplier, invert, uSecTick);
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
      client.publish(snooptopic,message_buff);
      printMsg(msg, msglen);
   }
   client.loop();
}

