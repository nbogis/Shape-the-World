// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// December 28, 2014
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PORTE_Init(void);	// initilize port E pins
void toggle_LED(void);
void Delayms(unsigned long ms);
void Delay1ms(unsigned long msec);

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ unsigned long volatile Input;
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1);  // activate grader and set system clock to 80 MHz
  PORTE_Init();
	
  EnableInterrupts();           // enable interrupts for the grader
  
	GPIO_PORTE_DATA_R |= 0x02; // turn LED on
	while(1){
		Delayms(100);	// delay of 100ms
		Input = GPIO_PORTE_DATA_R & 0x01;
		if (Input == 0x01) {	// if switch (PE1) is pressed
			toggle_LED();	// toggle LED once
		}
		else{
			GPIO_PORTE_DATA_R = 0x02;
		}
  }
}

// Subroutine to initialize port E 
// PE0 is input and PE1 is output
// Inputs: None
// Output: None
// Notes: ...
void PORTE_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;	// 1) enable port E clock
	delay = SYSCTL_RCGC2_R; 	// delay
	GPIO_PORTE_AMSEL_R = 0x00;	// 2) disable analog function. 
	GPIO_PORTE_PCTL_R	= 0x00000000;	// 3) regular digitl function
	GPIO_PORTE_DIR_R = 0X02; 	// 4) set PE0 as input and PE1 as output
	GPIO_PORTE_AFSEL_R = 0x00; 	// 5) clear bits in alternate function
	GPIO_PORTE_DEN_R = 0x03;	// 6) enable digital pins PE0 and PE1
}


// Subroutine to toggle LED (PE1) with a delay of 100 ms
// Inputs: None
// Output: None
// Notes: ...
void toggle_LED(void){
	GPIO_PORTE_DATA_R ^= 0x02;	// toggle PE1
}

// Subroutine to add a delay
// Inputs: number of 1ms delays
// Output: None
// Notes: assumes 80 MHz clock
void Delayms(unsigned long ms){
	unsigned long delay;
	while (ms>0){
		delay = 16000;
		while (delay > 0){
			delay -= 1;
		}
		ms -= 1;
	}
}
