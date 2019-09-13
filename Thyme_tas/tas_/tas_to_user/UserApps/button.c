#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_LENGTH 256 // The buffer length
#define BUTTON_DEV "/dev/button_driver"
#define BUTTON 1000


static char receive[BUFFER_LENGTH];
int sensor_Val = 0;

int main() {
	FILE * f;
	char buf[16];
	int ret, fd;
	int i;
	int fp;
	char stringToSend[BUFFER_LENGTH];
	int count;
	fd = open(BUTTON_DEV, O_RDWR | O_NDELAY); 
	
	if (fd < 0) {
		perror("Failed to open device... ");
		return errno;
	}


	printf("Reading from the device... \n");
	while (1) {
		ret = read(fd, receive, BUFFER_LENGTH);// Get the response from the LKM
		if (ret < 0) {
			perror("Failed to read the message from the device.");
			return errno;
		}
		for (i = 0; i < 17; i++) {
			printf("%d", receive[i]);
			if (receive[i] == 1 && sensor_Val / BUTTON != 1)
			{
				sensor_Val += BUTTON;
				break;
			}
		}
		printf("The received message is : ");
		f = fopen("out.txt", "w");
		fflush(f);
		fprintf(f, "%d", sensor_Val);
		fclose(f);
		if (sensor_Val / BUTTON >= 1) {
			sensor_Val -= BUTTON;
		}
		printf("\n");
		sleep(1);
	}
	printf("End of the program\n");
	return 0;
}
