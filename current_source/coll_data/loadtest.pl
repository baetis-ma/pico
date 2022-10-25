#!/usr/bin/perl
#this needs some expaining 
#   the standard input piped from /dev/ttyxxx port
#   transmits a burst of data terminated by line beginning with 'e'
#   the first buch of data put in file rload.txt
#   each subsequent burst put in discharge.txt where the
#      previuos discharge.txt is over written
#   ----------------------------------
#   So, in the beginning rload.txt is opened with handler FILE and ready = 0
#   First Group of data will be written to that rload.txt when first 'e'
#       terminates first group of data the handler FILE is closed and ready = 1
#   Whewn the data resumes and ready == 1 and counter > 10 the handler FILE
#       is reopened pointing to discharge.txt
#   discharge.txt will continue to be written over with newest data
use strict;
use warnings;
my $numberargs = $#ARGV + 1;
if ($numberargs != 1) {
    printf("pass battery name arg to ./loadtest.pl nameofbattery\n");
    exit;
}
my $batteryname = $ARGV[0];
my $filename = "data/$batteryname.rload";
my $filename1 = "data/$batteryname.discharge";

open(FILE_DEF, '>', "rload.txt") or die "cant open file";
open(FILE, '>', $filename) or die "cant open file";

my $ready = 0;
my $counter = 0;
while (1) {
    my $line = <STDIN>; 
    print $line;
    if ($line) {
        ++$counter;
        if ($ready == 1 and $counter > 10) {
            open(FILE, '>', $filename1) or die "cant open file";
            open(FILE_DEF, '>', "discharge.txt") or die "cant open file";
        $ready = 0;
        }
        print FILE $line;
        print FILE_DEF $line;
	my $first = substr $line, 0, 1;
	if($first eq 'e') { 
            close(FILE) or print "couldnt close file";
            close(FILE_DEF) or print "couldnt close file";
	    $ready = 1;
        }
    }
}
