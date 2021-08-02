/*CPSC 359 Winter 2021
 *Project Part 1
 * Made by: Priyavart Rajain (30074184) and 
 *          Muhammad Taimur Rizwan (30078941)
 * The purpose of this program is to emulate the buttons of a SNES controller.
 * The user will be prompted to press a button on their controller and will
 * receive a message displaying which button is pressed
 * Program terminates when the user presses Start*/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "initGPIO.h"
#include "wiringPi.h"
#include <sys/mman.h>
#include "framebuffer.h"
#include "frog.h"
#include "background.h"
#include "pinkCarLeft.h"
#include <pthread.h>

#define CLK 11
#define LAT 9
#define DAT 10

/* Definitions */
typedef struct { 
	short int color;
	int x, y;
} Pixel;

int x_=0;
int y_=0;
int carX=1350;
struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);


/* draw picture function */
int drawPicture(){

	/* initialize + get FBS */
	framebufferstruct = initFbInfo();
	
	short int *frogPtr=(short int *) frog.pixel_data;
    short int *pinkCarPtr=(short int *) pinkCarLeft.pixel_data;
	short int *backgroundPtr=(short int *)background.pixel_data;

	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));

    int i=0;
	for (int y = 200; y < 504+200 ;y++)// 504 is the background height
	{ 
		for (int x = 895; x < 516+895; x++) // 516 is background width
		{	
				pixel->color = backgroundPtr[i]; 
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;
		} 
	}
	i=0;
	//unsigned int quarter,byte,word;
	for (int y = 600; y < 24+600 ;y++)// 24 is the car height
	{ 
		for (int x = carX; x < 35+carX; x++) // 35 is car width
		{	
				pixel->color = pinkCarPtr[i]; 
				pixel->x = x;
				pixel->y = y;
				drawPixel(pixel);
				i++;
				
		} 
	}
	i=0;
	//unsigned int quarter,byte,word;
	for (int y = 675+y_; y < 27+675+y_ ;y++)// 27 is the frog height
	{ 
		for (int x = 1125+x_; x < 24+1125+x_ ; x++) // 24 is frog width
		{	
				pixel->color = frogPtr[i]; 
				pixel->x = x;
				pixel->y = y;

				drawPixel(pixel);
				i++;
				
		} 
	}

	/* free pixel's allocated memory */
	free(pixel);
	pixel = NULL;
	munmap(framebufferstruct.fptr, framebufferstruct.screenSize);
	
	return 0;
}

