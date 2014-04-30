#!/bin/bash
return_code1=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi s $1" | tail -1 | tr -d $'\r' `
if [ $return_code1 -ge 0 ] ; then
 state="Off"

 if [ $return_code1 -eq 1 ] ; then 
  state="On"
 fi
else
 echo "Error1... :("
 exit -1
fi

return_code2=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi t $1" | tail -1 | tr -d $'\r' `
if [ $return_code2 -ge 0 ] ; then
 hours=`echo "$return_code2 / 60" | bc`
 minutes=`echo "$return_code2 - ($hours * 60)" | bc`
 total_time="$hours:$minutes"
else
 echo "Error2... :("
 exit -2
fi
 
return_code3=`ssh root@gabi.kazav.net "cd /root/home/rasPi/librf24-rpi/librf24/examples ; ./gabi C $1" | tail -1 | tr -d $'\r' `
if [ $return_code3 -ge 0 ] ; then
 tempr="$return_code3 C"
else
 echo "Error3... :("
 exit -3
fi

echo "Power is $state"
echo "Up for $total_time"
echo "Tempreture is $tempr"
exit 0
