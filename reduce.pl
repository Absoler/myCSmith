#!/usr/bin/perl
=pod
    this script reduce cases in ./problem according to the correspoding compiler
=cut

use File::Find;
use Cwd;
# use Sort::Naturally;
exit(1);
$gcc_name=
$clang_name=

my $base;
open my $file, "<", "base.out";
while(<$file>){
    if($.==2){
        $base=$_;
    }
}

print "base", $base, "\n";

my $cwd = getcwd;
my @files=();

sub wanted{
    if($_ =~ m/\.c/){
        $_ =~ m/\d+/;
        
        if($&>=$base){
            push @files, $_;
        }
    }
}

find(\&wanted, "$cwd/problem");


@files = sort {
    $a =~ m/\d+/;
    my $anum = $&;
    $b =~ m/\d+/;
    my $bnum = $&;
    $anum<=>$bnum;
} @files;

for(my $i=0; $i<=$#files; $i++) {
    my $file=$files[$i];
    $compiler=$gcc_name;
    if($file =~ m/clang/){
        $compiler=$clang_name;
    }elsif($file =~ m/icc/){
        $compiler="icc";
    }
    
    system("$cwd/reduce.kb $file $compiler");
    
    $file =~ m/\d+/;
    my $cur_id = $&+1;
    system("sed \"2c ${cur_id}\" -i base.out")
}
