//import the driver library and the prototype and define statements
#include "driverlib.h"
#include <stdio.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define timer1_period 4688
//this value equals 1 second with 64 divider
//using a divider of 8, we cut the update time down to 1/8 of a second, which is close to the refresh rate of the sensors hopefully.

const Timer_A_UpModeConfig upConfig_0 =                     // Up Mode
		{ TIMER_A_CLOCKSOURCE_SMCLK,                // Timer A is based on SMCLK
				TIMER_A_CLOCKSOURCE_DIVIDER_8, // Slow down the counter 64 times over
				timer1_period,           // Period of Timer A, stored in TAxCCR0
				TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable interrupts when Timer A rolls over
				TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable interrupt for capture-compare
				TIMER_A_DO_CLEAR             // Clears counter when initialized
		};

volatile int Idisterr = 0;
volatile int Itwisterr = 0;

volatile int lastDdist = 0;
volatile int lastDtwist = 0;

volatile long encodercount1 = 0;

//the position of the car will need to be updated by sensors (not included in this code)
volatile int x = 0;
volatile int y = 0;
volatile int heading = 0;

//the goal position
const int goalx = 50;
const int goaly = 50;

//the cutoff distance for the PID controller to stop it from overshooting
#define distancecutoff 4

int main(void)
{
	// Halt watchdog timer
	MAP_WDT_A_holdTimer();

	//2.5V logic level is default

	Interrupt_disableMaster();

//	FPU_enableModule();
//	FPU_disableStacking() ;
//	FPU_enableLazyStacking() ;

	MAP_CS_setDCOFrequency(3E+6); // Set DCO clock source frequency
	MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); // Tie SMCLK to DCO

	//set pwm pins as outputs
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN6);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);

	//setting primary pwm functionality on pins
	P2SEL0 |= 0xF0; // Set bits  4-7 of P2SEL0 to enable TA0.1-4 functionality on pins 2.4-2.7
	P2SEL1 &= ~0xF0; // Clear bit 4-7 of P2SEL1 to enable TA0.1-4 functionality on pins 2.4-2.7
#define pwmperiod	1024
	// Set Timer A0 period (PWM signal period).  Shared for all timer A0 instances
	TA0CCR0 = pwmperiod;
	// Set Duty cycle for all pwm items to 0
	//*****************not sure if this is correct, since
	TA0CCR1 = 0;
	TA0CCR2 = 0;
	TA0CCR3 = 0;
	TA0CCR4 = 0;

	// Set output mode to Reset/Set
	TA0CCTL1 = OUTMOD_7;
	TA0CCTL2 = OUTMOD_7;
	TA0CCTL3 = OUTMOD_7;
	TA0CCTL4 = OUTMOD_7;

	// Initialize Timer A
	TA0CTL = TASSEL__SMCLK | MC__UP | TACLR; // Tie Timer A to SMCLK, use Up mode, and clear TA0R

	//*********************Not really neccisary for this project, as all I need to do is simply control the relative pwm control to each motor************

	//4 gpio to read from rotary encoders
	//try without special chip at first
	MAP_GPIO_setAsInputPin(GPIO_PORT_P6, GPIO_PIN7);
	MAP_GPIO_setAsInputPin(GPIO_PORT_P4, GPIO_PIN1);
	MAP_GPIO_setAsInputPin(GPIO_PORT_P4, GPIO_PIN6);
	MAP_GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN3);

	//enable the edge interrupt
	GPIO_enableInterrupt(GPIO_PORT_P6, PIO_PIN7);
	GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN3);
	GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN1 | GPIO_PIN6);
