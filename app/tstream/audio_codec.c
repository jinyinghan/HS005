#include "include/common_env.h"
#include "include/common_func.h"

#include "audio_codec.h"

static HANDLE_AACENCODER hEncoder;
static HANDLE_AACDECODER hDecoder;

unsigned char *aac_decoder_buf = NULL;
static uint32_t aac_decoder_len = PCM_BUF_SIZE ;//(2048 * CHANNEL * PER_SAMPLE_BYTE);

/* My G711 Encoder */
#define SIGN_BIT    (0x80)      /* Sign bit for a A-law byte. */
#define QUANT_MASK  (0xf)       /* Quantization field mask. */
#define NSEGS       (8)         /* Number of A-law segments. */
#define SEG_SHIFT   (4)         /* Left shift for segment number. */
#define SEG_MASK    (0x70)      /* Segment field mask. */
#define BIAS        (0x84)      /* Bias for linear code. */

static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,
	0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(int val, short	*table, int	size)
{
	int	i;

	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return (i);
	}
	return (size);
}

static unsigned char linear2alaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	aval;

	if (pcm_val >= 0) {
		mask = 0xD5;		/* sign (7th) bit = 1 */
	} else {
		mask = 0x55;		/* sign bit = 0 */
		pcm_val = -pcm_val - 8;
	}

	seg = search(pcm_val, seg_end, 8);
	if (seg >= 8)		/* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		aval = seg << SEG_SHIFT;
		if (seg < 2)
			aval |= (pcm_val >> 4) & QUANT_MASK;
		else
			aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
		return (aval ^ mask);
	}
}

static int my_g711a_encode(unsigned char g711_data[], const short amp[], int len)
{
	int i;

	for (i = 0;  i < len;  i++) {
		g711_data[i] = linear2alaw(amp[i]);
	}

	return len;
}

int MY_Audio_PCM2G711a(char *InAudioData, char *OutAudioData, int DataLen)
{
	int Retaen = 0;

	if((NULL == InAudioData) && (NULL == OutAudioData) && (0 == DataLen)) {
		printf("Error, empty data or transmit failed, exit !\n");
		return -1;
	}

	Retaen = my_g711a_encode((unsigned char *)OutAudioData, (short*)InAudioData, DataLen/2);

	return Retaen;
}

static int alaw2linear( unsigned char a_val )
{
	int	t;
	int	seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ( (unsigned)a_val & SEG_MASK ) >> SEG_SHIFT;
	switch (seg)
	{
		case 0:
			t += 8;
			break;
		case 1:
			t += 0x108;
			break;
		default:
			t += 0x108;
			t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}

static int my_g711a_decode( short amp[], const unsigned char g711a_data[], int g711a_bytes )
{
	int i;
	int samples;
	unsigned char code;
	int sl;

	for ( samples = i = 0; ; )
	{
		if (i >= g711a_bytes)
			break;
		code = g711a_data[i++];

		sl = alaw2linear( code );

		amp[samples++] = (short) sl;
	}
	return samples*2;
}

int MY_Audio_G711a2PCM(char *InAudioData, char *OutAudioData, int DataLen)
{
	int Retade = 0;

	if((NULL == InAudioData) && (NULL == OutAudioData) && (0 == DataLen)) {
		printf("Error, empty data or transmit failed, exit !\n");
		return -1;
	}

	Retade = my_g711a_decode((short*)OutAudioData, (unsigned char *)InAudioData, DataLen);

	return Retade;
}


/*************************AAC encoder*****************************/
int aac_encoder_init(int sample_rate,int bitrate, AACENC_InfoStruct *info)
{
	int vbr = 0 , eld_sbr = 0 ;
	int aot = 2;
	CHANNEL_MODE mode = MODE_1;

	if(aacEncOpen(&hEncoder, 0x01|0x02|0x10, CHANNEL) != AACENC_OK) {
        printf( "Unable to open encoder\n");
        return 1;
    }
    printf("aacEncOpen ok \n");

	#if 0
	#define JZ_SAMPLE_FRAME_LENGTH  640
    if (aacEncoder_SetParam(hEncoder, AACENC_GRANULE_LENGTH, JZ_SAMPLE_FRAME_LENGTH) != AACENC_OK) {
        printf("Unable to set the frame length\n");
        return 1;
    }
	#endif
	
    if (aacEncoder_SetParam(hEncoder, AACENC_AOT, aot) != AACENC_OK) {
        printf("Unable to set the AOT\n");
        return 1;
    }
    if (aot == 39 && eld_sbr) {
        if (aacEncoder_SetParam(hEncoder, AACENC_SBR_MODE, 1) != AACENC_OK) {
            printf("Unable to set SBR mode for ELD\n");
            return 1;
        }
    }
    if (aacEncoder_SetParam(hEncoder, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
        printf( "Unable to set the AOT\n");
        return 1;
    }
    if (aacEncoder_SetParam(hEncoder, AACENC_CHANNELMODE, mode) != AACENC_OK) {
        printf("Unable to set the channel mode\n");
        return 1;
    }
    if (aacEncoder_SetParam(hEncoder, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        printf("Unable to set the wav channel order\n");
        return 1;
    }
    if (vbr) {
        if (aacEncoder_SetParam(hEncoder, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
            printf( "Unable to set the VBR bitrate mode\n");
            return 1;
        }
    } else {
        if (aacEncoder_SetParam(hEncoder, AACENC_BITRATE, bitrate) != AACENC_OK) {
            printf("Unable to set the bitrate\n");
            return 1;
        }
    }
    if (aacEncoder_SetParam(hEncoder, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {       //ADTS
        printf("Unable to set the ADTS transmux\n");
        return 1;
    }
    if (aacEncoder_SetParam(hEncoder, AACENC_AFTERBURNER, 0) != AACENC_OK) {
        printf( "Unable to set the afterburner mode\n");
        return 1;
    }
    if (aacEncEncode(hEncoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
        printf( "Unable to initialize the encoder\n");
        return 1;
    }
    if (aacEncInfo(hEncoder, info) != AACENC_OK) {
        printf( "Unable to get the encoder info\n");
        return 1;
    }

	return 0;
}


int audio_encoder_start(int sampleRate, int *aac_len , int *pcm_len)
{
	int ret = 0;
	AACENC_InfoStruct aac_enc_info = {0};

	/*AAC encoder init */
	if(aac_encoder_init(sampleRate,AUDIO_SAMPLERATE_YSX,&aac_enc_info) != 0)
	{
		printf("aac_encoder_init failed !\n");
		return -1;
	}
	*aac_len = aac_enc_info.maxOutBufBytes;
	*pcm_len = aac_enc_info.frameLength * CHANNEL * 2;
	printf("################input frameLength = %d \n",aac_enc_info.frameLength);
	return ret;
}

/*
arguments:
	InAudioData : source data  (pcm)
	InLen : the lenght of InAudioData
	OutAudioData: dest data
	Outlen :  max size of OutAudioData
return : The actual length  OutAudioData if successfully
*/
int MY_Audio_PCM2AAC(unsigned char * InAudioData, int InLen ,unsigned char * OutAudioData,int Outlen)
{
	int ret;
	int pcm_buf_identifier = IN_AUDIO_DATA, aac_buf_identifier = OUT_BITSTREAM_DATA;
	int pcm_element_size = 2, aac_element_size = 1;
	AACENC_BufDesc in_buf   = { 0 }, out_buf = { 0 };
	AACENC_InArgs  in_args  = { 0 };
	AACENC_OutArgs out_args = { 0 };
	AACENC_ERROR err;

	int in_size = InLen , out_size = Outlen;

	in_args.numInSamples = in_size/2;
	in_buf.numBufs = 1;
	in_buf.bufs = (void**)&InAudioData;
	in_buf.bufferIdentifiers = &pcm_buf_identifier;
	in_buf.bufSizes = &in_size;
	in_buf.bufElSizes = &pcm_element_size;

	out_buf.numBufs = 1;
	out_buf.bufs = (void**)&OutAudioData;
	out_buf.bufferIdentifiers = &aac_buf_identifier;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = &aac_element_size;

	if((err = aacEncEncode(hEncoder, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK)
	{
		if (err == AACENC_ENCODE_EOF)
			printf( "[%s:%d]AAC ENC EOF\n",__FUNCTION__,__LINE__);
		else
			printf( "[%s:%d]Encoding failed\n",__FUNCTION__,__LINE__);
		return -1;
	}

	ret = out_args.numOutBytes ;
	return ret;
}


/****************AAC decoder ***********************/
int aac_decoder_init()
{
	int retval = 0;

	hDecoder = aacDecoder_Open(TT_MP4_ADTS, 1);
	if(hDecoder == NULL)
	{
		LOG( "Error opening decoder\n");
		retval = -11;
		goto out;
	}

	if(aacDecoder_SetParam(hDecoder, AAC_CONCEAL_METHOD, ERR_CONCEAL) != AAC_DEC_OK)
	{
		LOG( "Unable to set error concealment method\n");
		retval = -12;
		goto err;
	}

	if(aacDecoder_SetParam(hDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, CHANNEL) != AAC_DEC_OK)
	{
		LOG( "Unable to set output channels in the decoder\n");
		retval = -13;
		goto err;
	}


	if(aacDecoder_SetParam(hDecoder, AAC_DRC_BOOST_FACTOR, DRC_BOOST) != AAC_DEC_OK)
	{
		LOG( "Unable to set DRC boost factor in the decoder\n");
		retval = -14;
		goto err;
	}

	if(aacDecoder_SetParam(hDecoder, AAC_DRC_ATTENUATION_FACTOR, DRC_CUT) != AAC_DEC_OK)
	{
		LOG( "Unable to set DRC attenuation factor in the decoder\n");
		retval = -15;
		goto err;
	}

	if(aacDecoder_SetParam(hDecoder, AAC_DRC_REFERENCE_LEVEL, DRC_LEVEL) != AAC_DEC_OK)
	{
		LOG( "Unable to set DRC reference level in the decoder\n");
		retval = -16;
		goto err;
	}

	if(aacDecoder_SetParam(hDecoder, AAC_DRC_HEAVY_COMPRESSION, DRC_HEAVY) != AAC_DEC_OK)
	{
		LOG( "Unable to set DRC heavy compression in the decoder\n");
		retval = -17;
		goto err;
	}

/*
	if(aacDecoder_SetParam(handle, AAC_TPDEC_CLEAR_BUFFER, 1) != AAC_DEC_OK)
	{
		fprintf(stderr, "failed to clear buffer when flushing\n");
		retval = -19;
		goto err;
	}
*/


out:
	return retval;

err:
	aacDecoder_Close(hDecoder);
	return retval;
}

#define AUDIO_TEST
static uint8_t first_decode = 0;
int aac_decode(unsigned char *src,  int  src_len )
{
	AAC_DECODER_ERROR err;
	unsigned int valid_bytes  , sample_rate,dst_len = 0;
	static unsigned int frame_size = 0;

#ifdef AUDIO_TEST
	static FILE *out_fp = NULL;
	if(NULL == out_fp)
	{
		out_fp= fopen("recv.pcm", "ab+");
		if(out_fp == NULL)
		{
			LOG( "open output file %s failed\n","recv.pcm" );
			return -1;
		}
	}
	fwrite(src,  1, src_len, out_fp);
#endif

	valid_bytes = src_len;

refill:

	if(aacDecoder_Fill(hDecoder, (UCHAR **)&src, (const UINT *)&src_len, &valid_bytes) != AAC_DEC_OK)
	{
		LOG( "aacDecoder_Fill failed\n");
		return -1;
	}

	err = aacDecoder_DecodeFrame(hDecoder, (INT_PCM *)aac_decoder_buf, aac_decoder_len, 0);
	if(err == AAC_DEC_NOT_ENOUGH_BITS)
	{
		if(valid_bytes)
			goto refill;

		return 0;
	}

	if(err != AAC_DEC_OK)
	{
		LOG( "aacDecoder_DecodeFrame failed --- 0x%08x\n", err);
		return -1;
	}

	if(first_decode == 1)
	{
		CStreamInfo *info = aacDecoder_GetStreamInfo(hDecoder);
		if(info == NULL)
		{
			LOG( "Unable to get stream info\n");
			return -1;
		}

		if(info->sampleRate <= 0)
		{
			LOG( "Stream info not initialized\n");
			return -1;
		}

		sample_rate = info->sampleRate;
		frame_size = info->frameSize;
		LOG("sample rate:%d\n", sample_rate);
		LOG("frame size:%d\n", frame_size);

		first_decode = 0;
	}
	dst_len = frame_size * 2;
//	LOG("dst_len = %d\n",dst_len);


//#ifdef AUDIO_TEST
#if 0
		int res;
		res = fwrite(aac_decoder_buf,  PER_SAMPLE_BYTE, frame_size, out_fp);
		if(res <= 0)
		{
			LOG( "write file error\n");
			return -1;
		}
		fflush(out_fp);
#endif

	if(valid_bytes)
		goto refill;


	return dst_len;
}

int aac_decoder_start()
{
    first_decode = 1;

	aac_decoder_buf =(unsigned char *)mem_malloc(aac_decoder_len);
	if( !aac_decoder_buf ){
		perror("malloc for aac_decoder_buf error");
		return -1;
	}

	return 0;
}


int aac_decoder_stop()
{
    if(aac_decoder_buf)
        mem_free(aac_decoder_buf);

    aac_decoder_buf = NULL;
    first_decode = 0;
}
