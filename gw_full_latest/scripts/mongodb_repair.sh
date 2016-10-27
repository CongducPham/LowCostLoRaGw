#!/bin/bash

LOCK_FILE=/var/lib/mongodb/mongod.lock
PID_FILE=/var/run/mongodb.pid

mongo_running=0

#if mongodb.lock and mongodb.pid are different, then need to repair
pid=`cat $PID_FILE`
lock=`cat $LOCK_FILE`
if [ $lock == $pid ]; then
  mongo_running=1
fi

if [ $mongo_running == 1 ]; then
  echo "Mongo already running pid:$pid"
  exit 0
fi

echo "Mongo is not running. Starting to repair."

if [ -f $LOCK_FILE ]; then
  echo -n "Removing lock_file:$LOCK_FILE..."
  sudo rm $LOCK_FILE
  echo "done."
fi

echo "Repairing mongodb..."
mongod --repair
echo "done, starting mongodb..."
sudo service mongod start
