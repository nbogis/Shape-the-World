// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// December 29, 2014

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
// NDU: check the intialization later
void Ports_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000032;     // 1) activate clock for Ports F,E,B
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
	
	// port F initialization for pedestrians lights
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x0A;           // allow changes to PF1 and PF3
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0A;          // 5) PF1, PF3 as output (walk and don't walk)
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x00;          // enable pull-up on PF1 and PF3
  GPIO_PORTF_DEN_R = 0x0A;          // 7) enable digital I/O on PF1, PF3
	
	// port E initilization for sensors
//GPIO_PORTE_CR_R = 0x03;           // allow changes to PE0,PE1,PE2
	GPIO_PORTE_AMSEL_R = 0x00;	// 2) disable analog function. 
	GPIO_PORTE_PCTL_R	= 0x00000000;	// 3) regular digitl function
	GPIO_PORTE_DIR_R = ~0X07; 	// 4) set PE0,PE1,PE2 as inputs
	GPIO_PORTE_AFSEL_R = 0x00; 	// 5) clear bits in alternate function
	GPIO_PORTE_DEN_R = 0x07;	// 6) enable digital pins PE0, PE1, PE2
	
	// port B initilization for cars traffic lights
	//GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB0-PB5
	GPIO_PORTB_AMSEL_R = 0x00;	// 2) disable analog function. 
	GPIO_PORTB_PCTL_R	= 0x00000000;	// 3) regular digitl function
	GPIO_PORTB_DIR_R = 0X3F; 	// 4) set PB0-PB5 as outputs
	GPIO_PORTB_AFSEL_R = 0x00; 	// 5) clear bits in alternate function
	GPIO_PORTB_DEN_R = 0x3F;	// 6) enable digital pins PB0-PB5
}

// Subroutine to add a delay
// Inputs: number of 1ms delays
// Output: None
// Notes: assumes 80 MHz clock
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
// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){           
  NVIC_ST_CTRL_R = 0x00000005;          // enable SysTick with core clock
}

struct state{
		unsigned long Out;	// 6-LEDs output
		unsigned long walk_Out;	// pedestrians traffic lights
		unsigned long Time;	// time between states
		unsigned long Next[16];	// next states
	};
	typedef struct state SType;
	
	#define SG 0x21
	#define SY 0x22
	#define SR 0x24
	#define WG 0x0C
	#define WY 0x14
	#define WR 0x24
	#define SWR 0x24
	#define Walk 0x08
	#define Dont_Walk 0x02
	#define Stop_Walk 0x00
	
	const SType FSM[11]={
		{WG, Dont_Walk, 500, {0,1,1,1,1,1,1,1}},	//GoW
		{WY, Dont_Walk, 500, {2,2,2,2,2,2,2,2}},	//ReadyW
		{WR, Dont_Walk, 500, {2,0,3,3,6,6,3,3}},	//StopW
		{SG, Dont_Walk, 500, {3,4,4,4,4,4,4,4}},	//GoS
		{SY, Dont_Walk, 500, {5,5,5,5,5,5,5,5}},	//ReadyS
		{SR, Dont_Walk, 500, {5,0,3,0,6,0,6,6}},	//StopS
		{SWR, Walk, 500, {6,7,7,7,7,7,7,7}},	//Walk
		{SWR ,Dont_Walk, 500, {8,8,8,8,8,8,8,8}},	//Don't_Walk 1
		{SWR, Stop_Walk, 500, {9,9,9,9,9,9,9,9}},	//Stop_Walk 1
		{SWR ,Dont_Walk, 500, {10,10,10,10,10,10,10,10}},	//Don't_Walk 2
		{SWR ,Stop_Walk, 500, {10,0,3,0,6,0,3,0}}	//Stop_Walk 2
	};
	unsigned char cState;	//current state (0 to 10)
int main(void){ unsigned char In; // inputs from sensors
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz

  EnableInterrupts();
  Ports_Init();   // initialize ports F,E,B 
	SysTick_Init();
	cState=0;	// start with WG state
  while(1){
    // output to the W and S lights
		GPIO_PORTB_DATA_R = (FSM[cState].Out & 0x3F);	//only 0x---- --xx
		// output to pedestrians light
		GPIO_PORTF_DATA_R = (FSM[cState].walk_Out & 0x0A);  
		// wait for 0.5 sec between each transition
		Delay1ms(FSM[cState].Time);
		In = GPIO_PORTE_DATA_R & 0x07;
		// define the next state
		cState = FSM[cState].Next[In];
			
  }
}

