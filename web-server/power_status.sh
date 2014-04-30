#!/bin/bash
return_code=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi s $1" | tail -1 | tr -d $'\r' `
if [ $return_code -ge 0 ] ; then
state="Off"
if [ $return_code -eq 1 ] ; then 
state="On"
fi
echo "Power is $state"
exit 0
else
echo "Error... :("
exit -1
fi
