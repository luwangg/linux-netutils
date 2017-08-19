# linux-netutils
A collection of small Linux networking utilities:
- sit-ctl
- ssh-door
- udp-tun

## sit-ctl
### A tool to remotely change the endpoint of a sit tunnel interface.
If invoked as a server, it waits for the incoming requests and changes the remote endpoint of the tracked sit interface to the source address of the request packet. If invoked as a client, it sends such request to the server.

## ssh-door
### A tool to allow SSH connections from a client who successively opened a secret URL.
The program adds client's source address to a list managed by xt_recent iptables extension. To be able to do so, it must be set up as a setuid root CGI application. Surely, its URL should require authentication, and the transport should be HTTPS.

## udp-tun
### Simple host-to-host VPN for IPv4 or IPv6 over UDP with support for dynamic endpoints.
The program creates an ad-hoc VPN by connecting client's and server's tunnel interfaces by UDP packets. Addresses and routing have to be set up manually. The server keeps track of the client address by using the address of the last packed received. The program provides no authentication, integrity checks, or encryption, and cannot withstand any deliberate attack. So it should be used in controlled environments only.

## Usage
For any questions on command line options, constants, paths etc. refer to the source.