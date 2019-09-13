#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define FILE_DEVICE    "/dev/sound_sensor"  /* Char device driver file. */
#define SOUND 10

int sensor_Val;


int main(int argc, char *argv[])
{
    char buf[16];
    int fp;
	FILE *f;
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
     printf("Sound Detected %c\n",buf[0]);
	 if (buf[0] == '1' && sensor_Val / SOUND != 1)
	 {
		 sensor_Val += SOUND;
	 }
	 f = fopen("out2.txt", "w");
	 fflush(f);
	 fprintf(f, "%d", sensor_Val);
	 fclose(f);
	 sleep(1);
	 f = fopen("out2.txt", "w");
	 fflush(f);
	 sensor_Val -= SOUND;
	 fprintf(f, "%d", sensor_Val);
	 fclose(f);
    /* Clear buffer. */
    memset(buf, 0, 16);
  }

  return 0;
}
