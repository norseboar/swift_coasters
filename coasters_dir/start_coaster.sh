#!/bin/bash

TRACE=$1
PORT=$2
LPORT=$3

RUN=`date +%m%d%H`
LOGDIR=/home/jchu542/logs/$RUN/
mkdir $LOGDIR
touch $LOGDIR/worker-$RUN.log
touch $LOGDIR/coaster-$RUN.log
strace -f -e trace=$TRACE -o ${LOGDIR}strace-coaster-$RUN.log coaster-service -passive -nosec -p $PORT -localport $LPORT | tee -a ${LOGDIR}/coaster-$RUN.log
