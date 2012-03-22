#!/usr/bin/perl

use Algorithm::Permute;

my @array = 'a'..'d';

Algorithm::Permute::permute {
print "next permutation: (@array)\n";
} @array;

