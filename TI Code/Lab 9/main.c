//import the driver library and the prototype and define statements
#include "driverlib.h"
#include <stdio.h>

//the code will take in 4 inputs from the ultrasonic sensors
//output is 4 pwm to 4 dc motors and possibly a servo


//set this to 0 for calibration, and 1 for testing (change this mid-demonstration)
uint8_t experiment = 1;

#define period 1000
//use this define to control what percentage to use in testing.
#define percentage 500
short counter = 0;







#define timer1_period 46875
//1 Hz update rate

const Timer_A_UpModeConfig upConfig_0 =                     // Up Mode
        { TIMER_A_CLOCKSOURCE_SMCLK,                // Timer A is based on SMCLK
          TIMER_A_CLOCKSOURCE_DIVIDER_64, // Slow down the counter 64 times over
          timer1_period,                 // Period of Timer A, stored in TAxCCR0
          TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable interrupts when Timer A rolls over
          TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable interrupt for capture-compare
          TIMER_A_DO_CLEAR                   // Clears counter when initialized
        };

uint8_t temperature = 0;
int index=1;
bool calibrationcomplete=true;


/* I2C Master Configuration Parameter */
const eUSCI_I2C_MasterConfig i2cConfig = {
EUSCI_B_I2C_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
        3000000, // SMCLK = 3MHz
        EUSCI_B_I2C_SET_DATA_RATE_400KBPS, // Desired I2C Clock of 400khz
        0, // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP // No Autostop
        };

void readI2C();

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

    //set pwm pin 2.5 as output
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);

    Interrupt_disableMaster();

    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    //Enable and configure the interrupt for up mode
    Timer_A_enableInterrupt(TIMER_A1_BASE);
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA1_0);
    //this timer A interrupt is consistently changing the values even when debug is running, an

    //Counts in up mode
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    Interrupt_enableMaster();

//    //these together set these pins to primary module function, which apparently is timer A0
//        P2SEL0 |= 0x30; // Set bit 4 and 5 of P2SEL0 to enable TA0.1 functionality on P2.4 and P2.5
//        P2SEL1 &= ~0x30; // Clear bit 4 and 5 of P2SEL1 to enable TA0.1 and TA0.2 functionality on P2.4 and P2.5 respectively
//        P2DIR |= 0x30;     // Set pin 2.4 and 2.5 as an output pins
//        // Set Timer A0 period (PWM signal period).  Shared for all timer A0 instances
//        TA0CCR0 = period;
//        // Set Duty cycle
//        TA0CCR1 = 0;
//        // Set output mode to Reset/Set
//        TA0CCTL1 = OUTMOD_7; // Macro which is equal to 0x00e0, defined in msp432p401r.h
//        //set/reset also works as well.  which is mode 1
//        // Initialize Timer A
//        TA0CTL = TASSEL__SMCLK | MC__UP | TACLR; // Tie Timer A to SMCLK, use Up mode, and clear TA0R
//
//    //how do I go to timer A0 capture compare register 2
//
//        //now the pwm for the reverse direction
//        TA0CCTL2 = OUTMOD_7;


    //these together set these pins to primary module function, which apparently is timer A0
    P2SEL0 |= 0x30; // Set 5 of P2SEL0 to enable TA0.1 functionality on P2.5
    P2SEL1 &= ~0x30; // Clear 5 of P2SEL1 to enable TA0.2 functionality on P2.5
    P2DIR |= 0x30;     // Set pin 2.5 as an output pin

    // Set Timer A0 period (PWM signal period).  Shared for all timer A0 instances
    TA0CCR0 = period;

    // Set Duty cycle
    TA0CCR1 = 0;
    TA0CCR2 = 0;


    // Set output mode to Reset/Set
    TA0CCTL1 = OUTMOD_7; // Macro which is equal to 0x00e0, defined in msp432p401r.h
    TA0CCTL2 = OUTMOD_7;
    //set/reset also works as well.  which is mode 1

    // Initialize Timer A
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR; // Tie Timer A to SMCLK, use Up mode, and clear TA0R

    // PWM signal will now be available on P2.5

    //Configure the I2C module
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN5,
                                                GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN4,
                                                GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig);
    MAP_I2C_enableModule(EUSCI_B1_BASE);

//    /* Set Master in receive mode */
//    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);

