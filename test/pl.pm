#!/usr/bin/perl
=pod
    args[0] is the orginal case file to be reduced, and reduction won't modify it
    args[1] is the compiler we use
=cut

$len=scalar @ARGV;
$src_file="output2.c";
if($len>=1){
    $src_file=$ARGV[0];
}
$compiler="gcc-12.1";
if($len>=2){
    $compiler=$ARGV[1];
}
$target_file="target.c";
$define_file="define.c";
$main_file="main.c";

$target_pat="FUNCTIONS";
$target_start=-1;
$main_pat="int side=0";
$main_start=-1;

open $file, "<", $src_file or die "open file failed:$!";
$num=0;
@defs=();
@targets=();
@main=();
$state=0;
while(my $line=<$file>){
    # print $line;
    if((my $pos=index($line, $target_pat))!=-1){
        $target_start=$num;
        $state=1;
    }
    if((my $pos=index($line, $main_pat))!=-1){
        $main_start=$num;
        $state=2;
    }
    if($state==0){
        push @defs, $line;
    }elsif($state==1){
        push @targets, $line;
    }else{
        push @main, $line;
    }
    $num+=1;
}
close(file);

# print(@main);

open $file, ">", $target_file;
print $file @defs;
close file;

open $file, ">>", $target_file;
print $file @targets;
close file;

open $file, ">", $main_file;
print $file @main;
close file;


if($compiler =~ m/gcc/){
    system("creduce ./judge_gcc.kb target.c");
}elsif($compiler =~ m/clang/){
    system("creduce ./judge_clang.kb target.c");
}elsif($compiler =~ m/icc/){
    system("creduce ./judge_icc.kb target.c");
}

# system("creduce `./judge.sh @{[$compiler]}` target.c");
