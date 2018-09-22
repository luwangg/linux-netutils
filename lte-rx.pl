#!/usr/bin/perl

# LTE statistics parser
# Copyright (c) 2018 Alexander Mukhin
# MIT License

$TTY="/dev/ttyUSB.LTE-MON";
$ORIG_STTY=`stty -F $TTY -g`;
`stty -F $TTY -icrnl -ixon -echo`;
open FH, "+>", "$TTY";
print FH "AT^HCSQ?\n";
while (<FH>) {
	print ".\n";
	if (/LTE/) {
		($a,$b,$c,$d) = (split(/,/))[1..4];
		print "$a . $b . $c . $d\n";
		printf "RSSI = %4d dBm\n", $a-121;
		printf "RSRP = %4d dBm\n", $b-141;
		printf "SINR = %4d dB\n", ($c-1)/5-20;
		printf "RSRQ = %4d dB\n", $d/2-20;
	}
	last if (/OK/);
}
close FH;
`stty -F $TTY $ORIG_STTY`;
