#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#define BUTTON 0	//  use GPIO17 as a button

static uint8_t matrix[3][8] = {};
static int pos[3] = {3, 3};
int currentColor = 5;
int color[8][3] = {
	{0,0,0},
	{0,0,1},
	{0,1,0},
	{0,1,1},
	{1,0,0},
	{1,0,1},
	{1,1,0},
	{1,1,1}
};

void display();
int getHorz();
int getVert();
int getI2CData(int);
void setColor();

int main(void){
	int i,j;
	for (i=0;i<3;i++){	// init the matrix
		for (j=0;j<8;j++){
			matrix[i][j] = 0xff;
		}
	}

	wiringPiSetup();	// necessary for wiringPi
	pinMode(BUTTON, INPUT);
	wiringPiSPISetup(0, 500000); // necessary for SPI in wiringPi
	while(1){
		int state = digitalRead(BUTTON);	// get the state of button
		if(state==0){	// if it's pressed, change the current color
			if(currentColor==7){ currentColor=0; }
			else{ currentColor++; }
		}
		int Vert = getVert();	// get vert from adc
		int Horz = getHorz();	// get horz form adc
		if (Vert<100){			// set the pos
			if(pos[0]>0){ pos[0]-=1; }
		}else if(Vert>32000){
			if(pos[0]<7){ pos[0]+=1; }
		}

		int i;
		for(i=0;i<20;i++){	// display the matrix for a better performace
			display();		// otherwise it looks more like blinking leds
		}

		if (Horz<100){		// set the pos
			if(pos[1]>0){ pos[1]-=1; }
		}else if(Horz>32000){
			if(pos[1]<7){ pos[1]+=1; }
		}
		setColor();
	}
	return 0;
}

void display(){
	int i;
	static uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
	for (i=0;i<8;i++){
		data[0] = matrix[0][i];	// send Red data
		data[1] = matrix[1][i];	// send Green data
		data[2] = matrix[2][i];	// send Blue data
		data[3] = 0x01 << i;	// send row
		wiringPiSPIDataRW(0, data, sizeof(data));
	}
}

int getI2CData(int n){
	int file;
	if((file = open("/dev/i2c-1", O_RDWR)) < 0){	// open i2c bus
		printf("Failed to open the bus.");
		exit(1);
	}
	ioctl(file, I2C_SLAVE, 0x48);
	char config[3] = {0};
	config[0] = 0x01;
	if(n==0){				// different value for different ports of adc
		config[1] = 0xE5;	// 11100101
	}else{
		config[1] = 0xD5;	// 11010101
	}
	config[2] = 0x84;
	write(file, config, 3);
	int i;
	for(i=0;i<20;i++){
		display();
	}
	char reg[1] = {0};
	write(file, reg, 1);
	char recv[2] = {0};
	if(read(file, recv, 2) != 2){	// get the raw data
		printf("Error: Read Error.");
		exit(1);
	}else{
		int raw_adc = (recv[0]<<8) + recv[1];	// compute adc data
		close(file);
		return raw_adc;
	}
}

int getHorz(){
	return getI2CData(1);
}

int getVert(){
	return getI2CData(0);
}
void setColor(){
	int i=0;
	for (i=0;i<3;i++){	// set current color to the certain point of matrix using binary oprands
		if (color[currentColor][i]==1){
			matrix[i][pos[0]] |= (1 << pos[1]);
		}else{
			matrix[i][pos[0]] &= ~(uint8_t)(1 << pos[1]);
		}
	}
}
