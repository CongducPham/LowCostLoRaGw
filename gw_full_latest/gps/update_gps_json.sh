#!/bin/bash

if [ $# == 1 ]
then
	cur_date=$1
else
	cur_date=`date +%FT%T`
fi

echo "Replacing time with current time $cur_date"

sed -i 's/"time".*,/"time":"'"$cur_date"'",/g' gps.json

echo "time diff is: "
date -ud@$(($(date -ud"`date +%FT%T`" +%s)-$(date -ud"$cur_date" +%s))) +%T

