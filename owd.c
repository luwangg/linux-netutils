/*
	One-way delay measurement
	Copyright (c) 2018 Alexander Mukhin
	MIT License
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define PORT "65002"
#define MAXPKLEN 1500

int
main (int argc, char *argv[])
{
	int s, err, opt;
	struct addrinfo hints, *res, *r;
	char pk[MAXPKLEN];
	int pklen;
	struct timespec tx, rx, bx;
	float delta;
	struct sockaddr_storage pa;
	socklen_t palen = sizeof(pa);

	if (argc == 2) {
		// Client
		// Prepare socket
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		err = getaddrinfo(argv[1], PORT, &hints, &res);
		if (err) {
			fprintf(stderr, "%s\n", gai_strerror(err));
			return 1;
		}
		for (r=res; r; r=r->ai_next) {
			s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
			if (s > 0)
				break;
		}
		if (r == NULL)
			return 2;
		// Get TX time
		clock_gettime(CLOCK_REALTIME, &tx);
		// Prepare packet body
		pklen = sizeof(tx);
		memcpy(pk, &tx, pklen);
		// Send request packet
		sendto(s, pk, pklen, 0, r->ai_addr, r->ai_addrlen);
		// Wait for the reply
		pklen = recvfrom(s, pk, MAXPKLEN, 0, NULL, NULL);
		// Get RX time
		clock_gettime(CLOCK_REALTIME, &rx);
		// Fetch bounce timestamp from the packet body
		memcpy(&bx, pk, sizeof(bx));
		// Print those timestamps and their deltas
		printf("TX %9ld %9ld\n", tx.tv_sec, tx.tv_nsec);
		printf("BX %9ld %9ld, ", bx.tv_sec, bx.tv_nsec);
		delta = (bx.tv_sec-tx.tv_sec)*1000 + (bx.tv_nsec-tx.tv_nsec)/1000000.0;
		printf("delta = %.3g ms\n", delta);
		printf("RX %9ld %9ld, ", rx.tv_sec, rx.tv_nsec);
		delta = (rx.tv_sec-bx.tv_sec)*1000 + (rx.tv_nsec-bx.tv_nsec)/1000000.0;
		printf("delta = %.3g ms\n", delta);
	} else {
		// Server
		// Prepare socket
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;
		err = getaddrinfo(NULL, PORT, &hints, &res);
		if (err) {
			fprintf(stderr, "%s\n", gai_strerror(err));
			return 1;
		}
		for (r=res; r; r=r->ai_next) {
			s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
			if (s > 0)
				break;
		}
		if (r == NULL)
			return 2;
		opt = 0;
		setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
		err = bind(s, r->ai_addr, r->ai_addrlen);
		if (err == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			return 3;
		}
		// Wait for the message from a client
		while (1) {
			pklen = recvfrom(s, pk, MAXPKLEN, 0, (struct sockaddr*)&pa, &palen);
			// Set bounce time to the time when the packet was received
			clock_gettime(CLOCK_REALTIME, &bx);
			// Put it in the body of the reply packet
			pklen = sizeof(bx);
			memcpy(pk, &bx, pklen);
			// Send reply
			err = sendto(s, pk, pklen, 0, (struct sockaddr*)&pa, palen);
			// Print the timestamp
			printf("BX %9ld %9ld\n", bx.tv_sec, bx.tv_nsec);
		}
	}

	// Cleanup
	close(s);
	freeaddrinfo(res);
	return 0;
}
