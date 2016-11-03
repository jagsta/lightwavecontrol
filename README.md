# lightwavecontrol
Arduino C++ code to control lightwaverf via MQTT

I'm fed up with the lightwave wifi controller, it sucks, so i've decided to replace it with an Arduino and a pair of cheap (£1)
433MHz rx/tx modules. I could have controlled it via the serial port, but I'm not sure what coverage will be like, so
i've opted to use the ethernet shield and use MQTT instead, so I can run these standalone around the house as needed.

The beauty of MQTT is that I can have several of these subscribed to the same command queue, so it'll scale if I need to add more
of them to cover all areas of the house without having to modify the code or the proxy which posts the commands to MQTT.

I use a simple nodejs proxy to take HTTP GET requests from either domoticz, ha-bridge, my phone and put the room, subunit, command and dimlevel 
to the MQTT message queue. Since I started off with a wifi controller I use the same constructs, so my requests look like:

http://<nodejs-proxy-ip>:<listenport>/?device=R1D1&command=(on|off|dimlevel 0-31)

See https://github.com/jagsta/lightwaveproxy/blob/master/lightwaveproxy-mqtt.js

The arduino receives an 11 byte string:

room (hex 1-f)
subunit (hex 1-f)
command (0/1)
dimlevel (hex 1-f)
requestId (48 bits as 6 hex nibbles)

Then forms a lightwave command using the command id configured in the include file config.cpp, and sends that using the 433 tx module

As a side benefit the arduino also receives any lightwave commands sent by itself or other devices, this allows me to keep tabs on
slave light switches, remote controllers etc and update domoticz with state (of course this is inferred, since lightwave is 1-way communication).
