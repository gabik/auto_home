#!/bin/bash
return_code=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi f $1" | tail -1 | tr -d $'\r' `
if [ $return_code -eq 0 ] ; then
echo "Powered Off :)"
exit 0
else
echo "Error... :("
exit -1
fi
