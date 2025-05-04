#!/bin/bash

LOCK_FILE=/var/lib/mongodb/mongod.lock
#PID_FILE=/var/run/mongodb.pid

mongo_running=0

lock="0"
pid="1"

#if mongodb.lock and mongodb.pid are different, then need to repair
pid=`pgrep mongod`
if [ -f $LOCK_FILE ]; then
  lock=`cat $LOCK_FILE`
fi
echo "pid : $pid";
echo "lock : $lock";

if [ "$lock" == "$pid" ]; then
  mongo_running=1
fi

if [ "$lock" == "" ] || [ "$pid" == "" ]; then
  mongo_running=0
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

echo "Stopping mongodb if needed..."
sudo service mongodb stop
echo "Repairing mongodb..."
mongod --repair
echo "done, starting mongodb..."
sudo service mongodb start
