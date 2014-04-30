#!/bin/bash
return_code=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi t $1" | tail -1 | tr -d $'\r' `
if [ $return_code -ge 0 ] ; then
hours=`echo "$return_code / 60" | bc`
minutes=`echo "$return_code - ($hours * 60)" | bc`
echo "Total time: $hours:$minutes"
exit 0
else
echo "Error... :("
exit -1
fi