//
	//Readings from rotary encoders
	//set interrupts to start when there is a low to high or high to low
	GPIO_interruptEdgeSelect(GPIO_PORT_P6, PIO_PIN7,
			GPIO_LOW_TO_HIGH_TRANSITION);
	GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN3,
			GPIO_LOW_TO_HIGH_TRANSITION);
	GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN1 | GPIO_PIN6,
			GPIO_LOW_TO_HIGH_TRANSITION);

	//clear the interrupt flag and the values in the intput register to prevent false positives on start
	P2IN = 0x00;
	P2IFG = 0x00;
	P4IN = 0x00;
	P4IFG = 0x00;
	P6IN = 0x00;
	P6IFG = 0x00;

	//note that this is required for interrupt to work.  the nvic needs to have interrupts enabled
	NVIC_EnableIRQ(PORT2_IRQn);
	NVIC_EnableIRQ(PORT4_IRQn);
	NVIC_EnableIRQ(PORT6_IRQn);

	//enable interrupts
	P2IE = 0b00001000;
	P4IE = 0b01000010;
	P6IE = 0b10000000;

	GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN3,
			GPIO_LOW_TO_HIGH_TRANSITION | GPIO_HIGH_TO_LOW_TRANSITION);

	GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN3,
			GPIO_LOW_TO_HIGH_TRANSITION | GPIO_HIGH_TO_LOW_TRANSITION);

	GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN3,
			GPIO_LOW_TO_HIGH_TRANSITION | GPIO_HIGH_TO_LOW_TRANSITION);

	//Interrupt_setPriority can set the priority of the interrupt.

	//enable the Timer A interrupt for refreshing the device

	Timer_A_clearInterruptFlag(TIMER_A2_BASE);
	//Enable and configure the interrupt for up mode
	Timer_A_enableInterrupt(TIMER_A2_BASE);
	Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig_0);
	Interrupt_enableInterrupt(INT_TA2_0);

	//Counts in up mode to the equivalent time for 0.05 second
	Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

	Interrupt_enableMaster();

	while (1)
	{
//		MAP_PCM_gotoLPM0();
	}

//interrupts for the data sensors
	void PORT2_IRQHandler()
	{
		if (P5IFG & 0b00000100)
		{
//pin 3 is activated

			//remove interrupt flag
			P5IFG &= ~0b00000100;

			//Add a count to the registered encoder values
			encodercount++;

		}

//Port 4 and 6 will have similar code for the other 3 encoder readings.

		void TA2_0_IRQHandler()
		{
			//clear interrupt flag
			Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
					TIMER_A_CAPTURECOMPARE_REGISTER_0);

			//the PID controller is contained and updated at this frequency.

			if ((float) pow(pow(x - goalx, 2) + pow(y - goaly, 2), 0.5)
					<= cutoffdist)
			{
				//calculate the errors
				int disterr = pow(pow(x - goalx, 2) + pow(y - goaly, 2), 0.5);
				int angleerr = heading - arctan2(x - goalx, y - goaly);

#define Pdist 0
#define Ptwist 0
#define Idist 0
#define Itwist 0
#define Ddist 0
#define Dtwist 0

				int distpart = Pdist * disterr + Ddist * (disterr - lastDdist)
						+ Idist * Idisterr;
				int twistpart = Ptwist * twisterr
						+ Dtwist * (twisterr - lastDtwist) + Itwist * Itwisterr;

				//the greater turn in this case is always 1 (maxed out)
				int lesserturn = pwmperiod * (distpart - twistpart)
						/ (distpart + twistpart)

				//motors 1 and 2 are on left, 3 and 4 are on right

				if (twistpart > 0)
				{
					TA0CCR1 = (int) pwmperiod;
					TA0CCR2 = (int) pwmperiod;
					TA0CCR3 = (int) lesserturn;
					TA0CCR4 = (int) lesserturn;
				}
				else
				{
					TA0CCR3 = (int) pwmperiod;
					TA0CCR4 = (int) pwmperiod;
					TA0CCR1 = (int) lesserturn;
					TA0CCR2 = (int) lesserturn;

				}
			}
			else
			{
				TA0CCR3 = 0;
				TA0CCR4 = 0;
				TA0CCR1 = 0;
				TA0CCR2 = 0;
			}

		}

