// #include "PWM.h"
// //note that in C, you would define the header file as the functional templates, and this .c file would be for the identical functional templates with content filled in for each function

// // #include <stdio.h>
// #include <cstdio>
// //interestingly, this test has shown that the c++ linker will bring this in to the main body so functions in the main use cstdio as well

// #include <math.h>   


// //this object will be wrapped for a wheel object, one forward and the other backward

// //PWM has a timer config and a channel config.
// //to change the PWM output, 


// //freq is defaulted to 1000.  PWMres is 12 bit
// PWM::PWM(float dutycycle, int gpionum, int channel, int timer, ledc_timer_bit_t PWMres, int freq)
// {
//     timer=timer;
//     static ledc_timer_config_t timer_conf;
//     timer_conf.duty_resolution = PWMres;
//     timer_conf.freq_hz = freq;
//     //use high speed mode.  Note that 8 channesl can be high speed, and we only need 8 pwm signals
//     timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
//     timer_conf.timer_num = static_cast<ledc_timer_t>(timer);

//     //initialize the PWM timer
//     if (!timerinit)
//         ledc_timer_config(&timer_conf);

//     static ledc_channel_config_t channelconfig;
//     channelconfig.gpio_num = gpionum;
//     channelconfig.speed_mode = LEDC_HIGH_SPEED_MODE;
//     channelconfig.channel = static_cast<ledc_channel_t>(channel);
//     channelconfig.intr_type = LEDC_INTR_DISABLE;
//     channelconfig.duty = PWMdutycount;
//     channelconfig.hpoint = 0;

//     ledc_channel_config(&channelconfig);
// };
// //using templates
// // template <typename wholenum, typename fractional>
// // PWM::PWM(wholenum PWMcountval, fractional dutycycle)
// // {
// //     PWMcount = static_cast<unsigned int>(PWMcountval);
// //     setDutyCycle(static_cast<float>(dutycycle));
// // }; //with setting details

// bool PWM::setDutyCycle(float dutycycle, int freq)
// {
//     if (0 <= dutycycle && dutycycle <= 1)
//     {
//         PWMdutycount = static_cast<unsigned int>(dutycycle * PWMcount);

//         static ledc_timer_config_t timer_conf;
//         timer_conf.duty_resolution = PWMres;
//         timer_conf.freq_hz = freq;
//         //use high speed mode.  Note that 8 channesl can be high speed, and we only need 8 pwm signals
//         timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
//         timer_conf.timer_num = static_cast<ledc_timer_t>(timer);

//     //initialize the PWM timer
//     if (!timerinit)
//         ledc_timer_config(&timer_conf);

//     static ledc_channel_config_t channelconfig;
//     channelconfig.gpio_num = gpionum;
//     channelconfig.speed_mode = LEDC_HIGH_SPEED_MODE;
//     channelconfig.channel = static_cast<ledc_channel_t>(channel);
//     channelconfig.intr_type = LEDC_INTR_DISABLE;
//     channelconfig.duty = PWMdutycount;
//     channelconfig.hpoint = 0;

//     ledc_channel_config(&channelconfig);

//         return true;
//     }
//     else
//     {
//         {
//             printf("Invalid Duty Cycle.  Value must be between 0 and 1\n");
//             return false;
//         }
//     }
// };

// //note the float conversion is to prevent ambiguity
// float PWM::getDutyCycle() { return static_cast<float>(PWMdutycount/(std::pow((float)2,PWMres)-1)); };

// //I can't figure out how to use const functions.  For me, they're not important, but for some reason, the compiler will never accept it.  Even on returning an explicietly const variable.
//  int PWM::getDutyCount() { return PWMres; };
