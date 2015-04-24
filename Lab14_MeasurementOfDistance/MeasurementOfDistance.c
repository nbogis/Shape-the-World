// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
// December 29, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014

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

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE2(Ain1) and PD3
// Slide pot pin 1 connected to ground


#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"

void EnableInterrupts(void);  // Enable interrupts
void Delay1ms(unsigned long ms);		

unsigned char String[10]; // null-terminated ASCII string
unsigned long Distance;   // units 0.001 cm
unsigned long ADCdata;    // 12-bit 0 to 4095 sample
unsigned long Flag;       // 1 means valid Distance, 0 means Distance is empty

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
	unsigned long distance;
	distance = (((unsigned long)(sample * 1700.3))>>10)+ 250;		// calibration was very tough with the pot.
//	I would work on whatever value I have and develop the argument accordingly  
	//distance = (((unsigned long)(545*sample))>>10) + 583;		  // we calibrate a and found it to be 0.5-0.6
 // distance = (((unsigned long)(2345*sample))>>10) + 383;		// we calibrate a and found it to be 0.5-0.6
	return distance; 
}

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
	// SysTick initialization
	NVIC_ST_CTRL_R = 0;						// disable SysTick for set up
	NVIC_ST_RELOAD_R = period - 1;// set the RELOAD value
	NVIC_ST_CURRENT_R = 0;				// 	any write to current clear it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFF)|0x40000000;
	NVIC_ST_CTRL_R = 0x07;				// enable SysTick with core clock and interrupt
}
// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
	GPIO_PORTF_DATA_R ^= 0x02;			// toggle PF1
	GPIO_PORTF_DATA_R ^= 0x02;			// toggle PF1 again
	ADCdata =ADC0_In();							// read ADC data from ADC0
	Distance = Convert(ADCdata);
	ADC0_ISC_R = 0x0004; 					// set flag in EMUX indicating new data is ready
	GPIO_PORTF_DATA_R ^= 0x02;			// toggle PF1 again
}

//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n){
// as part of Lab 11 you implemented this function
		if (n < 10){
		String[0] = 0x30;
		String[1] = '.';
		String[2] = 0x30;
		String[3] = 0x30;
		String[4] = n + 0x30;
	}
	else if (n >= 10 && n < 100){
		String[0] = 0x30;
		String[1] = '.';
		String[2] = 0x30;
		String[3] = n/10 + 0x30;
		n = n % 10;
		String[4] = n + 0x30;
	}		
	else if (n >=100 && n < 1000){
		String[0] = 0x30;
		String[1] = '.';
		String[2] = n/100 + 0x30;
		n= n % 100;
		String[3] = n/10 + 0x30;
		n = n % 10;
		String[4] = n + 0x30;
	}
	else if ( n >=1000 && n < 10000){
		String[0] = n/1000 + 0x30; // get the thousands digit
		n = n % 1000;	// to move to the next digit wihtout missing the number
		String[1] = '.';
		String[2] = n/100 + 0x30;	// get to the hundred digit
		n= n % 100;
		String[3] = n/10 + 0x30;	// get to the tens digit
		n = n % 10;
		String[4] = n + 0x30;	// get the ones digit
	}
	else {
		String[0] = String[2] = String[3] = String[4] = '*';
		String[1] = '.';
	}
	String[5] = 0x20; // put a space
	String[6] = 'c';
	String[7] = 'm';
	String[8] = 0x00;  // put a null terminator
}

// main1 is a simple main program allowing you to debug the ADC interface
//int main(void){ 
//  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
//  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
//  EnableInterrupts();
//  while(1){ 
//    ADCdata = ADC0_In();				// software start, read ADC, return 12-bit reault
//  }
//}
// once the ADC is operational, you can use main2 to debug the convert to distance
//int main(void){ 
//  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_NoScope);
//  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
//  Nokia5110_Init();             // initialize Nokia5110 LCD
//  EnableInterrupts();
//  while(1){ 
//    ADCdata = ADC0_In();
//    Nokia5110_SetCursor(0, 0);
//    Distance = Convert(ADCdata);
//    UART_ConvertDistance(Distance); // from Lab 11
//    Nokia5110_OutString(String);    // output to Nokia5110 LCD (optional)
//  }
//}
// once the ADC and convert to distance functions are operational,
// you should use this main to build the final solution with interrupts and mailbox
//int main(void){ 
//  volatile unsigned long delay;
//  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
//// initialize ADC0, channel 1, sequencer 3
//	//ADC0_Init();
//// initialize Nokia5110 LCD (optional)
//// initialize SysTick for 40 Hz interrupts
//// initialize profiling on PF1 (optional)
//                                    //    wait for clock to stabilize

//  EnableInterrupts();
//// print a welcome message  (optional)
//  while(1){ 
//// read mailbox
//// output to Nokia5110 LCD (optional)
//  }
//}

void Delay1ms(unsigned long ms){ 
	unsigned long delay;
	while (ms>0){
		delay = 16000;
		while (delay > 0){
			delay -= 1;
		}
		ms -= 1;
	}
}
int main(void){
	TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_NoScope);
  ADC0_Init();    							// initialize ADC0, channel 1, sequencer 3
  Nokia5110_Init();             // initialize Nokia5110 LCD
	SysTick_Init(2000000);				// initialize SysTick timer, every 0.25 ms

	// port F initialization for SW1
	SYSCTL_RCGC2_R |= 0x00000020;	// activate clock on port F
	GPIO_PORTF_AMSEL_R &= ~0x02;	// disable analog function on PF1
	GPIO_PORTF_PCTL_R &= ~0x02;		// clear PCTL bit for regular digital function
	GPIO_PORTF_DIR_R |= 0x02;			// make PF1 output
	GPIO_PORTF_AFSEL_R &= ~0x02;	// disble alterante function
	GPIO_PORTF_DEN_R |= 0x02;			// enable digital I/O on PF1
	EnableInterrupts();
	
	while(1){
		ADC0_ISC_R = ~0x0004;				// clear flag
		Delay1ms(1000);								// wait for the flag to be cleared
		UART_ConvertDistance(Distance);// convert the distance to display
		Nokia5110_Clear();
		Nokia5110_SetCursor(0, 0);	// set cursor to beginning
		Nokia5110_OutString(String);// output the string Distance
	}
}
