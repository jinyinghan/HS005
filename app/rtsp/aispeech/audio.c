#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>

#define SNDCTL_EXT_SET_REPLAY_VOLUME        _SIOR ('P', 90, int)
#define SNDCTL_EXT_SET_RECORD_VOLUME        _SIOR ('P', 103, int)

int set_volume(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > 0xcb)
		volume = 0xcb;

	int AudioFileNo = open("/dev/dsp", O_WRONLY, 644);
	if (AudioFileNo < 0) {
		printf( "Unable to open \"/dev/dsp\"\n");
		return -1;
	}

	if (ioctl(AudioFileNo, SNDCTL_EXT_SET_REPLAY_VOLUME, &volume) < 0) {
		printf("set replay volume error\n");
		return -1;
	}

	/*volume = 10;*/
	/*if (ioctl(AudioFileNo, SNDCTL_EXT_SET_RECORD_VOLUME, &volume) < 0) {*/
		/*printf("set reorcd volume error\n");*/
		/*return -1;*/
	/*}*/

	close(AudioFileNo);

	return 0;
}

int audio_play(char *file)
{
	int AudioFileNo = open("/dev/dsp", O_WRONLY, 644);
	if (AudioFileNo < 0) {
		printf( "Unable to open \"/dev/dsp\"\n");
		return -1;
	}

	FILE *play_fd = fopen(file, "rb");
	if (play_fd == NULL) {
		printf("fopen file %s error\n", file);
		return -1;
	}

	while(1) {
		unsigned char buf[512] = {0};
		int size = fread(buf, 1, 512, play_fd);
		if(size < 512)
			break;
		write(AudioFileNo, buf, size);
	}

	fclose(play_fd);
	close(AudioFileNo);

	return 0;

}
