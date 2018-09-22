/*
	SSH door
	Copyright (c) 2018 Alexander Mukhin
	MIT License
*/

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TBL_PATH "/proc/net/xt_recent/SSH.DOOR"

int
chkip (char *s) {
	int k;
	char *p;
	for (k=0; k<4; ++k) { 	/* loop for octets */
		for (p=s; isdigit(*s); ++s);
		if ((*s != '.') && (*s != '\0')) 	/* Garbage input */
			return -1;
		if (p == s)				/* Empty octet */
			return -2;
		if ((*s == '\0') && (k != 3)) 		/* Less than four octets */
			return -3;
		if ((*s == '.') && (k == 3)) 		/* More than four octets */
			return -4;
		if ((s-p > 3) || (atoi(p) > 255)) 	/* Value out of range */
			return -5;
		++s;	/* go to next octet */
	}
	return 0;
}

int
main (void) {
	char *ra;
	int e;
	FILE *f;

	printf("Content-type: text/plain\n\n");

	ra = getenv("REMOTE_ADDR");

	e = chkip(ra);
	if (e < 0) {
		printf("REMOTE_ADDR (%s) is invalid (%d)\n", ra, e);
		exit(EXIT_FAILURE);
	}

	f = fopen(TBL_PATH, "w");
	if (f == NULL) {
		printf("Error opening %s (%s)\n", TBL_PATH, strerror(errno));
		exit(EXIT_FAILURE);
	}

	e = fprintf(f, "+%s", ra);
	if (e < 0) {
		printf("Error writing %s\n", TBL_PATH);
		exit(EXIT_FAILURE);
	}
	printf("+%s\n", ra);

	fclose(f);
	exit(EXIT_SUCCESS);
}
