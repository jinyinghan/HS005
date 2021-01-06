#!/bin/sh

#fsck.fat -awy /dev/$MDEV
mount -o noatime,nodiratime /dev/$MDEV /mnt/sdcard
rm /mnt/sdcard/*.REC
if [ -x /mnt/sdcard/360_autorun.sh ];then
    echo "find 360_autorun.sh" > /dev/console
    cd /mnt/sdcard/; sh 360_autorun.sh
    echo "done 360_autorun.sh" > /dev/console
fi