#define slaveaddr 0x48    //(1001101b)
#define TEMP 0x00
#define TIMEOUT 1000
    //taken from manufacturer datasheet
    I2C_setSlaveAddress(EUSCI_B1_BASE, slaveaddr);

    //We want to read from the TEMP register on the i2c device
    //you send the slave address, then the register you want to access (TEMP), then the slave address again, and then receive the data.

    //but what is the TEMP address?  it doesn't seem to be unique/specified
    //I'm gussing it's just 0

    //disable the interrupt that we don't need
    Interrupt_disableInterrupt(INT_EUSCIB1);

    //starting the demonstration

    while (1)
    {
//        MAP_PCM_gotoLPM0();
    }

}

void TA1_0_IRQHandler()
{
    //clear interrupt flag
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0);

    if (experiment == 0)
    {
        //the system is currently in the initial calibration stage.
        if (counter <120)
        {
        	//read temperature and increment timer
            readI2C();
            counter++;
        }
        else
        {
        	if(calibrationcomplete){
        		calibrationcomplete=false;
        		printf("Calibration Complete\n");
        	}
        	//change pwm to 50%
        	TA0CCR1 = period / 2;
        	TA0CCR2 = period / 2;
            readI2C();

        }
    }
    else
    {
    	//run the duty cycle test at constant duty cycle
    	TA0CCR2 = percentage;
    	TA0CCR1 = percentage;
        readI2C();

    }

}

void readI2C()
{

	int result = false;

	GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);   // toggle LED1

	    // Set the I2C address of the device we wish to talk to.
	    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, slaveaddr);

	    // Set write mode.
	    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	    // Write START + address + register to the sensor.
	    result = MAP_I2C_masterSendSingleByteWithTimeout(EUSCI_B1_BASE, TEMP, TIMEOUT);

//	    for(;;){
//	    	MAP_I2C_masterSendSingleByteWithTimeout(EUSCI_B1_BASE, TEMP, TIMEOUT);
//	    	short j;
//	    	for(j = 0; j < 1000; j++){}
//
//
//	    }

	    // Delay
	    short i;
	    for(i = 0; i < 1000; i++){}

	    //for some reason this only works with B1 base

	    // Continue only if the write was successful.
	    if (result == true)
	    {
	        // Set read mode.
	        MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);

	        // RESTART.
	        MAP_I2C_masterReceiveStart(EUSCI_B1_BASE);

	        //receive a single byte from sensor
	        result = MAP_I2C_masterReceiveMultiByteFinishWithTimeout(EUSCI_B1_BASE, &temperature , TIMEOUT);
	        //the temperature appears to be biased upwards by 4-5 degrees C
//	        temperature-=4;
	        printf("%d\n", temperature);

	    }
	    else
	    {printf("start failed \n");}
}




//    //send to register
//    //receive temperature
//    //print the temperature in this format [#] temperature F
//
//    //slave address already set
//
//    //set mode to write to communicate with i2c device
//    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
//
//    //following the instructions for reading the temp register
//
//////    for(;;){MAP_I2C_masterSendSingleByteWithTimeout(EUSCI_B1_BASE, TEMP, TIMEOUT);}
////
////
////    // Write START + address + register to the sensor.    result = MAP_I2C_masterSendSingleByteWithTimeout(EUSCI_B1_BASE, reg, TEMP_TIMEOUT);
////    I2C_masterSendMultiByteStart(EUSCI_B1_BASE, slaveaddr);
////
////
////
////    short i = 0;
////    //DELAY
////    for (i = 0; i < 100; i++)
////    {
////    }
////
////    I2C_masterSendMultiByteNext(EUSCI_B1_BASE, TEMP);
////    I2C_masterSendMultiByteStart(EUSCI_B1_BASE, slaveaddr);
////    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
////    temperature = MAP_I2C_masterReceiveSingle(EUSCI_B1_BASE);
////    printf("%d\n", temperature);
//
//    // Write START + address + register to the sensor.
//    if (MAP_I2C_masterSendSingleByteWithTimeout(EUSCI_B1_BASE, TEMP, TIMEOUT))
//    {    //start byte sent
//
//        short i = 0;
//        //DELAY
//        for (i = 0; i < 1000; i++)
//        {
//        }
//        // Set read mode.
//        MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
//        // RESTART.
//        MAP_I2C_masterReceiveStart(EUSCI_B1_BASE);
//        //Receive the value in the TEMP register that the device will send.
//        temperature = (signed char) MAP_I2C_masterReceiveSingle(EUSCI_B1_BASE);
//
//        printf("[%d] %d degrees Celsius\n", index, temperature);
//
//        index++;
//
//        //this is different from what the manual tells us to do to access the temperature register
//
//    }
//    else
//    {
//        printf("failed to initialize I2C connection\n");
//    }