/* Draw a pixel */
void drawPixel(Pixel *pixel){
	long int location =(pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}


/*Function to initialize a GPIO line
 * line no., function code (1 or 0) and gpio base pointer are passed as arguments*/
void init_GPIO(int line, int fncode, unsigned int *gpio){
    if(fncode == 1){
        *(gpio+((line)/10)) &= ~(7<<(((line)%10)*3));
    }
    else{
        *(gpio+((line)/10)) |= (1<<(((line)%10)*3));
    }
}

/*function to set the latch pin*/
void write_latch(unsigned int* gpio){
    gpio[7] = 1<<LAT;
}

/*function to clear the latch*/
void clear_latch(unsigned int* gpio){
    gpio[10] = 1<<LAT;
}

/*function to set the clock*/
void write_clock(unsigned int* gpio){
    gpio[7] = 1<<CLK;
}

/*function to clear the clock*/
void clear_clock(unsigned int* gpio){
    gpio[10] = 1<<CLK;
}

/*function to read the value of Data line
 * Returns 0 if button is pressed, else returns 1*/
int read_data(unsigned int* gpio){
    if(((gpio[13])&(1<<DAT))==0) return 0;
    return 1;
}

/*main SNES subroutine that reads input(buttonn pressed) from a SNES controller.
 * gpio base address, array to keep track of buttons pressed previously, buttons pressed in this iteration, and a binary flag are passed as arguments
 * Updates the buttons array with button currently pressed and the previous_state of the button*/
 
void read_snes(unsigned int *gpio, int *previous_state, int* buttons, int* change){

    // Set Latch to output 
    init_GPIO(LAT, 1, gpio);
    init_GPIO(LAT, 0, gpio);

    // Set clock to output 
    init_GPIO(CLK, 1, gpio);
    init_GPIO(CLK, 0, gpio);
    
    // Set Data to input 
    init_GPIO(DAT, 1, gpio);

    //write_clock(gpio);
    write_latch(gpio);
    delayMicroseconds(12);
    clear_latch(gpio);
    
    for(int i=1; i<=16; i++){

        delayMicroseconds(6);
        clear_clock(gpio);
        delayMicroseconds(6);
        
        int pressed = read_data(gpio);                                  // pressed is either 0 or 1 depending on whether a button is pressed
        
        if(pressed==0 && previous_state[i]==0){                         // if a button is pressed and it was previously pressed, update the previous state to not pressed and buttons value to not pressed
            previous_state[i] = 1;
            buttons[i] = 1;
            *change = 0;                                                // flag not set   
           
        }

        else if(pressed==0 && previous_state[i]==1){                    // else button is pressed and was not pressed previously
            previous_state[i]=0;                                        // set previous_state value to 0 i.e pressed
            buttons[i] = 0;                                             // buttons value set i.e button currently pressed
            *change = 1;                                                // flag set
        }
	write_clock(gpio);
        
       
    }    
}

/*prints the appropriate button*/

void print_message(int* buttons, int* start, int* previous_state, int* change){
    for(int j=0; j<=12; j++){
	if(buttons[j]==0){                                                  // if a button pressed
	    switch(j){                                                      // depending upon which iteration it is, display the button
	    case 1:
            printf("You pressed B\n");
            *change = 0;
            previous_state[1] = 0;
            break;
	    case 2:
            printf("You pressed Y\n");
            *change = 0;
            previous_state[2] = 0;
            break;
	    case 3:
            printf("You pressed Select\n");
            *change = 0;
            previous_state[3] = 0;
            break;
	    case 4:
            printf("You pressed Start\nExiting the program now\n");
            *change = 0;
            previous_state[4] = 0;
            *start = 0;
            break;
	    case 5:
            printf("You pressed Joy-pad UP\n");
            *change = 0;
            previous_state[5] = 0; 
	    y_-=10;
	    drawPicture();
            break;
	    case 6:
            printf("You pressed Joy-pad DOWN\n");
            *change = 0;
            previous_state[6] = 0;
	    y_+=10;
	    drawPicture();
            break;
	    case 7:
            printf("You pressed Joy-pad LEFT\n");
            *change = 0;
            previous_state[7] = 0;
	    x_-=10;
	    drawPicture();
            break;
	    case 8:
            printf("You pressed Joy-pad RIGHT\n");
            *change = 0;
            previous_state[8] = 0;
	    x_+=10;
	    drawPicture();
            break;
	    case 9:
            printf("You pressed A\n");
            *change = 0;
            previous_state[9] = 0;
            break;
	    case 10:
            printf("You pressed X\n");
            *change = 0;
            previous_state[10] = 0;
            break;
	    case 11:
            printf("You pressed Left\n");
            *change = 0;
            previous_state[11] = 0;
            break;
	    case 12:
            printf("You pressed Right\n");
            *change = 0;
            previous_state[12] = 0;
            break;              
	    }
	}
        
    }
    if(*change==0) printf("\nPlease press a button\n");
    
    for(int i=1; i<=12;i++){                                            // set all buttons to not pressed again
        buttons[i] = 1;
    } 
    
    delayMicroseconds(140000);
}

void* runCar(void* param){
    for(int i=1350; i>895; i-=10){
        sleep(1);
        carX = i;
    }
    pthread_exit(0);
}

int main(){

    unsigned int *gpio = getGPIOPtr();

    int start = 1;                                                      // start is a flag to determine if start button is pressed
    int previous_state[13];                                             // previous_state button to keep track of previous state of the buttons
    int buttons[13];                                                    // buttons array to keep track of which buttons are currently pressed
    printf("Created by: Priyavart Rajain and Taimur Rizwan\nPlease press a button..\n");

    for(int i=1; i<=12;i++){                                            // initialize the arrays
        previous_state[i] = 0;
    }
    for(int i=1; i<=12;i++){
        buttons[i] = 1;
    }
    int change = 0;                                                     // flag to keep track of whether a button is pressed. if 0 , flag off, else flag set
    
    pthread_t pinkCar;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int carId = 0;
    pthread_create(&pinkCar, &attr, runCar, &carId);

    drawPicture();
    while(start){
        
	read_snes(gpio, previous_state, buttons, &change);                       // read input from SNES controller
        if(change==1){print_message(buttons, &start, previous_state, &change);}  // if flag set, print appropriate message
	
    }
    return 0;
}
