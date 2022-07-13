#! /bin/bash

#-----
#   this script is used to judge whether target.c will 
#   introduce an additional load on the given compiler
#   
#   args[1] is the compiler we use
#
#-----

> full.c
# cat /home/csmith-2.3.0/test/define.c >> full.c
cat target.c >> full.c
cat /home/csmith-2.3.0/test/main.c >> full.c

compiler=gcc-12.1
if [ $# -ge 1 ]; then
    compiler=$1
fi
$compiler full.c -g -O1 -o full >/dev/null 2>&1
compile_stat=$?
if [ $compile_stat -ne 0 ]; then
    echo "fail due to compilation error: " $compile_stat
    exit 1;
fi
timeout -s SIGTERM 5s /home/csmith-2.3.0/../pin-3.21-98484-ge7cd811fd-gcc-linux/pin -t /home/csmith-2.3.0/checkRead/obj-intel64/checkRead.so -- ./full func 1>/dev/null 1>&2
read line < ./result.out
if [ $line == 1 ] ; then
    exit 0
fi
exit 1