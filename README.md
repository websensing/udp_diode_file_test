# Diode Test Specification
Version 1.3  
Authors: Stephen Taylor & James Brock

This directory contains a UDP file transfer server/client program for testing transmitting a file across a Web Sensing Diode and outlines the testing specification. 


## Pre-requisites

Copy this entire directory to Linux servers on both sides of the diode
and build each of the respective codes in the server subdirectories by running:

```
> make clean
> make
```

UDP diodes only support transport using their respective protocols, consequently neither ICMP nor DHCP will be able to operate, and the arp cashe must be configured. To do this a shell scipt arpconf.sh is provided. It must be run on the servers on either side of the diode. The arpconf script usage is below. 

The script should be run as below on the "inside" server that will be transmitting across the diode.
```
> sudo ./arpconf.sh in  <outside ip> <outside mac>
```

The script should be run as below on the "outside" server that will be receiving from diode
```
> sudo ./arpconf.sh out <inside ip> <inside mac>
```


## Executable Test Programs
  
**us** -- udp server  
**uc** -- udp client


## Program Usage

### Client Usage

```
-h -- display usage  
-ip <ipaddress> <port> -- set server ip in client (default: 127.0.0.1 in client)  
-f <file> -- used in a client to send messages from a file
-t <sec> <ms> -- timeout after secs+ms (default 3.0)
-v -- verbose messaging
-ack -- Require acknowledgement packet from the server side for each packet
```


### Server Usage

```
-h -- display usage  
-p <serverport> -- set server ip in client (default: 127.0.0.1 in client)  
-v -- verbose messaging
-ack -- Require acknowledgement packet from the server side for each packet
```


### Running the Executables

The commands provided here show how to run the UDP server and client. On the "outside", recieving side, of the diode run the server:

```
./us -p 9001 -v
```

On the "inside", sending side, of the diode run the client:

```
./uc -f 1mb.test -ip <IP_OF_OUTSIDE> 9001 -v
```

This command sends the file ./tests/1mb.test accross the diode on
port 9001. On both the client and server, diagnostic messages will
be printed out as packets are transmitted and received.

Both programs provide help with -h flag (e.g. ./uc -h).


### Testing Proceedure

#### Hardware Set
Place both UDP servers on each side of the diode serving on unique ports -- defaults are already provided in the source (use the -h flag to see the defaults).

#### Verify ICMP Blocked
Using ping, verify that ICMP messsages do not traverse the diodes in either direction.

#### Verify TCP Blocked
Using "netcat", attempt to transmit TCP packets across the diode. On the "outside" server, run the following command:
```
> netcat -l -p 9001
```

On the "inside" server, run the following command:
```
> netcat <IP_OF_OUTSIDE> 9001
```

Then, type anything in the "inside" server prompt and verify that it does not appear on the server side.

#### Verify ARP Blocked
If you have already run "arpconf.sh", clear the ARP cache by running
```
> arp -d <IP_OF_OUTSIDE>
```

Then, run the server commands below:
```
On Outside Server
> ./us -p 9001 -v
```
```
On Inside Server
./uc -f 1mb.test -ip <IP_OF_OUTSIDE> 9001 -v
```

#### Verify UDP Passes
Set up the ARP table by running the "arpconf.sh" script as described in the *Prerequisites* section. Then, run the server commands below:
```
On Outside Server
> ./us -p 9001 -v
```
```
On Inside Server
./uc -f 1mb.test -ip <IP_OF_OUTSIDE> 9001 -v
```

These commands will, on the "inside" server, get the file ./test/1mb.test, which contains 1MB+ of randomly generated data and transmit it in 16kB chunks across the diode with a 1 second pause in between chunks. Once the entire file is complete, the program will exit. On the "outside" server, the program will listen for the 16kB chunks, assemble them into a file ./1mb.test. This test can be verified by doing a diff on the two files. They should max exactly. 




