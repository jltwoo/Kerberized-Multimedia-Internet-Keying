#!/usr/bin/perl
# sudo apt-get install libmath-combinatorics-perl
# sudo apt-get install libalgorithm-permute-perl
use Math::Combinatorics;
use Algorithm::Permute;
#@numbers = (1 .. 20);
$array_size = 4;
$select_size = 3;

$permutations = factorial( $select_size ); 
#@shuffle = @array [ n2perm( 1+int(rand $permutations), $numbers ) ];
my $index = rand($permutations);

my @array;

for( $i=1; $i <= $array_size; $i++ ) 
{
   push( @array, $i );
}

#Algorithm::Permute::permute {print "@array\n" } @array;

my $p = new Algorithm::Permute([1..$array_size], $select_size);


while((@res = $p->next) && ($index-- > 0 ) ) {
   print join(", ", @res), "\n";
}
