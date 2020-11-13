#include "driverlib.h"
#include <stdio.h>

//set the dco frequency to 3MHz
uint32_t dcoFreq = 3E+6 ;

//strangely, the period chosen seems to change the value
const int timer1_period = 46875;                            // 3E+6/64 = 46875 Hz --> read once every second if you count to this number

//the farenheit temperature sensor has a 10mV/F scale factor
//at 150F, it should produce 1.5V.  To scale this to 2.5 volts, the op-amp setup needs to have a gain of 2.5/1.5=1.6666
//the expected gain with respect to temperature in F is 10mV/F*1.6666666=16.66666mv/F=.01666666V/F
//for the actual system, the system gain was adjusted such that 2.5 would not be exceeded
//.785V was measured from the sensor and 1.274V was measured from the Op-Amp
//This gives a gain of 1.62293
//Reversing it, we get 10mV/F*1.62293=.0162293V/F

const float tempgain=.0162293; //(float)5/3/1000;

volatile float voltage;                                     // Calculated temperature
volatile float temp;
int i=0;

const Timer_A_UpModeConfig upConfig_0 =                     // Up Mode
{   TIMER_A_CLOCKSOURCE_SMCLK,                              // Timer A is based on SMCLK
    TIMER_A_CLOCKSOURCE_DIVIDER_64,                         // Slow down the counter 64 times over
    timer1_period,                                          // Period of Timer A, stored in TAxCCR0
    TIMER_A_TAIE_INTERRUPT_DISABLE,                         // Disable interrupts when Timer A rolls over
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,                     // Enable interrupt for capture-compare
    TIMER_A_DO_CLEAR                                        // Clears counter when initialized
};


void main(void)
 {
    //Halt Watchdog Timer
    WDT_A_holdTimer();
    // Set DCO clock source frequency
    CS_setDCOFrequency(dcoFreq);
    // Set Submaster Clock to be 1-1 with DCO.  The larger the divider, the slower the clock signal will be
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //Set pin to read ADC
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);

    //enable ADC
    ADC14_enableModule();

    //set ADC Resolution to 10 bit
    ADC14_setResolution(ADC_10BIT);

    //this is using the masterclock to sample the ADC module (so it's running at 3MHz)
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);

    ADC14_configureSingleSampleMode(ADC_MEM0, false);

    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    //conversion memory to intbuf
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    //set the ENC bit

    //enable manual sampling so we only sample every 1 second when the timer is run.
    //contrary to the slides, this needs to be put before enabling conversion for some reason
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    //Enable the ADC
    ADC14_enableConversion();





    Interrupt_disableMaster();
    //interrupt not when the ADC is complete, but when 1 second has passed and timer A is finished
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    //Enable and configure the interrupt for up mode
    Timer_A_enableInterrupt(TIMER_A0_BASE);
    Timer_A_configureUpMode(TIMER_A0_BASE,&upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);

    //wasn't in the notes.  Starts the counter.  Counts in up mode to the equivalent time for 1 second
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Interrupt_enableMaster();


    //enter infinite loop and wait for interrupt
    while(1){}
}

void TA0_0_IRQHandler()
{
    //clear interrupt flag
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);

    //for some reason, the interrupt flag does nothing here.
//  Timer_A_clearInterruptFlag(TIMER_A0_BASE|TIMER_A1_BASE|TIMER_A2_BASE|TIMER_A3_BASE);

    //Wait until the ADC is finished with some other conversion.
    while(ADC14_isBusy()){};

    //start a new conversion
    ADC14_toggleConversionTrigger();


    //read the result of the conversion
    uint16_t adc = ADC14_getResult(ADC_MEM0);

    //multiply by 2.5, which is the max voltage to convert to voltage
    //multiply by the temperature gain of the circuit, divide by 2^10-1 subdivisions
    //tempgain is .0162293V/F
//    voltage = ((float)adc/1023*2.5)/tempgain;
//    temperature = (float ) adc /1023
    temp=(float) adc/1023*150;
//    printf("%f Volts Read\n",adc);
//    printf("%f Degrees F\n",voltage);
    printf("%f\n",temp);
}

