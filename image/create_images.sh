#!/bin/bash
PRODUCT_NAME=HS005
FW_NAME=FIRMWARE_$PRODUCT_NAME 
echo "@@@@@@@@@${FW_NAME}@@@@@@@@@@@@"
FW_VERSION=020000

UBOOT_SIZE_KB=256
UIMAGE_SIZE_KB=1792
ROOTFS_SIZE_KB=1792
USER_SIZE_KB=4288
UIMAGE2_SIZE_KB=1792
ROOTFS2_SIZE_KB=1792
USER2_SIZE_KB=4288
MTD_SIZE_KB=256
FACTORY_SIZE_KB=128

UBOOT_OFFSET_KB=0
UIMAGE_OFFSET_KB=$[${UBOOT_OFFSET_KB}+${UBOOT_SIZE_KB}]
ROOTFS_OFFSET_KB=$[${UIMAGE_OFFSET_KB}+${UIMAGE_SIZE_KB}]
USER_OFFSET_KB=$[${ROOTFS_OFFSET_KB}+${ROOTFS_SIZE_KB}]
UIMAGE2_OFFSET_KB=$[${USER_OFFSET_KB}+${USER_SIZE_KB}]
ROOTFS2_OFFSET_KB=$[${UIMAGE2_OFFSET_KB}+${UIMAGE2_SIZE_KB}]
USER2_OFFSET_KB=$[${ROOTFS2_OFFSET_KB}+${ROOTFS2_SIZE_KB}]
MTD_OFFSET_KB=$[${USER2_OFFSET_KB}+${USER2_SIZE_KB}]
FACTORY_OFFSET_KB=$[${MTD_OFFSET_KB}+${MTD_SIZE_KB}]

rm binaries -rf
mkdir binaries

echo "Generate rootfs & rootfs2 image..."
cp -r root root_tmp
find root_tmp -name .gitignore | xargs rm
mksquashfs root_tmp binaries/root.squashfs -comp xz -b 65536 ||exit 1
sed -i "s#^mount -t squashfs /dev/mtdblock3 /usr#mount -t squashfs /dev/mtdblock5 /usr#g"  root_tmp/etc/init.d/rcS
mksquashfs root_tmp binaries/root2.squashfs -comp xz -b 65536 ||exit 1
rm root_tmp -rf

echo "Generate user & user2 image..."
mksquashfs user binaries/user.squashfs -comp xz -b 65536 ||exit 1
mksquashfs user binaries/user2.squashfs -comp xz -b 65536||exit 1
 
#check user & rootfs size 
user_size=`du -k binaries/user.squashfs |awk '{print $1}'`
root_size=`du -k binaries/root.squashfs |awk '{print $1}'`
echo use size is $user_size
echo root size is $root_size
if [ $user_size -ge $[$USER_SIZE_KB-64] ];then
	echo "use.squashfs is too big ,please check it"
	exit -1
fi
if [ $root_size -ge $[$ROOTFS_SIZE_KB-64] ];then
	echo "root.squashfs is too big ,please check it"
	exit -1
fi
echo "size check ok"

echo "Generate mtd image..."
#mkfs.jffs2 -o binaries/mtd.jffs2 -r etc -e 0x8000 -s 0x40000 -n -l -X zlib --pad=0x100000
#tr '\000' '\377' < /dev/zero | dd of=binaries/mtd.bin bs=1024 count=1024 > /dev/null
tr '\000' '\377' < /dev/zero | dd of=binaries/mtd.bin bs=1024 count=${MTD_SIZE_KB} > /dev/null

echo "Generate empty factory image..."
tr '\000' '\377' < /dev/zero | dd of=binaries/factory.bin bs=1024 count=${FACTORY_SIZE_KB} > /dev/null
 
