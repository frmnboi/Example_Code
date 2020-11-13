// #ifndef PWM_H
// #define PWM_H

// #include <driver/ledc.h>
// //interestingly, it cannot determine the datatype of the enumerations for PWM without including the header file here


// class PWM
// {
// private:
//     unsigned int PWMcount;
//     unsigned int PWMdutycount;


//     int timer;
//     ledc_timer_bit_t PWMres;
//     //timer initialization
//     static inline bool timerinit = false;

// public:
//     PWM(float dutycycle, int gpionum, int channel, ledc_timer_bit_t PWMres=LEDC_TIMER_12_BIT, int freq=1000);
//     // template <typename wholenum, typename fractional>
//     // PWM(wholenum PWMcountval, fractional dutycycle);

//     bool setDutyCycle(float dutycycle);
//     float getDutyCycle();
//     void display() const;
//     int getDutyCount();

// };

// #endif