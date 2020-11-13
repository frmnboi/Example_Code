//import the driver library and the prototype and define statements
#include "driverlib.h"
#include <stdio.h>

//controlling a unipolar stepper motor
//lab 7

//the current motor phase it is in.
volatile uint8_t energizeseq = 0;

//direction variable.  1 is forward, 2 is reverse, 0 is neutral
volatile uint8_t direction = 0;


#define timer1_period 4688 // /12
// /12 is the limit at which the servo can behave normally
//20Hz *12=240Hz


//3E+6/32 = 2*46875 Hz --> use 4687.5 as the up timer to count at 20 Hz -->4688 approximately
const Timer_A_UpModeConfig upConfig_0 =                     // Up Mode
        { TIMER_A_CLOCKSOURCE_SMCLK,                // Timer A is based on SMCLK
          TIMER_A_CLOCKSOURCE_DIVIDER_32, // Slow down the counter 64 times over
          timer1_period,                 // Period of Timer A, stored in TAxCCR0
          TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable interrupts when Timer A rolls over
          TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable interrupt for capture-compare
          TIMER_A_DO_CLEAR                   // Clears counter when initialized
        };


//how do you change timer A to reinitialize it?


int main(void)
{
    //Taken from template:

    // Halt watchdog timer
    MAP_WDT_A_holdTimer();

    MAP_CS_setDCOFrequency(3E+6); // Set DCO clock source frequency
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); // Tie SMCLK to DCO

    // Configure Red LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //Configure the 3 color LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    // Configure Switches
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);

    //set pwm pins as outputs
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN6);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);
    //probably also need to set pwm (primary? function)

    //2.4 energizes A1 state 0
    //2.5 energizes B1 state 1
    //2.6 energizes B2 state 3
    //2.7 energizes A2 state 2

    Interrupt_disableMaster();

    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    //Enable and configure the interrupt for up mode
    Timer_A_enableInterrupt(TIMER_A0_BASE);
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);
    //this timer A interrupt is consistently changing the values even when debug is running, an

    //Counts in up mode to the equivalent time for 0.05 second
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Interrupt_enableMaster();

    while (1)
    {
//        Interrupt_disableMaster();
        //if one switch is pressed go forward (takes priority)
        if (GPIO_INPUT_PIN_LOW
                == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1))
        {
            direction = 1;
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
        }
        else
        {    //if the other switched is pressed, go backwards
            if (GPIO_INPUT_PIN_LOW
                    == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4))
            {
                direction = 2;
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);

            }
            else
            {
                //if no switch is pressed, stop the motor.
                direction = 0;

                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

            }

        }

//        Interrupt_enableMaster();

    }

}
//3 modes, one button pressed, left, no pressed, not moving, other button pressed, right

//2.4 energizes A1 state 0
//2.5 energizes B1 state 1
//2.6 energizes B2 state 3
//2.7 energizes A2 state 2

void outputseq()
{
    //I confirmed the uart transmission is significantly slowing down the transmission.  constantly sending uart can cause an infinite loop, which is probably what happened in the uart lab.  It couldn't send as fast as the interrupt calls came in
    //printf("interrupt was called\n");
    //printf("%d \n", energizeseq);
    if (direction)
    {
        if (direction == 1)
        {
        switch (energizeseq)
        {
            case 0:
            //the motor is currently at A1 and needs to go to B1
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
            //not strictly neccisary
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
            energizeseq=1;
            break;
            case 1:
            //the motor is currently at B1 and needs to go to A2
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
            //not strictly neccisary
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
            energizeseq=2;
            break;

            case 2:
            //the motor is currently at A2 and needs to go to B2
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
            //not strictly neccisary
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
            energizeseq=3;
            break;
            case 3:
            //the motor is currently at B2 and needs to go to A1
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
            //not strictly neccisary
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
            energizeseq=0;
            break;
        }
    }
    else
    {
    switch (energizeseq)
    {
        case 0:
        //the motor is currently at A1 and needs to go to B2
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
        //not strictly neccisary
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
        energizeseq=3;
        break;
        case 3:
        //the motor is currently at B2 and needs to go to A2
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
        //not strictly neccisary
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
        energizeseq=2;
        break;
        case 2:
        //the motor is currently at A2 and needs to go to B1
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
        //not strictly neccisary
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
        energizeseq=1;
        break;
        case 1:
        //the motor is currently at B1 and needs to go to A1
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
        //not strictly neccisary
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
        energizeseq=0;
        break;
    }
}

}
else
{
MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
}

}

void TA0_0_IRQHandler()
{
    //clear interrupt flag
Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
                                     TIMER_A_CAPTURECOMPARE_REGISTER_0);
//change the digital output
outputseq();

}

