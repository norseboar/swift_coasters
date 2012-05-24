#!/bin/bash

TRACE=$1

if [ $2==NULL ] ; then
SCRIPT="/home/jchu542/swift-0.93/cog/modules/swift/examples/tutorial/hello.swift"
else 
SCRIPT=$2
fi

RUN=`date +%m%d%H`
LOGDIR=/home/jchu542/logs/$RUN/
mkdir $LOGDIR
strace -f -e trace=$TRACE -o ${LOGDIR}strace-swift-$RUN.log swift -sites.file sites.xml -tc.file tc.data $SCRIPT -config cf -logfile ${LOGDIR}hello-$RUN.log
