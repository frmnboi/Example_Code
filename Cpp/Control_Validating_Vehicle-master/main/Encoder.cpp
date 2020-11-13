#include "Encoder.h"

// #include "driver/periph_ctrl.h"
#include "driver/pcnt.h"

//each encoder object corresponds to a single encoder pair in each wheel
//see attached drawing for the encoding rules for directionality

Encoder::Encoder(int pin1, int pin2, int counterunit)
{
    counterUnit = counterunit;

    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = pin1,
        .ctrl_gpio_num = pin2,
        // What to do when control input is low or high?
        //normally, we have upcounting on any edge transition (forward increase)
        .lctrl_mode = PCNT_MODE_KEEP,    // Do nothing if control pin is low
        .hctrl_mode = PCNT_MODE_REVERSE, // Reverse if high

        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC, // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DEC, // Count down on the negative edge
        
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = INT16_MAX, 
        .counter_l_lim = INT16_MIN,
        
        //one counting unit per wheel (each unit has 2 channels which are both used)
        .unit = static_cast<pcnt_unit_t>(counterunit),
        //Only 1 channel will be used for counting.
        .channel = PCNT_CHANNEL_0,
    };

    // Initialize PCNT unit 
    pcnt_unit_config(&pcnt_config);

    // Configure and enable the input filter 
    //pulses should not be faster than 3300RPM/60*12 Hz
    //The clock frequency is APB_CLK cycles
    //we need (3300RPM/60*12 Hz)/APB_CLK cycles of ignoring

    //ignores anything less than the timer
    //1023 is also the limit for the number --> (APB_CLK_FREQ / 660) * .9 >1023 so 1023 is the best we can do
    pcnt_set_filter_value(PCNT_UNIT_0, 1023);
    pcnt_filter_enable(PCNT_UNIT_0);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);
};

//note that there needs to be testing on overflow/underflow here, as int16_t used by the counter can overflow/underflow if maxed out too much and no documentation of the results exists
//an ISR bound could work to correct this issue, or calculation can be done in the processing algorithm to account for "jumps"
//however, both are beyond this initial proof of concept
int Encoder::count()
{
    int16_t value;
    int16_t *ptr=&value;
    pcnt_get_counter_value((pcnt_unit_t)counterUnit, ptr);
    return static_cast<int>(value);
}

