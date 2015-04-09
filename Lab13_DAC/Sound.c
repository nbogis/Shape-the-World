// Sound.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// This routine calls the 4-bit DAC

#include "Sound.h"
#include "DAC.h"
#include "Piano.h"
#include "..//tm4c123gh6pm.h"

// global variables initialization
unsigned char Index = 0;
const unsigned char SineWave[32]={8,9,11,12,13,14,14,15,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4,5,7};
//const unsigned char SineWave[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3};
	
// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Also calls DAC_Init() to initialize DAC
// Input: none
// Output: none
void Sound_Init(void){
  DAC_Init();
	NVIC_ST_CTRL_R = 0;						// disable SysTick during setup
	NVIC_ST_RELOAD_R = 49999;		// set interrupt to 1.6 kHz
	NVIC_ST_CURRENT_R = 0;				// any write to current clear it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000;				// set priority 1
	NVIC_ST_CTRL_R = 0x0007;			// enable clock and interrupt
}

// **************Sound_Tone*********************
// Change Systick periodic interrupts to start sound output
// Input: interrupt period
//           Units of period are 12.5ns
//           Maximum is 2^24-1
//           Minimum is determined by length of ISR
// Output: none
void Sound_Tone(unsigned long period){
// this routine sets the RELOAD and starts SysTick
	NVIC_ST_CTRL_R = 0;						// disable SysTick during setup
	NVIC_ST_RELOAD_R = period-1;  // set interrupt depending on the tone
	NVIC_ST_CURRENT_R = 0;				// any write to current clear it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000;				// set priority 1
	NVIC_ST_CTRL_R = 0x0007;			// enable clock and interrupt
}


// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
	Index = (Index+1)&0x1F;			// index to cycle over sin wave (0x1F would cycle over the 32 table)
	DAC_Out(SineWave[Index]);		// output the sine wave value to DAC
}