echo "Pad images"
tr '\000' '\377' < /dev/zero | dd of=binaries/flash-16m.bin bs=1M count=16
dd if=u-boot-with-spl.bin of=binaries/flash-16m.bin bs=1K seek=${UBOOT_OFFSET_KB} ||exit 1
dd if=uImage of=binaries/flash-16m.bin bs=1K seek=${UIMAGE_OFFSET_KB}||exit 1
dd if=binaries/root.squashfs of=binaries/flash-16m.bin bs=1K seek=${ROOTFS_OFFSET_KB} ||exit 1
dd if=binaries/user.squashfs of=binaries/flash-16m.bin bs=1K seek=${USER_OFFSET_KB}||exit 1
dd if=uImage of=binaries/flash-16m.bin bs=1K seek=${UIMAGE2_OFFSET_KB}||exit 1
dd if=binaries/root2.squashfs of=binaries/flash-16m.bin bs=1K seek=${ROOTFS2_OFFSET_KB}||exit 1
dd if=binaries/user2.squashfs of=binaries/flash-16m.bin bs=1K seek=${USER2_OFFSET_KB}||exit 1
dd if=binaries/mtd.bin of=binaries/flash-16m.bin bs=1K seek=${MTD_OFFSET_KB}||exit 1
dd if=binaries/factory.bin of=binaries/flash-16m.bin bs=1K seek=${FACTORY_OFFSET_KB}||exit 1

echo "write flag:"
#cp binaries/flash-16m.bin binaries/FIRMWARE_DT20_F.Bin
cd tools/wr_flag/
make
./wr_flag ../../binaries/flash-16m.bin ${UIMAGE_OFFSET_KB}        # the end of kernel part
./wr_flag ../../binaries/flash-16m.bin ${ROOTFS_OFFSET_KB}        # the end of kernel part
./wr_flag ../../binaries/flash-16m.bin ${USER_OFFSET_KB}		# the end of user part	
./wr_flag ../../binaries/flash-16m.bin ${UIMAGE2_OFFSET_KB}        # the end of kernel part
./wr_flag ../../binaries/flash-16m.bin ${ROOTFS2_OFFSET_KB} 		# the end of kernel2 part
./wr_flag ../../binaries/flash-16m.bin ${USER_OFFSET_KB} 		# the end of kernel2 part
make clean
cd ../..

#dd if=binaries/flash-16m.bin of=binaries/FIRMWARE_DT20.Bin bs=1024 skip=256 count=7424

#dd if=binaries/flash-16m.bin of=binaries/FIRMWARE_DT20_F.Bin bs=1024 count=${FACTORY_OFFSET_KB}
echo ""
echo "Show partitions: "
printf "    boot: %dk@0x%08x\n" ${UBOOT_SIZE_KB} ${UBOOT_OFFSET_KB}
printf "    rootfs: %dk@0x%08x\n" ${ROOTFS_SIZE_KB} $[${ROOTFS_OFFSET_KB}*1024]
printf "    kernel: %dk@0x%08x\n" ${UIMAGE_SIZE_KB} $[${UIMAGE_OFFSET_KB}*1024]
printf "    user: %dk@0x%08x\n" ${USER_SIZE_KB} $[${USER_OFFSET_KB}*1024]
printf "    kernel2: %dk@0x%08x\n" ${UIMAGE2_SIZE_KB} $[${UIMAGE2_OFFSET_KB}*1024]
printf "    user2: %dk@0x%08x\n" ${USER2_SIZE_KB} $[${USER2_OFFSET_KB}*1024]
printf "    mtd: %dk@0x%08x\n" ${MTD_SIZE_KB} $[${MTD_OFFSET_KB}*1024]
printf "    factory: %dk@0x%08x\n" ${FACTORY_SIZE_KB} $[${FACTORY_OFFSET_KB}*1024]

echo ""
./mkimage -A mips -T firmware -C none -n jz_fw -d binaries/FIRMWARE_DT20.Bin binaries/FIRMWARE_DT20_CRC.Bin
./mkimage -A mips -T firmware -C none -n jz_fw_f -d binaries/FIRMWARE_DT20_F.Bin binaries/FIRMWARE_DT20_F_CRC.Bin
#rm binaries/FIRMWARE_DT20.Bin binaries/FIRMWARE_DT20_F.Bin
mv binaries/FIRMWARE_DT20_CRC.Bin binaries/${FW_NAME}.bin
mv binaries/FIRMWARE_DT20_F_CRC.Bin binaries/${FW_NAME}_F.bin
mv binaries/flash-16m.bin binaries/${FW_NAME}_flash.bin
md5sum binaries/${FW_NAME}.bin
md5sum binaries/${FW_NAME}_F.bin
echo "Done."
##########CUT FIRMWARE########
kernel_bytes=`ls -l uImage |awk '{print $5}'`
kernel_size=$[${kernel_bytes}/1024]
if [ ${kernel_bytes}%0 ];then
	kernel_size=$[${kernel_size}+ 1]	
