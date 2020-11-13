#include "MPWM.h"

//this object will be wrapped for a wheel object, one forward and the other backward

//PWM has a timer config and a channel config.
//to change the PWM output,

MPWM::MPWM(){};

MPWM::MPWM(const int gpiopin,
           const int pinfrequency,
           const mcpwm_unit_t pwmunit,
           const mcpwm_timer_t timer,
           const mcpwm_generator_t pwmgen,
           const mcpwm_io_signals_t moduleconfig)
{
    initializePin(pwmunit, timer, moduleconfig, gpiopin);
};

void MPWM::initializePin(const mcpwm_unit_t pwmunit, const mcpwm_timer_t timer, const mcpwm_io_signals_t moduleconfig, const int gpiopin)
{
    mcpwm_gpio_init(pwmunit, moduleconfig, gpiopin);
    pwm_config.frequency = pinfrequency; //frequency = 500Hz,
    pwm_config.cmpr_a = 0;               //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;               //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(pwmunit, timer, &pwm_config); //Configure PWM0A & PWM0B with above settings
    mcpwm_start(pwmunit, timer);
}


void MPWM::setDuty(const float dutycycle)
{
    mcpwm_set_duty(pwmunit, timer, pwmgen, dutycycle);
};

// void MPWM::turnoff();
// void MPWM::turnon();


// //use this if the pwm signal was previously set to low/high
// mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
