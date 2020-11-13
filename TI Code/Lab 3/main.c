//import the driver library and the prototype and define statements
#include "driverlib.h"
#include "Userfunctions.h"

//create a global variable LEDcounter that is used to change the LED color
unsigned short LEDcounter=0;

int main(void)
{
    //Taken from template:

    // Halt watchdog timer
    MAP_WDT_A_holdTimer();

    // Configure Red LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    //Configure the 3 color LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN1);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    // Configure Switches
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);

    //use pressed variable to stop the program from making all 3 colors cycle too quickly for the human eye
    unsigned short pressed=1;
    while(1){
        //if one switch is pressed, turn on the light, otherwise, turn it off
        if(GPIO_INPUT_PIN_LOW == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1)){
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);}
        else{
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        }

        //if the other switch is pressed, change the light color and turn on the light
        if (GPIO_INPUT_PIN_LOW == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4)){
            if (pressed) {
                pressed=0;
                toggleRGB();}
        }
        else{
            pressed=1;
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }


        }


    }



void toggleRGB(void){
            //increment the LED counter
            LEDcounter++;

            //use LEDcounter as a switch to set one color on high and all others on low
            LEDcounter%=3;
            switch (LEDcounter){

            case 0:
                            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
                            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
                            break;
            case 1:
                            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
                            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
                            break;
            case 2:
                            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
                            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
                            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
                            break;

            }
}



//version of the code that doesn't allow simultaneous button presses


//    while(1){
//
//        while(GPIO_INPUT_PIN_LOW == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1)){
//            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
//        }
//
//        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
//
//
//        if (GPIO_INPUT_PIN_LOW == MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4)){
//            toggleRGB();
//
//            while (GPIO_INPUT_PIN_LOW ==MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4))
//            {}
//            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
//            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
//            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
//
//        }
//    }
//
//}
