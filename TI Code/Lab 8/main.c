//import the driver library and the prototype and define statements
#include "driverlib.h"
#include <stdio.h>

//potentiometer is 0 to 9.14 kohm
//resistor is 10K
//min voltage is approx 0
//max voltage is approx 1/2 of 5V =2.5V

uint8_t adc = 0;
int toggle1=0;
int toggle2=0;
int delay=0;


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

#define period 255   //this is to have 1-1 correspondance with the 8 bit ADC input

    //uses timer A0 and 2 different capture compare pins

    //these together set these pins to primary module function, which apparently is timer A0
    P2SEL0 |= 0x30; // Set bit 4 and 5 of  	P2SEL0 to enable TA0.1 functionality on P2.4 and P2.5
    P2SEL1 &= ~0x30; // Clear bit 4 and 5 of P2SEL1 to enable TA0.1 and TA0.2 functionality on P2.4 and P2.5 respectively
    P2DIR |= 0x30;     // Set pin 2.4 and 2.5 as an output pins

    // Set Timer A0 period (PWM signal period).  Shared for all timer A0 instances
    TA0CCR0 = period;

    // Set Duty cycle
    TA0CCR1 = 0;

    // Set output mode to Reset/Set
    TA0CCTL1 = OUTMOD_7; // Macro which is equal to 0x00e0, defined in msp432p401r.h
    //set/reset also works as well.  which is mode 1

    // Initialize Timer A
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR; // Tie Timer A to SMCLK, use Up mode, and clear TA0R

    // PWM signal will now be available on P2.4

//how do I go to timer A0 capture compare register 2

    //now the pwm for the reverse direction
    TA0CCTL2 = OUTMOD_7;

//this should be correct.  According to page 785, it says that when the TAxCCR0 is nonzero, the timer will start when there is a mode set


    //Set pin to read ADC
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
    GPIO_TERTIARY_MODULE_FUNCTION);

    //enable ADC
    ADC14_enableModule();

    //set ADC Resolution to 8 bit
    ADC14_setResolution(ADC_8BIT);

    //this is using the masterclock to sample the ADC module (so it's running at 3MHz)
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,
    ADC_NOROUTE);

    ADC14_configureSingleSampleMode(ADC_MEM0, false);

    //using up to 2.5 volts as reference voltage
    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    //conversion memory to intbuf
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS,
    ADC_INPUT_A0,
                                    ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    //Enable the ADC
    ADC14_enableConversion();


    for (;;)
    {
        //at first, go forward if button is pressed
        if (GPIO_INPUT_PIN_LOW
                == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1))
        {
            toggle2=0;
            if (toggle1==0){
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
            //I actually don't know what datatype the counting register is in.  adc is uint8, but the timer seems to be uint16.  However, it seems not through a compile error.
            TA0CCR1 = adc;
            TA0CCR2 = 0;
            toggle1=1;
            //delay to prevent switching issues
            for(delay=0;delay<100000;delay++){}
            }
            else if (toggle1==1){
                //turn off switch next time button is pressed
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                TA0CCR1 =0;
                            TA0CCR2 = 0;
                            toggle1=0;
            }
        }


        //same as above but in reverse
        if (GPIO_INPUT_PIN_LOW
                        == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4))
                {

            toggle1=0;
                    if (toggle2==0){
                        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                                       MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                                       TA0CCR1 = 0;
                                       TA0CCR2 = adc;
                    toggle2=1;
                    for(delay=0;delay<100000;delay++){}
                    }
                    else if (toggle2==1){
                        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                        TA0CCR1 =0;
                                    TA0CCR2 = 0;
                                    toggle2=0;
                    }
                }

        //check the ADC and set the appropriate values for the pwm signals
        ADC14_toggleConversionTrigger();
        while (ADC14_isBusy())
        {}
        adc = ADC14_getResult(ADC_MEM0);
        printf("%d\n", adc);
        if(toggle1==1){TA0CCR1 = adc;
                TA0CCR2 = 0;}
        if(toggle2==1){TA0CCR2 = adc;
                TA0CCR1 = 0;}

    }
}
