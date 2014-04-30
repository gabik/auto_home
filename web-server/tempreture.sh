#!/bin/bash
return_code=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi C $1" | tail -1 | tr -d $'\r' `
echo "Tempreture: $return_code C"
exit 0
else
echo "Error... :("
exit -1
fi
