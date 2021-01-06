CFLAGS :=
CFLAGS += -g -O2 -muclibc
CFLAGS += -D_GNU_SOURCE
all:
	mips-linux-gnu-gcc ptz_test.c -o ptz_test $(CFLAGS)
	cp ptz_test /home/lzf/nfs
	mips-linux-gnu-gcc ptz_init.c -o ptz_init $(CFLAGS)
	cp ptz_init /home/lzf/nfs/


