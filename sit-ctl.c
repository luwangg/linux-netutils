/*
	SIT tunnel endpoint changer
	Copyright (c) 2018 Alexander Mukhin
	MIT License
*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/in6.h>
#include <linux/if_tunnel.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define PORT 65001
#define MAXPKLEN 1500
#define MAXNAMESIZ 255

void chgdst (char *ifname, struct in_addr dst)
{
	int s;
	struct ifreq ifr;
	struct ip_tunnel_parm p;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = (void *)&p;
	ioctl(s, SIOCGETTUNNEL, (void *)&ifr);
	p.iph.daddr = dst.s_addr;
	ioctl(s, SIOCCHGTUNNEL, (void *)&ifr);
	close(s);
}


int
main (int argc, char *argv[])
{
	struct sockaddr_in my_addr, peer_addr;
	char pk[MAXPKLEN];
	int len;
	socklen_t peer_addr_len;
	struct in_addr dst;
	int sockfd;
	char sfname[MAXNAMESIZ+1];
	int sfd;

	/* UDP socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&my_addr, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (argc == 4) {
		/* Client */
		/* Usage: sit-ctl ifname local_addr remote_addr */
		/* Bind to a random port */
		bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
		/* Set peer address and port */
		bzero(&peer_addr, sizeof(peer_addr));
		inet_aton(argv[3], &peer_addr.sin_addr);
		peer_addr.sin_port = htons(PORT);
		peer_addr_len = sizeof(peer_addr);
		/* To be done: Prepare signed message */
		/* For now, just send local IP in plaintext */
		inet_aton(argv[2], &dst);
		len = sizeof(dst);
		memcpy(pk, &dst, len);
		/* Send packet */
		sendto(sockfd, pk, len, 0, (struct sockaddr *)&peer_addr, peer_addr_len);
	} else if (argc == 3) {
		/* Server */
		/* Usage: sit-ctl state_dir ifname */
		/* Compose state file name */
		if (strlen(argv[1]) + 1 + strlen(argv[2]) > MAXNAMESIZ) {
			fprintf(stderr, "Too long file name\n");
			exit(1);
		}
		strcpy(sfname, argv[1]); /* Directory */
		strcat(sfname, "/");
		strcat(sfname, argv[2]); /* Interface */
		/* Read state file */
		sfd = open(sfname, O_RDONLY);
		len = read(sfd, &dst, sizeof(dst));
		close(sfd);
		/* Init tunnel destination */
		if (len == sizeof(dst)) {
			fprintf(stderr, "INIT %s %s\n", argv[2], inet_ntoa(dst));
			/* Modify tunnel destination */
			chgdst(argv[2], dst);
		}
		/* Bind to the defined port */
		my_addr.sin_port = htons(PORT);
		bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
		/* Peer address and port are not known yet (zeroized) */
		bzero(&peer_addr, sizeof(peer_addr));
		/* Receive messages */
		while (1) {
			peer_addr_len = sizeof(peer_addr);
			len = recvfrom(sockfd, pk, MAXPKLEN, 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
			/* Verify packet contents and signature (to be done) */
			if (len == sizeof(dst)) {
				memcpy(&dst, pk, len);
				if (dst.s_addr == peer_addr.sin_addr.s_addr) {
					fprintf(stderr, "CHG %s %s\n", argv[2], inet_ntoa(dst));
					/* Modify tunnel destination */
					chgdst(argv[2], dst);
					/* Save state file */
					sfd = open(sfname, O_RDWR | O_CREAT, 0644);
					write(sfd, &dst, sizeof(dst));
					close(sfd);
				} else
					fprintf(stderr, "IGN INVALID FROM %s\n", inet_ntoa(peer_addr.sin_addr));

			} else
				fprintf(stderr, "IGN INVALID FROM %s\n", inet_ntoa(peer_addr.sin_addr));
		}
	}
}
