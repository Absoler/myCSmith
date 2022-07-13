#!/usr/bin/perl
=pod
    this script reduce cases in ./problem according to the correspoding compiler
=cut

use File::Find;
use Cwd;

my $base;
open my $file, "<", "base.out";
while(<$file>){
    if($.==2){
        $base=$_;
    }
}
my $dir = getcwd;
print $dir;
print "base", $base, "\n";
sub wanted{
    if($_ =~ m/\.c/){
        $_ =~ m/\d+/;
        
        if($&==$base){
            
            $compiler="gcc-12.1";
            if($_ =~ m/clang/){
                $compiler="clang";
            }elsif($_ =~ m/icc/){
                $compiler="icc";
            }
            system("/home/csmith-2.3.0/reduce.kb $_ $compiler");
            # system("./pl.pm output2.c $compiler");
            # system("cp /home/csmith-2.3.0/test/full.c ../problem/$_ && cd ..");
        }
    }
}
find(\&wanted, "./problem");