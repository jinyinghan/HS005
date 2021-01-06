include Makefile.param
PWD := $(shell pwd)
MAKE := make


.PHONY: app install clean 
app: 
	make tstreamer
#	make aud_dtc
#	make aud_dtc_fcs
#	make apsta
#	make qr_scan
	make nvram
#	make ptz_init
#	make rtsp
#	make sysconf
#	make fwdownload

################################
## install 
install:
	make tstreamer_install
#	make audio_install
	make nvram_install
#	make ptz_init_install
#	make fwdownload_install
#	make qr_scan_install


################################
## clean 
clean:
	rm $(INS_DIR)/* -rf
	make tstreamer_clean
#	make nvram_clean
#	make fwdownload_clean
	

################################
## distclean 
distclean:
	rm $(INS_DIR)/* -rf



################################
## make app 	
audio:
	$(MAKE) -C $(PWD)/app/audio

tstreamer:
	$(MAKE) -C $(PWD)/app/tstream 

xiaocao_tstreamer:
	$(MAKE) -C $(PWD)/app/tstream_xiaocao 


aud_dtc:
	$(MAKE) -C $(PWD)/app/aud_dtc 
aud_dtc_fcs:
	$(MAKE) -C $(PWD)/app/aud_dtc_fcs 

#qr_scan:
#	$(MAKE) -C $(PWD)/app/qr_code

apsta:
	$(MAKE) -C $(PWD)/app/apsta 

nvram:
	$(MAKE) -C $(PWD)/app/nvram 

ptz_init:
	$(MAKE) -C $(PWD)/app/ptz_init

sysconf:
	$(MAKE) -C $(PWD)/app/sysconf

fwdownload:
	$(MAKE) -C $(PWD)/app/fwdownload

rtsp:
	$(MAKE) -C $(PWD)/app/rtsp

####################
###########
## make app clean
tstreamer_clean:
	$(MAKE) -C $(PWD)/app/tstream clean 

aud_dtc_clean:
	$(MAKE) -C $(PWD)/app/aud_dtc clean 

#qr_scan_clean:
#	$(MAKE) -C $(PWD)/app/qr_scan clean 

apsta_clean:
	$(MAKE) -C $(PWD)/app/apsta clean

audio_clean:
	$(MAKE) -C $(PWD)/app/audio clean

nvram_clean:
	$(MAKE) -C $(PWD)/app/nvram clean

ptz_init_clean:
	$(MAKE) -C $(PWD)/app/ptz_init clean

fwdownload_clean:
	$(MAKE) -C $(PWD)/app/fwdownload clean

rtsp_clean:
	$(MAKE) -C $(PWD)/app/rtsp clean

################################
## make app install
tstreamer_install:
	$(MAKE) -C $(PWD)/app/tstream install

#qr_scan_install:
#	$(MAKE) -C $(PWD)/app/qr_code install

tstreamer_xiaocao_install:
	$(MAKE) -C $(PWD)/app/tstream_xiaocao install

360iot_tsnew_install:
	$(MAKE) -C $(PWD)/app/360iot_tsnew install

audio_install:
	$(MAKE) -C $(PWD)/app/audio install

nvram_install:
	$(MAKE) -C $(PWD)/app/nvram install
	
ptz_init_install:
	$(MAKE) -C $(PWD)/app/ptz_init install

fwdownload_install:
	$(MAKE) -C $(PWD)/app/fwdownload install

middleware:
	$(MAKE) -C $(PWD)/private2/

middleware_clean:
	$(MAKE) -C $(PWD)/private2/ clean
################################
## make pack 
pack: #clean
	pushd $(PWD)/image;sh create_images.sh;popd;
	


all: app install
