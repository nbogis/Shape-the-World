// Lab13.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// edX Lab 13 
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Port B bits 3-0 have the 4-bit DAC
// Port E bits 3-0 have 4 piano keys

#include "..//tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "DAC.h"
#include "TExaS.h"

// global vaiable declaration
unsigned char Note;
// RELOAD values with sepect to key number
const unsigned long f_note[16] = {0,4778,4257,0,3792,0,0,0,3189,0,0,0,0,0,0,0};
//const unsigned long f_note[16] = {0,9560,8517,0,7587,0,0,0,6385,0,0,0,0,0,0,0};		// with 16 table size

unsigned long n;
// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void delay(unsigned long msec);
int main(void){ // Real Lab13 
	// for the real board grader to work 
	// you must connect PD3 to your DAC output
	
   TExaS_Init(SW_PIN_PE3210, DAC_PIN_PB3210,ScopeOn); // activate grader and set system clock to 80 MHz
// PortE used for piano keys, PortB used for DAC        

	Sound_Init(); // initialize SysTick timer and DAC
  Piano_Init();
  EnableInterrupts();  // enable after all initialization are done
  while(1){                
		// read input key from Piano
		Note = Piano_In();					// get input from the keys
		Sound_Tone(f_note[Note]);		// specifiy the frequency depending on the tone
		delay(1);
  }
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles and 5 cycles in simulation
    msec--;
  }
}


