# linux-netutils
A collection of small Linux networking utilities:
- nbr
- owd
- sit-ctl
- ssh-door
- udp-tun
- lte-rx

## nbr
### Non-blocking resolver.
It reads domain names from stdin and resolves bunches of them to IP addresses in parallel.

## owd
### A tool to measure one-way network transmission delay.
It sends a probe packet to the remote host, which records its arrival time and sends it back in the reply packet. To get meaningful results, both local and remote hosts must keep accurate time synchronized with an independent source.

## sit-ctl
### A tool to remotely change the endpoint of a sit tunnel interface.
If invoked as a server, it waits for the incoming requests and changes the remote endpoint of the tracked sit interface to the source address of the request packet. If invoked as a client, it sends such request to the server.

## ssh-door
### A tool to allow SSH connections from a client who successively opened a secret URL.
The program adds client's source address to a list managed by xt_recent iptables extension. To be able to do so, it must be set up as a setuid root CGI application. Surely, its URL should require authentication, and the transport should be HTTPS.

## udp-tun
### Forwards IPv4 or IPv6 over UDP with support for dynamic endpoints.
The program creates tunnel interfaces and connects them with UDPv4 packets. Addresses and routing have to be set up manually later. The server keeps track of changes of the client address by simply using the last address seen. The program provides no authentication, integrity checks, or encryption, and therefore cannot withstand any deliberate attack. It should be used in controlled environments only.

## lte-rx
### Prints LTE receive signal parameters.
This program queries a Huawei LTE USB modem (ME909 or similar), and prints out signal strength statistics - RSSI, RSRP, SINR, and RSRQ - in a human-readable form.

## Usage
For any questions on command line options, constants, paths etc. refer to the source.
