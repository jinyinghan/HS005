#! /bin/sh  

echo "=============checknetwork.sh  ================"
count=0
while true ; do  
let count++
NUM=`date +%H`  
if [ "${NUM}" -eq 4 ];then  

#echo "ssid error count = ${count}"
if [ ${count} -gt 1440 ];then
echo "start reboot"
killall tstreamer
sleep 10
sync
reboot
fi
fi  

sleep 60
done  

exit 0
