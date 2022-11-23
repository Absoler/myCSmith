#!/usr/bin/perl
=pod

=cut
use File::Find;
use File::Path;
use File::Copy;
my @files=();
sub wanted{
    if($_ =~ m/\.c/){
        push @files, $_;
    }
}
find(\&wanted, "./patterns");

$target_pat="func_1";
$main_pat="side=0";

for my $src_file (@files){
    open $file, "<", "patterns/$src_file" or die "open file failed:$!";
    $num=0;
    $main_start=0;
    $target_start=0;
    $state=0;
    while(my $line=<$file>){
        # print $line;
        if((my $pos=index($line, $target_pat))!=-1){
            $target_start=$num;
        }
        if((my $pos=index($line, $main_pat))!=-1){
            $main_start=$num;
            last;
        }
        
        $num+=1;
    }
    close(file);
    if($main_start-$target_start>150){
        print $src_file, "\n";
    }
    # $file =~ m/\d+/;
    # $num = $&;
    # if($num > 330 ){
    #     next;
    # }
    # print $file;
    # if(-e "./problem/${num}_gcc-12.1_checkAccess.out"){
    #     $id=1;
    #     copy("./problem/${num}_output.c" , "./problem/${num}_gcc-12.1_output.c");
    # }
    # if(-e "./problem/${num}_clang_checkAccess.out"){
    #     $id=1;
    #     copy("./problem/${num}_gcc-12.1_output.c" , "./problem/${num}_clang_output.c");
    # }
    # if(-e "./problem/${num}_icc_checkAccess.out"){
    #     $id=1;
    #     copy("./problem/${num}_gcc-12.1_output.c" , "./problem/${num}_icc_output.c");
    # }
    # if(-e "./problem/${num}_gcc-12.1_checkAccess.out" || -e "./problem/${num}_checkAccess.out"){
    #     next;
    # }
    # unlink "./problem/${num}_gcc-12.1_output.c";
    # if($id==1){
    #     unlink "./problem/${num}_output.c";
    # }else{
    #     move("./problem/${num}_output.c" , "./problem/${num}_gcc-12.1_output.c");
    # }
}