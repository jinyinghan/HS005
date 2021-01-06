#ifndef __AUDIO_CODEC_H__
#define __AUDIO_CODEC_H__
#include "fdk-aac/aacenc_lib.h"
#include "fdk-aac/aacdecoder_lib.h"

#include "adpcm.h"

/*AAC*/
#define PER_SAMPLE_BYTE			2
#define CHANNEL 1

/*
ERR_CONCEAL: Error concealment method
0: Spectral muting.
1: Noise substitution (see CONCEAL_NOISE).
2: Energy interpolation (adds additional signal delay of one frame, see CONCEAL_INTER).
*/
#define ERR_CONCEAL			2

/*
DRC_BOOST: Dynamic Range Control: boost, where [0] is none and [127] is max boost
*/
#define DRC_BOOST			0

/*
DRC_CUT: Dynamic Range Control: attenuation factor, where [0] is none and [127] is max compression
*/
#define DRC_CUT				0

/*
DRC_LEVEL: Dynamic Range Control: reference level, quantized to 0.25dB steps where [0] is 0dB and [127] is -31.75dB
*/
#define DRC_LEVEL			0

/*
DRC_HEAVY: Dynamic Range Control: heavy compression, where [1] is on (RF mode) and [0] is off
*/
#define DRC_HEAVY			0

/*
LIMIT_ENABLE: Signal level limiting enable
-1: Auto-config. Enable limiter for all non-lowdelay configurations by default.
0: Disable limiter in general.
1: Enable limiter always. It is recommended to call the decoder with a AACDEC_CLRHIST flag to reset all states when the limiter switch is changed explicitly.
*/
#define LIMIT_ENABLE			-1

/*
1024 or 960 for AAC-LC
2048 or 1920 for HE-AAC (v2)
512 or 480 for AAC-LD and AAC-ELD
*/
#define PCM_BUF_SIZE			(2048 * CHANNEL * PER_SAMPLE_BYTE)

#define AAC_BUF_SIZE			768



int MY_Audio_PCM2G711a(char *InAudioData, char *OutAudioData, int DataLen);
int MY_Audio_G711a2PCM(char *InAudioData, char *OutAudioData, int DataLen);

int aac_encoder_init(int sample_rate,int bitrate, AACENC_InfoStruct *info);
int audio_encoder_start(int sampleRate, int *aac_len , int *pcm_len);
int MY_Audio_PCM2AAC(unsigned char * InAudioData, int InLen ,unsigned char * OutAudioData,int Outlen);
int aac_decoder_start();
int aac_decoder_stop();
int aac_decode(unsigned char *src,  int  src_len  );

extern unsigned char *aac_decoder_buf;


#endif	/*__AUDIO_CODEC_H__*/
