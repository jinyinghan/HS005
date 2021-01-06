#ifndef __COMMAND_HH__
#define __COMMAND_HH__

#include <RCF/RCF.hpp>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>

using namespace std;

class ISPEffect {
public:
	int wdrswitch;
	int denisise3dswitch;
	int iridixSwitch;
	unsigned char contrast;
	unsigned char brightness;
	unsigned char saturation;
        int enableMirror;
        int enableFlip;
	int sharpness;
	int antiflicker;

	void serialize(SF::Archive &ar) {
		ar &wdrswitch &denisise3dswitch &iridixSwitch \
                        &contrast &brightness &saturation &enableMirror \
			&enableFlip &sharpness &antiflicker;
	}
};

class SensorEffect {
public:
	uint32_t reg;
	uint32_t value;

	void serialize(SF::Archive &ar) {
		ar &reg &value;
	}
};

enum RCMethod {
	CBR,
	FixQP
};

class RcAttrCBR {
public:
	uint32_t maxGop;
	uint32_t outFrmRateNum;
	uint32_t outFrmRateDen;
	uint32_t outBitRate;
	uint32_t maxQp;
	uint32_t minQp;
	uint32_t maxFPS;
	uint32_t minFPS;
	int      IBiasLvl;
	uint32_t FrmQPStep;
	uint32_t GOPQPStep;
	bool	 AdaptiveMode;
	bool	 GOPRelation;

	void serialize(SF::Archive &ar) {
		ar &maxGop &outFrmRateNum &outFrmRateDen &outBitRate &maxQp &minQp &maxFPS \
			&minFPS &IBiasLvl &FrmQPStep &GOPQPStep &AdaptiveMode &GOPRelation;
	}
};

class RcAttrFixQP {
public:
	uint32_t maxGop;
	uint32_t outFrmRateNum;
	uint32_t outFrmRateDen;
	uint32_t qp;

	void serialize(SF::Archive &ar) {
		ar &maxGop &outFrmRateNum &outFrmRateDen &qp;
	}
};

class LiSaveInfo {
public:
	uint32_t	maxSize;
	string		path;

	void serialize(SF::Archive &ar) {
		ar &maxSize &path;
	}
};

class PTZInfo {
public:
    int     enable;
    int     speed;
    int		direction;

    void serialize(SF::Archive &ar) {
        ar &enable &speed &direction;
    }
};

RCF_BEGIN(I_CarrierCommand, "I_CarrierCommand")
/* ISP */
  RCF_METHOD_R1(int, WDRSwitch, int);
  RCF_METHOD_R1(int, Denoise3DSwitch, int);
  RCF_METHOD_R1(int, IridixSwitch, int);
  RCF_METHOD_R1(int, NightModeSwitch, int);
  RCF_METHOD_R1(int, GetWDRSwitch, int &);
  RCF_METHOD_R1(int, GetDenoise3DSwitch, int &);
  RCF_METHOD_R1(int, GetIridixSwitch, int &);
  RCF_METHOD_R1(int, GetNightModeSwitch, int &);
  RCF_METHOD_R1(int, GetISPEffect, ISPEffect &);
  RCF_METHOD_R1(int, SetISPEffect, ISPEffect &);
  RCF_METHOD_R1(int, GetSensorEffect, SensorEffect &);
  RCF_METHOD_R1(int, SetSensorEffect, SensorEffect &);
  RCF_METHOD_R2(int, GetSensorFPS, int &, int &);
  RCF_METHOD_R2(int, SetSensorFPS, int, int);
  RCF_METHOD_R1(int, EnableFramesource3D, int);

/* Encoder */
  RCF_METHOD_R2(int, GetRCMethod, int, enum RCMethod &);
  RCF_METHOD_R2(int, SetRCAttr, int, RcAttrCBR &);
  RCF_METHOD_R2(int, SetRCAttr, int, RcAttrFixQP &);
  RCF_METHOD_R2(int, GetRCAttr, int, RcAttrCBR &);
  RCF_METHOD_R2(int, GetRCAttr, int, RcAttrFixQP &);
  RCF_METHOD_R3(int, GetBitRate, int, double &, double &);
  RCF_METHOD_R2(int, GetSecondVideoInfo, int &, int &);
  RCF_METHOD_R2(int, GetMainVideoInfo, int &, int &);

/* Audio */
  RCF_METHOD_R0(int, AudioStart);
  RCF_METHOD_R0(int, AudioStop);

/* IVS */
  RCF_METHOD_R1(int, LineDetectionSwitch, int);
  RCF_METHOD_R1(int, MotionDetectionSwitch, int);
  RCF_METHOD_R1(int, HumanoidDetectionSwitch, int);
  RCF_METHOD_R1(int, MixhumanoidDetectionSwitch, int);
  RCF_METHOD_R1(int, FaceDetectionSwitch, int);
  RCF_METHOD_R4(int, DrawLine, int, int, int, int);
  RCF_METHOD_R4(int, RemoveLine, int, int, int, int);
  RCF_METHOD_R0(int, ClearLines);
  RCF_METHOD_R1(int, RoiSwitch, int);
  RCF_METHOD_R1(int, SdcSwitch, int);
  RCF_METHOD_R1(int, GetLineDetectionSwitch, int &);
  RCF_METHOD_R1(int, GetMotionDetectionSwitch, int &);
  RCF_METHOD_R1(int, GetHumanoidDetectionSwitch, int &);
  RCF_METHOD_R1(int, GetMixhumanoidDetectionSwitch, int &);
  RCF_METHOD_R1(int, GetRoiSwitch, int &);
  RCF_METHOD_R1(int, GetSdcSwitch, int &);
  RCF_METHOD_R1(int, GetFaceDetectionSwitch, int &);
  RCF_METHOD_R1(int, PermDetectionSwitch, int);
  RCF_METHOD_R8(int, DrawPerm, int, int, int, int, int, int, int, int);
  RCF_METHOD_R0(int, RemovePerm);
  RCF_METHOD_R1(int, SetMovesense, int);

