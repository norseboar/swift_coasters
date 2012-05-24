#! /bin/bash

SERVICEURL=$1
NWORKERS=$2
LOGLEVEL=$3

# WORKER_LOGGING_LEVEL=$LOGLEVEL ./worker.pl http://128.135.125.17:$PORT swork01 ./workerlogs

RUN=`date +%m%d%H`
LOGDIR=/home/jchu542/logs/$RUN/
WORKERDIR=/home/jchu542/worker_dir/worker.pl
mkdir $LOGDIR
cd $LOGDIR
touch $LOGDIR/worker.log

for worker in $(seq -w 0 $(($NWORKERS-1))); do
 WORKER_LOGGING_LEVEL=$LOGLEVEL
$WORKERDIR $SERVICEURL swork${worker} $LOGDIR
done
ls -lt $LOGDIR/
tail $LOGDIR/worker*
