// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0 SS3 to be triggered by
// software and trigger a conversion, wait for it to finish,
// and return the result. 
// Daniel Valvano
// December 28, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
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

#include "ADC.h"
#include "..//tm4c123gh6pm.h"

// This initialization function sets up the ADC 
// Max sample rate: <=125,000 samples/second
// SS3 triggering event: software trigger
// SS3 1st sample source:  channel 1
// SS3 interrupts: enabled but not promoted to controller
void ADC0_Init(void){ 
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;			// activate clock on port E
	delay = SYSCTL_RCGC2_R;	
	GPIO_PORTE_DIR_R = ~0x04;					// make PE2 input (channel 1)
	GPIO_PORTE_AMSEL_R |= 0x04; 			// enable analog function on PE2
	GPIO_PORTE_AFSEL_R |= 0x04;				// enable alternate function on PE2
	GPIO_PORTE_DEN_R &= ~0x04;				// disable digital I/O on PE2
	
	// initiate device driver. Initialize ADC0, channel 1, sequencer 3
	SYSCTL_RCGC0_R |= 0x00010000;			// activiate ADC0
	delay = SYSCTL_RCGC0_R;
	SYSCTL_RCGC0_R &= ~0x00000300;		// configure for 125k
	ADC0_SSPRI_R = 0x0123;						// make sequencer 3 the highest proprity
	ADC0_ACTSS_R |= ~0x0008;					// disable seq 3
	ADC0_EMUX_R &= 0x0FFF;						// set bit 15-12 for software trigger (seq3)
	ADC0_SSMUX3_R &= ~0x000F;					// clear SS3 field
	ADC0_SSMUX3_R += 1;								// set channel 1 (Ain1 = PE2)
	ADC0_SSCTL3_R = 0x0006;						// TS0 = 0 measure analog voltage on PE2 (ADC analog input pin)
																		// IE0 = 1 to set INR3 flag in ADC0_RIS_R when ADC conversion completes
																		// END0 = 1 this sample is the end of the sequence
																		// D0 = 0 sample single-ended analog input
	ADC0_ACTSS_R |= 0x0008;						// enable seq 3  
}


//------------ADC0_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long ADC0_In(void){  
	unsigned long result;
	ADC0_PSSI_R = 0x0008;							// initiate SS3
	while ((ADC0_RIS_R&0x08)==0){};
	result = ADC0_SSFIFO3_R&0xFFF;		// read result
	ADC0_ISC_R = 0x0008;							// clear RIS by clearing bit 3 in ISC	
  return result; 												// return result
}