/* Resolution */
  RCF_METHOD_R2(int, GetResolution, int &, int &);
  RCF_METHOD_R2(int, SetResolution, int, int);

/* ByteBuffer */
  RCF_METHOD_R1(int, DownloadInit, std::size_t);
  RCF_METHOD_R0(int, DownloadExit);
  RCF_METHOD_R0(int, DownloadRecordStart);
  RCF_METHOD_R0(int, DownloadRecordStop);
  RCF_METHOD_R0(int, DownloadRecordOpen);
  RCF_METHOD_R0(int, DownloadRecordClose);
  RCF_METHOD_R0(long int, DownloadRecordGetinfo);
  RCF_METHOD_R2(RCF::ByteBuffer, DownloadRecord, int, int);
  RCF_METHOD_R1(int, DownloadPictureStart, int);
  RCF_METHOD_R0(int, DownloadPictureOpen);
  RCF_METHOD_R0(int, DownloadPictureClose);
  RCF_METHOD_R0(long int, DownloadPictureGetinfo);
  RCF_METHOD_R2(RCF::ByteBuffer, DownloadPicture, int, int);

/* connect */
  RCF_METHOD_R0(int, Connect);

/* PTZ */
  RCF_METHOD_R1(int, SetPTZ, PTZInfo &);
  RCF_METHOD_R0(int, SetPTZOpen);
  RCF_METHOD_R0(int, SetPTZClose);
  RCF_METHOD_R1(int, SetPTZSpeed, int);

/* IRCUT */
  RCF_METHOD_R1(int, SetIRCUT, int);
RCF_END(I_CarrierCommand);

class CarrierCommandImpl {
public:
  CarrierCommandImpl();
  ~CarrierCommandImpl();

/* ISP */
  int WDRSwitch(int on);
  int Denoise3DSwitch(int on);
  int IridixSwitch(int on);
  int NightModeSwitch(int on);
  int GetWDRSwitch(int &on);
  int GetDenoise3DSwitch(int &on);
  int GetIridixSwitch(int &on);
  int GetNightModeSwitch(int &on);
  int GetISPEffect(ISPEffect &parameters);
  int SetISPEffect(ISPEffect &parameters);
  int GetSensorEffect(SensorEffect &parameters);
  int SetSensorEffect(SensorEffect &parameters);
  int GetSensorFPS(int &fpsNum, int &fpsDen);
  int SetSensorFPS(int fpsNum, int fpsDen);
  int EnableFramesource3D(int on);

/* Encoder */
  int GetRCMethod(int chn, enum RCMethod &rc);
  int GetRCAttr(int chn, RcAttrCBR &attrCbr);
  int SetRCAttr(int chn, RcAttrCBR &attrCbr);
  int GetRCAttr(int chn, RcAttrFixQP &attrFixQp);
  int SetRCAttr(int chn, RcAttrFixQP &attrFixQp);
  int GetBitRate(int chn, double &fps, double &bitrate);
  int GetSecondVideoInfo(int &w, int &h);
  int GetMainVideoInfo(int &w, int &h);

/* Audio */
  int AudioStart(void);
  int AudioStop(void);

/* IVS */
  int LineDetectionSwitch(int on);
  int MotionDetectionSwitch(int on);
  int HumanoidDetectionSwitch(int on);
  int MixhumanoidDetectionSwitch(int on);
  int FaceDetectionSwitch(int on);
  int RoiSwitch(int on);
  int SdcSwitch(int on);
  int GetLineDetectionSwitch(int &on);
  int GetMotionDetectionSwitch(int &on);
  int GetHumanoidDetectionSwitch(int &on);
  int GetMixhumanoidDetectionSwitch(int &on);
  int GetFaceDetectionSwitch(int &on);
  int GetRoiSwitch(int &on);
  int GetSdcSwitch(int &on);
  int DrawLine(int x0, int y0, int x1, int y1);
  int RemoveLine(int x0, int y0, int x1, int y1);
  int ClearLines(void);
  int PermDetectionSwitch(int on);
  int DrawPerm(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  int RemovePerm(void);
  int SetMovesense(int sensor);

/* Resolution */
  int GetResolution(int &w, int &h);
  int SetResolution(int w, int h);

/* ByteBuffer */
  int DownloadInit(std::size_t);
  int DownloadExit(void);
  int DownloadRecordStart(void);
  int DownloadRecordStop(void);
  int DownloadRecordOpen(void);
  int DownloadRecordClose(void);
  long int DownloadRecordGetinfo(void);
  RCF::ByteBuffer DownloadRecord(int, int);
  int DownloadPictureStart(int);
  int DownloadPictureOpen(void);
  int DownloadPictureClose(void);
  long int DownloadPictureGetinfo(void);
  RCF::ByteBuffer DownloadPicture(int, int);

/* connect */
  int Connect(void);

/* PTZ */
  int SetPTZ(PTZInfo &info);
  int SetPTZOpen();
  int SetPTZClose();
  int SetPTZSpeed(int );

/* IRCUT */
  int SetIRCUT(int );

private:
  IMPEncoderRcAttr rc_attr;

  int fdPicture;
  int fdRecord;
  RCF::ByteBuffer *bytebuffer;
};

#endif /* __COMMAND_HH__ */
