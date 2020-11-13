//import the driver library and the prototype and define statements
#include "driverlib.h"
#include <stdio.h>


//mode 0 is read from adc
//mode 1 is write
//mode 2 is read and send over uart
uint8_t modevar = 0;

//set the dco frequency to 3MHz
uint32_t dcoFreq = 3E+6;


const int timer1_period = 46875; // 3E+6/64 = 46875 Hz --> read once every second if you count to this number

const float tempgain = .0162293; //(float)5/3/1000;

volatile float voltage;
volatile float temp;
//counter variables
int numtempvals = 0;
int i = 0;
int j = 0;
uint8_t len = 0;

//I needed this because there is likely some hardware error with the microprocessor that keeps resetting the modevar variable to 1, but only after all 30 items have been picked.  Please ask about this.
bool completed = false;

//initialize arrays
float floatarr[30] = { };
char asciiarr[181] = { };
char temparr[4] = { };

const Timer_A_UpModeConfig upConfig_0 =                     // Up Mode
        { TIMER_A_CLOCKSOURCE_SMCLK,                // Timer A is based on SMCLK
          TIMER_A_CLOCKSOURCE_DIVIDER_64, // Slow down the counter 64 times over
          timer1_period,                 // Period of Timer A, stored in TAxCCR0
          TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable interrupts when Timer A rolls over
          TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable interrupt for capture-compare
          TIMER_A_DO_CLEAR                   // Clears counter when initialized
        };

//    //UART configuration for 38400 baud rate
//        //note that SMCLK is the same as MCLK here and is 3MHz
//        const eUSCI_UART_Config uartConfig = {
//        EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
//                4, // Calculated using 3MHz and 38400bps
//                14,
//                0,
//                EUSCI_A_UART_NO_PARITY, // No Parity
//                EUSCI_A_UART_LSB_FIRST, // LSB First
//                EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
//                EUSCI_A_UART_MODE, // UART mode
//                EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
//                };

    //UART configuration for 9600 baud rate
    //note that SMCLK is the same as MCLK here and is 3MHz

    const eUSCI_UART_Config uartConfig = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
            19, // Calculated using 3MHz and 38400bps
            8, 0,
            EUSCI_A_UART_NO_PARITY, // No Parity
            EUSCI_A_UART_LSB_FIRST, // LSB First
            EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
            EUSCI_A_UART_MODE, // UART mode
            EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
            };


#define MAIN_1_SECTOR_31 0x0003F000   // Address of Main Memory, Bank 1, Sector 31

//function prototypes
static void readMemory();
static void write2Memory();

