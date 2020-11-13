#ifndef wheel_h
#define wheel_h

#include "MPWM.h"

class Wheel
{
private:
    MPWM mpwmobj1;
    MPWM mpwmobj2;

public:
    Wheel(int gpio1,
          int gpio2,
          const int pinfreq,
          const mcpwm_unit_t pwmunit1,
          const mcpwm_unit_t pwmunit2,
          const mcpwm_timer_t timer1,
          const mcpwm_timer_t timer2,
          const mcpwm_generator_t pwmgen1,
          const mcpwm_generator_t pwmgen2,
          const mcpwm_io_signals_t moduleconfig1,
          const mcpwm_io_signals_t moduleconfig2);

    void Move(const float command);
};

#endif