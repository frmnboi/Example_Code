#ifndef MPWM_H
#define MPWM_H

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

class MPWM
{
private:
    int gpiopin;
    int pinfrequency{1000};
    //upmode is default
    mcpwm_unit_t pwmunit;
    mcpwm_timer_t timer;
    mcpwm_generator_t pwmgen;
    mcpwm_io_signals_t moduleconfig;

    mcpwm_config_t pwm_config;

public:
    MPWM();
    MPWM(const int gpiopin,
         const int pinfrequency,
         const mcpwm_unit_t pwmunit,
         const mcpwm_timer_t timer,
         const mcpwm_generator_t pwmgen,
         const mcpwm_io_signals_t moduleconfig);
    void initializePin(const mcpwm_unit_t pwmunit, const mcpwm_timer_t timer, const mcpwm_io_signals_t moduleconfig, const int gpiopin);
    void setDuty(const float dutycycle);
    // turnoff();
    // turnon();
};

#endif
