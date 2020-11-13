#include "Wheel.h"
#include "MPWM.h"

Wheel::Wheel(int gpio1,
             int gpio2,
             const int pinfreq,
             const mcpwm_unit_t pwmunit1,
             const mcpwm_unit_t pwmunit2,
             const mcpwm_timer_t timer1,
             const mcpwm_timer_t timer2,
             const mcpwm_generator_t pwmgen1,
             const mcpwm_generator_t pwmgen2,
             const mcpwm_io_signals_t moduleconfig1,
             const mcpwm_io_signals_t moduleconfig2)
{
    //initialize the MPWM objects
    //pwmobj1 is the one corresponding to foward motion and vice versa
    MPWM mpwmobj1{gpio1, pinfreq, pwmunit1, timer1, pwmgen1, moduleconfig1};
    MPWM mpwmobj2{gpio2, pinfreq, pwmunit2, timer2, pwmgen2, moduleconfig2};
}

void Wheel::Move(const float command)
{
    if (command > 100)
    {
        mpwmobj1.setDuty(static_cast<float>(100));
        mpwmobj2.setDuty(static_cast<float>(0));
        return;
    }
    if (command < -100)
    {
        mpwmobj1.setDuty(static_cast<float>(0));
        mpwmobj2.setDuty(static_cast<float>(100));
        return;
    }

    if (command > 0)
    {
        mpwmobj1.setDuty(command);
        mpwmobj2.setDuty(static_cast<float>(0));
        return;
    }
    else
    {
        if (command < 0)
        {
            mpwmobj2.setDuty(command);
            mpwmobj1.setDuty(static_cast<float>(0));
            return;
        }
        else
        {

            mpwmobj1.setDuty(static_cast<float>(0));
            mpwmobj2.setDuty(static_cast<float>(0));
            return;
        }
    }
}
