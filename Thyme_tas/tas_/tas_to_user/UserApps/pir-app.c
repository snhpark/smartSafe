#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define FILE_DEVICE    "/dev/pir_sensor"  /* Char device driver file. */
#define PIR 100

int sensor_Val;

int main(int argc, char *argv[])
{
	FILE *f;
    char buf[16];
    int fp;

  /* Clear buffer. */
  memset(buf, 0, 16);

  /* Open char device driver. */
  fp = open(FILE_DEVICE, O_RDONLY);
  if (fp < 0) {
    perror("pir-sensor-app: open() pir-sensor driver failed: reason ");
    exit (-1);
  }

  /* Infinite loop, the process will sleep if there is nothing to read. */
  while(1) {
    if (read(fp, buf, 16) < 0) {
      perror("pir-sensor-app: read() pir-sensor driver failed: reason ");
      exit (-1);
    }

    /* Dummy print for debug. */
     printf("Motion Detected\n");
	 if (buf[0] == '1' && sensor_Val / PIR != 1)
	 {
		 sensor_Val += PIR;
	 }
	 f = fopen("out1.txt", "w");
	 fflush(f);
	 fprintf(f, "%d", sensor_Val);
	 fclose(f);
	 sleep(1);
	 f = fopen("out1.txt", "w");
	 fflush(f);
	 sensor_Val -= PIR;
	 fprintf(f, "%d", sensor_Val);
	 fclose(f);
	 /*if (sensor_Val / PIR >= 1) {
		 
		
	 }
	 prinft("%d", sensor_Val);
	 printf("\n");*/
		/* Clear buffer. */
	 memset(buf, 0, 16);
  }

  return 0;
}
