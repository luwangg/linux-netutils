/*
	Non-blocking resolver
	Copyright (c) 2018 Alexander Mukhin
	MIT License
*/

#define NAMESERV "8.8.8.8"
#define DNAMSZ 256	// maximum domain name length
#define QSIZ 20		// bunch size
#define DLY 50000	// delay (in usec) between sending requests

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void
prtresp (unsigned char *buf, int bufsz) {
	ns_msg resp;
	ns_rr rr;
	int rc, i;
	char str[DNAMSZ];

	ns_initparse(buf, bufsz, &resp);
	// Question
	ns_parserr(&resp, ns_s_qd, 0, &rr);
	printf("Q=%s ", ns_rr_name(rr));
	// Answers
	rc = ns_msg_count(resp, ns_s_an);
	for (i=0; i<rc; ++i) {
		ns_parserr(&resp, ns_s_an, i, &rr);
		if (ns_rr_type(rr) == 1) {
			// A
			inet_ntop(AF_INET, (in_addr_t*)ns_rr_rdata(rr), str, DNAMSZ);
			printf("A=%s ", str);
		} else if (ns_rr_type(rr) == 28) {
			// AAAA
			inet_ntop(AF_INET6, (in_addr_t*)ns_rr_rdata(rr), str, DNAMSZ);
			printf("AAAA=%s ", str);
		}
	}
	printf("\n");
}

int
main (void) {
	int s;
	struct sockaddr_in sa;
	unsigned char buf[PACKETSZ];
	char dnam[DNAMSZ];
	int q, nq;
	int rsiz;

	// Prepare socket
	s = socket(AF_INET, SOCK_DGRAM, 0);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(53);
	inet_pton(AF_INET, NAMESERV, &sa.sin_addr);
	connect(s, (struct sockaddr *)&sa, sizeof(sa));

	// Read stdin
	while (!feof(stdin)) {
		// Fill the queue and send queries
		q = 0;
		while (q<QSIZ && fgets(dnam, DNAMSZ, stdin)) {
			*strchr(dnam,'\n') = '\0';	// Drop \n
			res_mkquery(QUERY, dnam, C_IN, T_A, NULL, 0, NULL, buf, sizeof(buf));
			write(s, buf, sizeof(buf));
			++q;
#if 0
// Uncomment to send AAAA query as well
			res_mkquery(QUERY, dnam, C_IN, T_AAAA, NULL, 0, NULL, buf, sizeof(buf));
			write(s, buf, sizeof(buf));
			++q;
#endif
			usleep(DLY);
		}
		// Read responses
		nq = q;
		q = 0;
		while (q<nq && (rsiz=read(s, buf, sizeof(buf)))!=-1) {
			printf("%d/%d ", q+1, nq);
			prtresp(buf, rsiz);
			++q;
		}
	}

	// Close socket
	close(s);
}