fi

##########################################################
# uboot | rootfs | kernel | user | kernel2 | user2 | mtd #
##########################################################
### add 1k space for header infomation
HEAD_SIZE=1
UBOOT_OFFSET=$[${HEAD_SIZE}]
UIMAGE_OFFSET=$[${UBOOT_OFFSET}+${UBOOT_SIZE_KB}]
ROOTFS_OFFSET=$[${UIMAGE_OFFSET}+${kernel_size}]
USER_OFFSET=$[${ROOTFS_OFFSET}+${root_size}]
UIMAGE2_OFFSET=$[${USER_OFFSET}+${user_size}]
ROOTFS2_OFFSET=$[${UIMAGE2_OFFSET}+${kernel_size}]
USER2_OFFSET=$[${ROOTFS2_OFFSET}+${root_size}]
MTD_OFFSET=$[${USER2_OFFSET}+${user_size}]
################Part 1 #################
dd if=/dev/zero of=binaries/FIRMWARE_DT20_L.bin bs=1k count=${HEAD_SIZE} ||exit 1
cd tools/gen_zone/
make clean
make
./gen_zone ../../binaries/FIRMWARE_DT20_L.bin 3 ${kernel_size} ${root_size} ${user_size}  || exit 1
cd ../..
dd if=uImage of=binaries/FIRMWARE_DT20_L.bin bs=1K seek=${HEAD_SIZE} ||exit 1
dd if=binaries/root.squashfs of=binaries/FIRMWARE_DT20_L.bin bs=1K seek=$[${HEAD_SIZE}+${kernel_size}] ||exit 1
dd if=binaries/user.squashfs of=binaries/FIRMWARE_DT20_L.bin bs=1K seek=$[${HEAD_SIZE}+${kernel_size}+${root_size}]||exit 1
# exit
./mkimage -A mips -T firmware -C none -n jz_fw -d binaries/FIRMWARE_DT20_L.bin binaries/${FW_NAME}.bin
################Part 1 #################
################Part 2 #################
dd if=/dev/zero of=binaries/FIRMWARE_DT20_L_F.bin bs=1k count=${HEAD_SIZE} ||exit 1
cd tools/gen_zone/
make
./gen_zone ../../binaries/FIRMWARE_DT20_L_F.bin 8 ${UBOOT_SIZE_KB} ${kernel_size} ${root_size} ${user_size} ${kernel_size} ${root_size} ${user_size} ${MTD_SIZE_KB}|| exit 1
cd ../..
dd if=u-boot-with-spl.bin of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${UBOOT_OFFSET} ||exit 1
dd if=uImage of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${UIMAGE_OFFSET}||exit 1
dd if=binaries/root.squashfs of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${ROOTFS_OFFSET} ||exit 1
dd if=binaries/user.squashfs of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${USER_OFFSET}||exit 1
dd if=uImage of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${UIMAGE2_OFFSET}||exit 1
dd if=binaries/root2.squashfs of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${ROOTFS2_OFFSET} ||exit 1
dd if=binaries/user2.squashfs of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${USER2_OFFSET}||exit 1
dd if=binaries/mtd.bin of=binaries/FIRMWARE_DT20_L_F.bin bs=1K seek=${MTD_OFFSET}||exit 1
./mkimage -A mips -T firmware -C none -n jz_fw -d binaries/FIRMWARE_DT20_L_F.bin binaries/${FW_NAME}_F.bin
################Part 2 #################


################generate md5 for ota##################
cp  binaries/${FW_NAME}.bin binaries/${FW_VERSION}.bin
rm -fr binaries/FIRMWARE_DT20*
