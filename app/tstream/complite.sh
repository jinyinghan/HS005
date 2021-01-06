#########################################################################
# File Name: complite.sh
# Author: unixcc
# mail: 2276626887@qq.com
# Created Time: 2018年08月30日 星期四 02时38分31秒
#########################################################################
#!/bin/bash
mips-linux-gnu-gcc network.c -o  network -Wall -O2 -muclibc -I./
cp network /home/winshare
