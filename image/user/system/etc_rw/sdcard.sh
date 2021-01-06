#!/bin/sh

SDROOT=/tmp/$MDEV
sd_log_dir=$SDROOT/log

do_copy_log()
{
	sync
	rm $sd_log_dir/*
	cp /var/log/* $sd_log_dir/
	cp /etc/SNIP39/version.conf $sd_log_dir/
	cp /usr/share/fw_version  $sd_log_dir/
    ps -T > $sd_log_dir/cmd_ps.txt
	echo 1 > /proc/sys/vm/drop_caches
	#free > $sd_log_dir/cmd_free.txt
	#for some security reason, dont get all process log
	top -n 1 | grep -E "COMMAND|monitor|tstreamer" | grep -v grep > $sd_log_dir/cmd_top.txt
	sync
	
#	opus_play -i /usr/bin/notify/dong.opus
}

do_insert()
{
	DO_AUTORUN=0
	DO_LOG=0
	DO_NORMAL=0

	if [ -s /system_rw/autorun_enable ]; then
		if [ -s $SDROOT/360_autorun.sh ]; then
			DO_AUTORUN=1
		fi
	fi
	
	if [ $DO_AUTORUN = 0 ]; then
		if [ -f "/tmp/sd_log_enable" ]; then
			if [ -d "$sd_log_dir" ]; then
				DO_LOG=1
			else
				DO_NORMAL=1
			fi
		fi
	fi
	
	if [ $DO_NORMAL != 0 ]; then
    if [ ! -f "/tmp/play_sd_pause" ];then
        sample_play 16000 /usr/notify/sd_mount.pcm 
	fi
    fi

	# check fs after normal sound
	# has no effect to exFat cards
	# fsck.fat -a -v /dev/$MDEV > /tmp/fsck.log
	# rm $SDROOT/FSCK*.REC

	if [ $DO_LOG != 0 ]; then
		LOG "copy_log"
		do_copy_log
	fi
	
	if [ $DO_AUTORUN != 0 ]; then
		LOG "autorun"
		$SDROOT/360_autorun.sh
	fi
	
	#echo $SDROOT > /tmp/sd_name
	cat /proc/uptime > /tmp/sd_time	
	rm -f /tmp/sd_add_pid
	
	LOG "ready"
}

LOG()
{
	log_str="[SD] PID:$$ $MDEV $1"

	logger $log_str
	# echo $log_str > /dev/ttyS000
}

if [ "$ACTION" = "add" ]; then

	LOG "adding"
	
	if [ -b /dev/$MDEV"p1" ]; then
		LOG "ignore"
		exit 0
	fi
	
	echo $$ > /tmp/sd_add_pid
	sleep 1
	mkdir $SDROOT	
	LOG "mounting"
	mount  -o noatime,nodiratime /dev/$MDEV $SDROOT
	
	if [ $? != 0 ]; then
		LOG "mount failed"
		rmdir $SDROOT
		rm -f /tmp/sd_add_pid
		exit 0
	fi
	
	LOG "mounted"
	echo $SDROOT > /tmp/sd_ok
	do_insert

elif [ "$ACTION" = "remove" ]; then

	LOG "remove"

	rm -f /tmp/sd_name
	rm -f /tmp/sd_ok
	rm -f /tmp/sd_time
	
	#kill running add action
	pid_insert=`cat /tmp/sd_add_pid`
	rm -f /tmp/sd_add_pid
	if [ "$pid_insert" != "" ] ; then
		kill $pid_insert
		LOG "kill old $pid_insert"
	fi
	
	umount $SDROOT -l
	rm -r $SDROOT
fi
