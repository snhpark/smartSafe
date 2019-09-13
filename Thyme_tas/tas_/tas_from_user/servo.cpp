#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#define BUFFER_LENGTH 256 // The buffer length
#define DEVICE_FILENAME "/dev/servo_driver"


static char receive[BUFFER_LENGTH];
int divisor = 192;
int range = 200;
int mark = 150;
int send[3];
int ret;
int i;
int fd;
int init_box() {
	send[0] = 0;
	send[1] = 192;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
	send[0] = 1;
	send[1] = 2000;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
	return 0;
}
// open = 1
//close = 2
int open_door() {

	send[0] = 2;
	send[1] = 150;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}

	return 0;
}
int close_door() {
	send[0] = 2;
	send[1] = 50;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}

	return 0;
}

int main(int argc, char *argv[])
{

	fd = open(DEVICE_FILENAME, O_RDWR | O_NDELAY); //Open the device with read/write access
	if (fd < 0) {
		perror("Failed to open device... ");
		return errno;
	}
	init_box();

	if (argc == 2) {
		if (strcmp(argv[1], "1")==0) open_door();
		if (strcmp(argv[1], "2")==0) close_door();
	}
	
	return 0;
}
