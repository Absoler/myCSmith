#!/usr/bin/perl
=pod
    this script reduce cases in ./problem according to the correspoding compiler
=cut

use File::Find;
use Cwd;
# use Sort::Naturally;

my $base;
open my $file, "<", "base.out";
while(<$file>){
    if($.==2){
        $base=$_;
    }
}

print "base", $base, "\n";

my @files=();

sub wanted{
    if($_ =~ m/\.c/){
        $_ =~ m/\d+/;
        
        if($&>=$base){     # 3942 3971  3997
            push @files, $_;
            # $compiler="gcc-12.1";
            # if($_ =~ m/clang/){
            #     $compiler="clang";
            # }elsif($_ =~ m/icc/){
            #     $compiler="icc";
            # }
            # system("/home/csmith-2.3.0/reduce.kb $_ $compiler");
        }
    }
}

find(\&wanted, "./problem");




@files = sort {
    $a =~ m/\d+/;
    my $anum = $&;
    $b =~ m/\d+/;
    my $bnum = $&;
    $anum<=>$bnum;
} @files;

for(my $i=0; $i<=$#files; $i++) {
    my $file=$files[$i];
    $compiler="gcc-12.1";
    if($file =~ m/clang/){
        $compiler="clang";
    }elsif($file =~ m/icc/){
        $compiler="icc";
    }
    
    system("/home/csmith-2.3.0/reduce.kb $file $compiler");
    
    $file =~ m/\d+/;
    my $cur_id = $&+1;
    system("sed \"2c ${cur_id}\" -i base.out")
}
