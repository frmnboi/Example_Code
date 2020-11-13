//After Talking with Dr. Rogers
//The interrupt should only send or receive a single 8 bit character, not loop or do anything
//do not reset the interrupt flag, because by default it is always set high.  By having the callback there, it may enter into the infinite loop
//you can communicate over any of the uart channels and it should be picked up by the computer
//do not printf, because it communicates through the usb uart0, which we also are using here.  It will trigger interrupts

//the solution polls until it sees an enter.  It then loops in the while loop, sending out a signal and waiting until the hardware resets the UART send flag to high.


#include <stdio.h>

//import the driver library
#include "driverlib.h"

//it seems that the prototpyes are not needed
//void UART_Receive();
//void UART_Send();

//define global variables index and charbuffer
uint32_t ind=0;
//const int baudrate=38400;
volatile char charbuffer[200]="";

void main(void)
{
    // Halt watchdog timer
    MAP_WDT_A_holdTimer();

    // Set timer frequency
    unsigned int dcoFrequency = 3E+6;
     MAP_CS_setDCOFrequency(dcoFrequency);
     MAP_CS_initClockSignal(CS_SMCLK,CS_DCOCLK_SELECT,CS_CLOCK_DIVIDER_1);

     //UART configuration for 38400 baud rate
     const eUSCI_UART_Config uartConfig =
     {
     EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
     4, // Calculated using 3MHz and 38400bps
     14,
     0,
     EUSCI_A_UART_NO_PARITY, // No Parity
     EUSCI_A_UART_LSB_FIRST, // LSB First
     EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
     EUSCI_A_UART_MODE, // UART mode
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
     };

     //set the GPIO ports to UART
     MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                        GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);


     /* Configuring and enabling UART Module */
     MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

     //enable interrupts for both receive and transmit
     MAP_UART_enableModule(EUSCI_A0_BASE);

     //clear flag to stop the interrupt for starting without a call
     UART_clearInterruptFlag ( EUSCI_A0_BASE, 0x3 );

     //EUSCI_A_UART_RECEIVE_INTERRUPT | EUSCI_A_UART_TRANSMIT_INTERRUPT
//     MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
     //Interrupt on both the receive and transmit interrupt
     MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT | EUSCI_A_UART_TRANSMIT_INTERRUPT);

     //throwaway code for a second interrupt
     //for the transmit interrupt
//     MAP_UART_initModule(EUSCI_A1_BASE, &uartConfig);
//     MAP_UART_enableModule(EUSCI_A1_BASE);
//     UART_clearInterruptFlag (EUSCI_A1_BASE, 0xF );
//     MAP_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);

     //setup interrupts on NVIC.  Without it, UART interrupt does nothing.
     Interrupt_disableMaster();

     //Enable interrupt and set priority
     Interrupt_enableInterrupt( INT_EUSCIA0 );
     Interrupt_setPriority ( INT_EUSCIA0, 2 );

//     Interrupt_enableInterrupt( INT_EUSCIA1 );
//     Interrupt_setPriority ( INT_EUSCIA1,1);

     Interrupt_enableMaster();

     //UART is now configured

//     printf("Start of Infinite Loop \n");

//     UART_transmitData(EUSCI_A1_BASE, letter);
//     while(1){UART_transmitData(EUSCI_A0_BASE, letter);} //this triggers the interrupt every time it is called

     //Wait for interrupt while doing nothing.
     while(1){}
     //the UART interrupt will do the work

}

void UART_Receive()
{
    //try just checking which bit was activated, because I think it may be able to stop an infinite loop, inneficient as it is.
    //check if the receive interrupt was called
    if ((UCA0IFG & 0x0002)^0x0002){
//        printf("Receive interrupt was called \n");
        //read the data into an array
        uint8_t character=(uint8_t) UART_receiveData (EUSCI_A0_BASE);
//        printf("%c \n",(char)character);
        charbuffer[ind]=character;
        if (ind==200){
            //clobber data if you have more than 200 characters
            ind=0;
        }
        else{
            ind++;
        }
        //clear read interrupt flag
        UART_clearInterruptFlag ( EUSCI_A0_BASE, 0x1);

        //if enter is pressed, send the message back
        if (character==0x0D){
//            printf("return was pressed just now \n");
    //        UART_transmitData(EUSCI_A0_BASE, letter);
//            ind++;
//            charbuffer[ind]=0x0A;
//            charbuffer[ind+1]=(uint8_t) '\r';
//            charbuffer[ind+2]=(uint8_t) '\n';

            //transmit data over UART
            uint16_t i=0;
            for (i=0;i<ind;i++){
                //for some reason, it also doesn't work without the printf statement.  For what reason, I couldn't explain
                printf("%c",charbuffer[i]);
                UART_transmitData(EUSCI_A0_BASE, charbuffer[i]);
            }
            //to make this work, you need to break on the UART transmit data and move it forward.  For some reason, putting in the newline character anywhere, in for loop or after causes an infinite loop inexplicably
            //notably, it enters this loop until a received interrupt is called, where it stops, and starts with it transmits.
            printf("return");
            UART_transmitData(EUSCI_A0_BASE, (uint8_t) 10);
//            UART_transmitData(EUSCI_A0_BASE, (uint8_t) '\r');
//            UART_transmitData(EUSCI_A0_BASE, (uint8_t) '\n');

            //reset index to 0 to start again
            ind=0;
        }
    }
    else
    {
        //if the transmit interrupt is called, clear the flag.
        UART_clearInterruptFlag (EUSCI_A0_BASE, 0x2);
    }


    //clear interrupt flag
    //0x2 clears the flag for write.  0x1 clears the flag for read.  But how could you know this was the one to clear?  I had to use 0xF at first to clear all
//    UART_clearInterruptFlag ( EUSCI_A0_BASE, 0xF);
}


//Unused function for a second interrupt
void UART_Send(){
//    printf("Send interrupt was called \n");
    UART_clearInterruptFlag (EUSCI_A1_BASE, 0xF);
}
