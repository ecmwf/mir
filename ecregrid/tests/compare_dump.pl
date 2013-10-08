#!/usr/local/bin/perl56
#--------------------------------------------------------
# S. Curic 2011
#--------------------------------------------------------
use strict;


die("Usage: compare_dump.pl <dump1> \n") if $#ARGV < 0;

my $dump1=$ARGV[0];

open(F,"<$dump1") or die "cannot open the  $dump1";

my $elem;
my $counter = 0;
my %content; 
my $first = 0;

while(<F>) {
	if ( /^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+\|\s+(\S+)\s+(\S+)\s+(\S+)/ ) {
         my $rb = $1; chomp($rb);
         my $lat = $2; chomp($lat);
         my $lon = $3; chomp($lon);
		 my $value = $4; chomp($value);
         my $lat2 = $5; chomp($lat2);
         my $lon2 = $6; chomp($lon2);
		 my $value2 = $7; $value2 =~ s/\s+$//;
		 $content{$lat} = $lon;
		 $counter++;
	 	if($lat ne $lat2 or $lon ne $lon2){
	   	  print "lat/lon differ $counter \n lat: $lat lon: $lon value: $value \n lat: $lat2 lon: $lon2 value: $value2 \n";
		 print "----------------------------------- \n";
		}
	}
}
close(F);
