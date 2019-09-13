#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#define BUFFER_LENGTH 256 // The buffer length
#define DEVICE_FILENAME "/dev/buzzer_driver"


static char receive[BUFFER_LENGTH];
int divisor = 192;
int range = 200;
int mark = 150;
int send[3];
int ret;
FILE *fp;
FILE *fp2;
int i;
int fd;
int init_box() {
	send[0] = 0;
	send[1] = 20;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
	send[0] = 0;
	send[1] = 0;
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
int buzzer_alert() {

	send[0] = 1;
	send[1] = 10;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
		
	return 0;
}
int stop_alert() {
	send[0] = 0;
	send[1] = 0;
	send[2] = 0;
	ret = write(fd, send, 3 * sizeof(int));// Send the string to the LKM

	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}
		
	return 0;
}

int main(void)
{
	int Sensor_Val = 0;
	char pir[5];
	char ul[5];
	fd = open(DEVICE_FILENAME, O_RDWR | O_NDELAY); //Open the device with read/write access
	if (fd < 0) {
		perror("Failed to open device... ");
		return errno;
	}
	init_box();
	while (1) {
		fp = fopen("out1.txt", "r");
		fp2 = fopen("out3.txt", "r");
		fgets(pir, sizeof(pir), fp);
		fgets(ul, sizeof(ul), fp2);
		Sensor_Val = atoi(pir) + atoi(ul);
		fclose(fp);
		fclose(fp2);
		//printf("Type in a short string to send to the kernel module\n");
		//fflush(stdin);
		//scanf("%d %d %d", &send[0], &send[1], &send[2]);
		//printf("Writing message to the device [%s].\n",stringToSend);
		
		printf("%d\n", Sensor_Val);
		sleep(1);
		if (Sensor_Val == 101) {
			buzzer_alert();
			printf("buzzer !!!!!");
			sleep(2);
			stop_alert();
		}
	}
	return 0;
}
