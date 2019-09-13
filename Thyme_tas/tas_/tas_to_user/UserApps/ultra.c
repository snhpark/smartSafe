#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#define HCIN  1
#define HCOUT 0
#define ultra 1

int sensor_Val = 0;
int main(void){

    float dist,s,e;
	FILE *f;
    if(wiringPiSetup()==-1)
        return 0;

    pinMode(HCIN, INPUT);
    pinMode(HCOUT, OUTPUT);
    
    while(1){
        digitalWrite(HCOUT, LOW);
        digitalWrite(HCOUT, HIGH);
        delayMicroseconds(10);
        digitalWrite(HCOUT, LOW);

        while(digitalRead(HCIN)==LOW)
            s=micros();
        while(digitalRead(HCIN)==HIGH)
            e=micros();
        dist = (e-s)/58;
        //printf("distance %f\n",dist);
		
        if(dist<=20.0){
            printf("20cm내 물체감지\n");
			sensor_Val += ultra;
            }
		f = fopen("out3.txt", "w");
		fflush(f);
		fprintf(f, "%d", sensor_Val);
		fclose(f);
		if (sensor_Val / ultra >= 1) {
			sensor_Val -= ultra;
		}
        delay(1000);
    }
    return 0;
}