int main(void)
{    // Halt watchdog timer
    MAP_WDT_A_holdTimer();

    //enable floating point module
    MAP_FPU_enableModule();

    // Set DCO clock source frequency
    CS_setDCOFrequency(dcoFreq);
    // Set Submaster Clock to be 1-1 with DCO.  The larger the divider, the slower the clock signal will be
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //Set pin to read ADC
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
    GPIO_TERTIARY_MODULE_FUNCTION);

    //enable ADC
    ADC14_enableModule();

    //set ADC Resolution to 10 bit
    ADC14_setResolution(ADC_10BIT);

    //this is using the masterclock to sample the ADC module (so it's running at 3MHz)
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,
    ADC_NOROUTE);

    ADC14_configureSingleSampleMode(ADC_MEM0, false);

    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    //conversion memory to intbuf
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_INTBUF_VREFNEG_VSS,
    ADC_INPUT_A0,
                                    ADC_NONDIFFERENTIAL_INPUTS);

    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    //Enable the ADC
    ADC14_enableConversion();

    Interrupt_disableMaster();
    //interrupt not when the ADC is complete, but when 1 second has passed and timer A is finished
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);
    //Enable and configure the interrupt for up mode
    Timer_A_enableInterrupt(TIMER_A0_BASE);
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_enableInterrupt(INT_TA0_0);

    //wasn't in the notes.  Starts the counter.  Counts in up mode to the equivalent time for 1 second
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Interrupt_enableMaster();

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

    //use pressed variable to stop the program from making all 3 colors cycle too quickly for the human eye

    //default mode is 0, so turn on the proper light
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);


    //set the GPIO ports to UART
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P1,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3,
            GPIO_PRIMARY_MODULE_FUNCTION);

    //notably, using the A1_BASE that should also work for UART does absolutely nothing.  this is carried over from the uart lab.  Why? ***************************************************
    /* Configuring and enabling UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    //enable interrupts for both receive and transmit
    MAP_UART_enableModule(EUSCI_A0_BASE);

    while (1)
    {
        //if one switch is pressed, turn on one light, turn the other off, and set mode to data acquisition
        if (GPIO_INPUT_PIN_LOW
                == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1)
                && modevar != 0)
        {
            //small red LED
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
            modevar = 0;
            printf("Input mode triggered\n");
        }

        //if the other switch is pressed, reverse the lights and set the mode to write
        if (GPIO_INPUT_PIN_LOW
                == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4)
                && modevar != 2)
        {
            //yellow light
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
            modevar = 2;

        }

        //Completed data collection
        if (modevar == 1) // && !completed
        {
            //green LED
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

            write2Memory();

            //set it to a nonexistant mode so the buttons will work again
            modevar = 7;
            completed = true;

        }

        //Output mode
        if (modevar == 2)
        {

            Interrupt_disableMaster();
            printf("the output mode has been triggered\n");

            readMemory();

            Interrupt_enableMaster();

            //yellow light after send
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);

            //set it to a nonexistant mode so the buttons will work again
            modevar = 7;
        }

    }

}

//note that if this isn't specified, it goes to the default null interrrupt and goes into an infinite while loop every interrupt
void TA0_0_IRQHandler()
{
    //clear interrupt flag
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0);

    //only collect if in data collection mode
    if (modevar == 0)
    {
        //wait for adc to be ready
        while (ADC14_isBusy())
        {
        };
        //convert values and print the converted temperature
        ADC14_toggleConversionTrigger();
        uint16_t adc = ADC14_getResult(ADC_MEM0);
        temp = (float) adc / 1023 * 150;
        printf("%.1f\n", temp);

        //collect 30 values and change mode to write to memory
        if (numtempvals < 30)
        {
//        modevar=0;
            floatarr[numtempvals] = temp;
            numtempvals++;
        }
        else
        {
            //you have collected 30 datapoints and need to write them to memory
            modevar = 1;
            numtempvals = 0;
        }
    }

}

static void write2Memory()
{
    // Unprotecting Main Bank 1, Sector 31
    MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);

    //Erase sector to write to it
    if (!MAP_FlashCtl_eraseSector(MAIN_1_SECTOR_31))
    {
        printf("Erase failed\r\n");
    }

    //write to memory
    if (!MAP_FlashCtl_programMemory(floatarr, (void*) MAIN_1_SECTOR_31, 151))
    {
        printf("Write failed\r\n");
    }
    else
    {
        printf("Write success\r\n");

    }
    /* Setting the sector back to protected  */
    MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR31);
}

static void readMemory()
{
    // Assume data is already in Main memory, Bank 1, Sector 31 from running program last time

    float *floats_from_flash = (float*) MAIN_1_SECTOR_31;

    //while it is not a null character, pull from memory
    while (*floats_from_flash)
    {
        //convert float to ascii characters and send over Uart
        len = (uint8_t) sprintf(temparr, "%.1f\r\n", floats_from_flash[i]);
        for (j = 0; j < len; j++)
        {
            UART_transmitData(EUSCI_A0_BASE, temparr[j]);
            printf("%c", temparr[j]);
        }
        //move memory addresses
        *floats_from_flash++;
    }

}
