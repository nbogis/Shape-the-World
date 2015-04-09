// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// December 29, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

// global variables
unsigned char switch_previous = 0; 	// a global variable to store the switch previous state
unsigned long count = 0;   // a global variable count number of pressing the switch

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	// configuring the pins
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x0000001;      	// 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;          		// allow time for clock to start
  GPIO_PORTA_AMSEL_R &= ~0x0C;      	// 3) disable analog on PF3,2
  GPIO_PORTA_PCTL_R &= ~0x0000FF00;   // 4) configure PA3,2 as GPIO. clear bits in PCTL to select regular digital function
  GPIO_PORTA_DIR_R |= 0x04;          	// 5) configure PA3 as input and PA2 as output
	GPIO_PORTA_AFSEL_R &= ~0x0C;       	// 6) disable alt funct on PA3,2
  GPIO_PORTA_DEN_R |= 0x0C;          	// 7) enable digital I/O on PA3,2
	
	// SysTick interrupt configuration
	NVIC_ST_CTRL_R = 0; 								// disable SysTick during initialization
	NVIC_ST_RELOAD_R = 90908; 					// reload value for 440 Hz (1/880 interrupt)
	NVIC_ST_CURRENT_R = 0; 							// write to current to clear it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R& 0x00FFFFFF;	// set priority 0
	NVIC_ST_CTRL_R = 0x00000007;				// enable clock and interrupt 
}

// called at 880 Hz
void SysTick_Handler(void){
	if ((GPIO_PORTA_DATA_R&0x08) && (switch_previous == 0)){	// if switch is on - detecting risign edge
		count ++;	// count: 1, 3 - the state to toggle PA2 and 
							//				2, 4 - the state to turn off PA2
	 switch_previous = 1;
	}
  else if(!(GPIO_PORTA_DATA_R&0x08) && (switch_previous == 1)){		// if switch is off - detecting falling edge	
		switch_previous = 0;
	}
	if ((count == 1) || (count == 3)){
		GPIO_PORTA_DATA_R ^= 0x04;		// toggle PA2		
	}
	else{	// if count is 2 or 4
		GPIO_PORTA_DATA_R = 0x00;		// turn off PA2	
		if (count == 4){		// restart count
			count =0;
		}
	}	
}


int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();         
  EnableInterrupts();   // enable after all initialization are done
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
  }
}
