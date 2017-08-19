#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

#define PORT 65000
#define MAXPKLEN 1500

int
main (int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in my_addr, peer_addr;
	char pk[MAXPKLEN];
	int pklen;
	socklen_t peer_addr_len;
	int tunfd;
	struct ifreq ifr;
	struct pollfd pfds[2];

	/* UDP socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (argc == 3) {
		/* Client */
		/* Bind to a random port */
		bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
		/* Set peer address and port */
		bzero(&peer_addr, sizeof(peer_addr));
		inet_aton(argv[2], &peer_addr.sin_addr);
		peer_addr.sin_port = htons(PORT);
	} else {
		/* Server */
		/* Bind to the defined port */
		my_addr.sin_port = htons(PORT);
		bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
		/* Peer address and port are not known yet (zeroized) */
		bzero(&peer_addr, sizeof(peer_addr));
	}

	/* Tunnel socket */
	tunfd = open("/dev/net/tun", O_RDWR);
	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	ioctl(tunfd, TUNSETIFF, (void *)&ifr);

	/* Prepare poll stuff */
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN;
	pfds[1].fd = tunfd;
	pfds[1].events = POLLIN;


	/* Drop privileges */
	setuid(65534);

	while (1) {
		poll(pfds, 2, -1);

		/* Copy network input to the tunnel */
		if (pfds[0].revents & POLLIN) {
			/* Net RX */
			peer_addr_len = sizeof(peer_addr);
			pklen = recvfrom(sockfd, pk, MAXPKLEN, 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
			/* Tun TX */
			write(tunfd, pk, pklen);
		}
		/* Copy tunnel input to the network */
		if (pfds[1].revents & POLLIN) {
			/* Tun RX */
			pklen = read(tunfd, pk, MAXPKLEN);
			/* Net TX (if we've already got a peer) */
			if (peer_addr.sin_addr.s_addr) {
				peer_addr_len = sizeof(peer_addr);
				sendto(sockfd, pk, pklen, 0, (struct sockaddr *)&peer_addr, peer_addr_len);
			}
		}
	}
}
