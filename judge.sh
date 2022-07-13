#! /bin/bash

gcc-12.1 /home/csmith-2.3.0/runtime/output2.c -g -w -O1 -o /home/csmith-2.3.0/runtime/output2
timeout -s SIGTERM 5s /home/csmith-2.3.0/../pin-3.21-98484-ge7cd811fd-gcc-linux/pin -t /home/csmith-2.3.0/checkRead/obj-intel64/checkRead.so -- /home/csmith-2.3.0/runtime/output2 func
read line < /home/csmith-2.3.0/result.out
if [ $line == 1 ] ; then
    exit 0
fi
exit 1