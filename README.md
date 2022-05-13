# Diode Test Specification
Version 4.3  
Author: Stephen Taylor  

This directory contains a UDP file transfer server/client program for testing and 
outlines the testing specification. 

## Executable Test Programs
  
**us** -- udp server  
**uc** -- udp client

## Program Usage

### Client Usage

**-h** -- display usage  
**-ip <ipaddress> <port>** -- set server ip in client (default: 127.0.0.1 in client)  
**-f <file>** -- used in a client to send messages from a file
**-t <sec> <ms>** -- timeout after secs+ms (default 3.0)
**-v** -- verbose messaging
**-ack** -- Require acknowledgement packet from the server side for each packet

### Server Usage

**-h** -- display usage  
**-p <serverport>** -- set server ip in client (default: 127.0.0.1 in client)  
**-v** -- verbose messaging
**-ack** -- Require acknowledgement packet from the server side for each packet

## Preliminaries

Copy this entire directory to Linux servers on both sides of the diode
and build each of the respective codes in the server subdirectories by running:

**make clean**  
**make**  

UDP diodes only support transport using their respective
protocols, consequently neither ICMP nor DHCP will be able to operate,
and the arp cashe must be configured. To do this a shell scipt
arpconf.sh is provided. It must be modified to set up the mac
addresses of servers on either side of the diode to communicate
accross the diode and is then executed using:

**sudo ./arpconf.sh**  


### Running the Executables

The commands provided here show how to run the UDP server and
client. However, similar commands are used for running the TCP server
and client (i.e. replace us with ts, and uc with tc):

On the recieving side of the diode run the server:

**./us -p 9001 -v -ack**

On the sending side of the diode run the client:

**./uc -f 1mb.test -ip \<IP_OF_SERVER\> 9001 -v -ack**  

This command sends the file ./tests/1mb.test accross the diode on
port 9001. On both the client and server, diagnostic messages will
be printed out as packets are transmitted and acknowledged.

Both programs provide help with -h flag (e.g. ./uc -h).


### Testing Proceedure

1) Place both UDP servers on each side of the diode serving on
unique ports -- defaults are already provided in the source (use the
-h flag to see the defaults).

2) Using ping, verify that ICMP messsages do not traverse the diodes in
either direction.

3) **For a UDP diode:**

Verify that messages travel accross the diode only in the intended
direction by sending a variety of messages, of increasing length, from
both sides of the diode.

In addition, verify that no TCP messages pass in either direction.




