/*
 * A simple program to write flag to the binary.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define FLAG "WROK"

static int htoi(char s[])
{
	int n = 0;
	int i = 0;
	while (s[i] != '\0' && s[i] != '\n') {
		if (s[i] == '0') {
			if (s[i+1] == 'x' || s[i+1] == 'X')
                            i+=2;
		}
		if (s[i] >= '0' && s[i] <= '9') {
			n = n * 16 + (s[i] - '0');
		} else if (s[i] >= 'a' && s[i] <= 'f') {
			n = n * 16 + (s[i] - 'a') + 10;
		} else if (s[i] >= 'A' && s[i] <= 'F') {
			n = n * 16 + (s[i] - 'A') + 10;
		} else
			return -1;
		++i;

	}
	return n;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("usage: wr_flag [file] [offset by the end(hex)]\n");
		_exit(1);
	}

	int fd;
	char *file_path = argv[1];
	off_t offset = (off_t)atoi(argv[2]) * 1024;

	printf("file: %s, offset: 0x%08x\n", file_path, (uint32_t)offset);

	fd = open(file_path, O_RDWR);
	if (fd < 0) {
		perror("Input file open error");
		_exit(1);
	}

	off_t cur_off;
	cur_off = lseek(fd, offset - 4, SEEK_SET);
	if (cur_off != (offset - 4)) {
		printf("seek to %08x error cur_off=0x%08x\n", (uint32_t)offset, (uint32_t)cur_off);
		_exit(1);
	}

	ssize_t ret = write(fd, FLAG, 4);
	if (ret != 4) {
		printf("write flag error\n");
		_exit(1);
	}

	close(fd);
	printf("write flag finished\n");

    return 0;
}
