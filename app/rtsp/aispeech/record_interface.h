/**
 * @file record_interface.h
 * @brief record interface
 * @author jbxu <jibin.xu@ingenic.com>
 * @version 1.0
 * @date 2015-03-31
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 *
 * The program is not free, Ingenic without permission,
 * no one shall be arbitrarily (including but not limited
 * to: copy, to the illegal way of communication, display,
 * mirror, upload, download) use, or by unconventional
 * methods (such as: malicious intervention Ingenic data)
 * Ingenic's normal service, no one shall be arbitrarily by
 * software the program automatically get Ingenic data
 * Otherwise, Ingenic will be investigated for legal responsibility
 * according to law.
 */

/**
 * @brief buffer size on once record
 */
#define READ_LEN 1024

/**
 * @brief record struct.
 */
typedef struct dmic_record {
	/**
	 * @brief sample rate, such 8000, 16000, 32000...
	 */
	long sample_rate;
	/**
	 * @brief sample bits, such 8, 16...
	 */
	int bits;
	/**
	 * @brief channels number, 1 or 2
	 */
	int channels;
	/**
	 * @brief fd of /dev/dsp
	 */
	int fd_record;
	/**
	 * @brief fd of /dev/mixer
	 */
	int fd_mixer;
	/**
	 * @brief record volume, 0~100
	 */
	int record_volume;
	/**
	 * @brief record data buffer size
	 */
	int data_size;
	/**
	 * @brief record data buffer
	 */
	char *data_buf;
}dmic_record;

/**
 * @brief init sound card. such as sample rate, channels etc...
 *
 * @param record_info [in-out] record struct, init sound card according to set.
 *
 * @return return 0 on success, else -1.
 */
extern int mozart_sound_card_init(dmic_record *record_info);

/**
 * @brief uninit sound card.
 *
 * @param record_info [in] uninit which record process?
 *
 * @return return 0 on success, else -1.
 */
extern int mozart_sound_card_uninit(dmic_record *record_info);

/**
 * @brief record one buffer(READ_LEN bytes)
 *
 * @param record_info [in-out] pass on the fd of /dev/dsp, and provide record data buffer.
 *
 * @return return 0 on success, else -1.
 */
extern int mozart_record_one_buf(dmic_record *record_info);

/**
 * @brief record some buffer
 *
 * @param record_info, the record info, pass on record buffer and record buffer size.
 *
 * @return return 0 on success, else -1.
 */
extern int mozart_record(dmic_record *record_info);
