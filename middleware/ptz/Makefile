TARGET 			:= mips-linux-gnu
###TARGET 			:= mips-linux-uclibc-gnu 
CROSS_COMPILE 	?= $(TARGET)-
CC				:= $(CROSS_COMPILE)gcc
STRIP			:= $(CROSS_COMPILE)strip --strip-unneeded
AR				:= $(CROSS_COMPILE)ar rcs
RM				:= rm -rf
CP				:= cp -rdf

CFLAGS :=
CFLAGS += -g -Wall -O2 ####-muclibc
CFLAGS += -D_GNU_SOURCE
CFLAGS += -fPIC

LDFLAGS :=
LDFLAGS += -lpthread
LDFLAGS += -lstdc++ 

OBJS := 
OBJS += uln2803.o
OBJS += ptz_ctrl.o

targets := 
targets += libmotor.a
targets += libmotor.so
##targets += ptz_test
targets += ptz_test2
targets += ptz_test_so

all: $(targets)
libmotor.a: $(OBJS)
	$(AR) -o $@ $(OBJS)	

libmotor.so: $(OBJS)
	$(CC) $^ -shared -o $@ $(LDFLAGS)
	cp libmotor.so  /home/lzf/nfs

ptz_test: test.c libmotor.a
	$(CC) -o $@ $^ $(LDFLAGS) 
	cp ptz_test  /home/lzf/nfs

ptz_test2: test2.c libmotor.a
	$(CC) -o $@ $^ $(LDFLAGS) 
	cp ptz_test2  /home/lzf/nfs
ptz_test_so:test2.c
	$(CC) -o $@ $^ $(LDFLAGS) -L . -lmotor
%.o : %.c
	$(CC) -c $^ $(CFLAGS)

clean:
	$(RM) *.o $(targets)
