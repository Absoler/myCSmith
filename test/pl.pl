#!/usr/bin/perl
=pod
    args[0] is the orginal case file to be reduced, and reduction won't modify it
    args[1] is the compiler we use
=cut
$root_dir=
$gcc_name=

use Cwd;
$len=scalar @ARGV;
$src_file="output2.c";
if($len>=1){
    $src_file=$ARGV[0];
}
$compiler=$gcc_name;
if($len>=2){
    $compiler=$ARGV[1];
}
$target_file="$root_dir/test/target.c";
$define_file="$root_dir/test/define.c";
$main_file="$root_dir/test/main.c";

$target_pat="FUNCTIONS";
$target_start=-1;
$main_pat="side=0";
$main_start=-1;

open $file, "<", $src_file or die "open file failed:$!";
$num=0;
@defs=();
@targets=();
@main=();
$state=0;

$core = `nproc`/2;
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

open $file, ">", $define_file;
print $file @defs;
close file;

open $file, ">", $target_file;
print $file @targets;
close file;

open $file, ">", $main_file;
print $file @main;
close file;

if (Cwd::abs_path(getcwd()) ne "$root_dir/test") {
    system("cp $root_dir/test/target.c ./target.c");
}
my $stat;
if($compiler =~ m/gcc/){
    $stat = system("creduce --tidy --n $core $root_dir/test/judge_gcc.kb target.c");
}elsif($compiler =~ m/clang/){
    $stat = system("creduce --tidy --n $core $root_dir/test/judge_clang.kb target.c");
}elsif($compiler =~ m/icc/){
    $stat = system("creduce --tidy --n $core $root_dir/test/judge_icc.kb target.c");
}
if (Cwd::abs_path(getcwd()) ne "$root_dir/test") {
    system("mv target.c $root_dir/test/")
}
exit $stat;