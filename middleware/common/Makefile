CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar rcs
STRIP=mips-linux-gnu-strip
PWD         := $(shell pwd)

INGENIC_INC = $(PWD)/../../ipc_sdk/include    
#####QCAM_INC ?= $(MIDDLEWARE)/include
LIB_INSTALL = $(PWD)/lib

CFLAGS += -Wall -O2 -muclibc
CFLAGS += -I $(INGENIC_INC)
##CFLAGS += -I . 
#########CFLAGS += -I $(QCAM_INC)
    
INGENIC_LIB = $(PWD)/../../ipc_sdk/lib 
LDFLAGS :=
LDFLAGS += -lpthread -lstdc++ 
LDFLAGS += -L . -lalog -lsysutils
##LDFLAGS += -L$(PWD)/../../ipc_sdk/lib -lsysutils -lalog -lrt ##########-limp 
###LDFLAGS += -L$(PWD)/../../ipc_sdk/lib/libsysutils.a
###LDFLAGS += -L$(LIB_INSTALL)

SRC +=base64.c
SRC +=ysx_sys.c
SRC +=crc32.c
SRC +=wdt.c

OBJS +=base64.o 
OBJS +=ysx_sys.o
OBJS +=crc32.o
OBJS +=wdt.o
#OBJS := $(SRC:%.c=%.o)

#LIB += $(PWD)/../../ipc_sdk/lib/libsysutils.a


TARGET +=libysx_sys.so
TARGET +=libysx_sys.a
TARGET +=fwupdate
TARGET +=gpio_led
TARGET +=wdt
TARGET +=key_test
TARGET +=key_test_so
TARGET +=keyfunction
TARGET +=keyfunction_so

all:$(TARGET)

%.o:%.c
###	$(CC) $(CFLAGS) $(LDFLAGS)  -c $^
###	$(CC) -c $^ $(CFLAGS) $(LDFLAGS) -fPIC
	$(CC) -c $^ $(CFLAGS) -fPIC

##libysx_sys.so:$(SRC) 
###	$(CC) -fPIC -shared $(CFLAGS) $(LDFLAGS) $^ -o $@
libysx_sys.so: $(SRC) 
	$(CC) $^ -o $@ -fPIC -shared $(LDFLAGS) $(CFLAGS) 

libysx_sys.a:$(OBJS)   
	$(AR) $@ $^  	
keyfunction:keyfunction.c libysx_sys.a  
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ ########-L$(LIB_INSTALL)

keyfunction_so:keyfunction.c  
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -L . -lysx_sys
	
fwupdate:fwupdate.c  libysx_sys.a 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ ####-L$(LIB_INSTALL)
wdt:test_wdt.c      libysx_sys.a  
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ ####-L$(LIB_INSTALL)
key_test:key_test.c libysx_sys.a  
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ ####-L$(LIB_INSTALL)
key_test_so:key_test.c  
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -L . -lysx_sys

gpio_led:gpio_led.c libysx_sys.a 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@   #####-L$(LIB_INSTALL)
	####$(CC) $(CFLAGS)  $^ -o $@ -L$(LIB_INSTALL)

install:
	if [ ! -d $(LIB_INSTALL) ];then 	\
		mkdir -p $(LIB_INSTALL) ; 	\
	fi
	#$(STRIP) $(TARGET)
	install -c $(TARGET) $(LIB_INSTALL)
	if [ ! -d $(LIB_INSTALL) ];then 	\
		mkdir -p $(LIB_INSTALL)/../include ; 	\
	fi
	cp ysx_sys.h $(LIB_INSTALL)/../include/ -av 
clean:
	rm -fr *.o $(TARGET)
