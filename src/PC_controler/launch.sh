#!/bin/sh

export LD_LIBRARY_PATH="../common/:$LD_LIBRARY_PATH"

echo "Launch pibot controler"
../pibot_controler/pibot_controler --ip $1 &

echo "Launch pibot video stream"
#nc -l -p 5000 | mplayer -fps 60 -cache 2048 -flip -
nc $1 5000 | mplayer -fps 60 -cache 2048 -flip -
