#ifndef TB_H
#define TB_H

#include <systemc>
#include "PWM.h"

class Top : public sc_module
{
    PWM obj_PWM;
    sc_clock i_clk;

    sc_signal<bool> i_rst;
    sc_signal<bool> pwm_out;
    sc_signal<bool> pwm_int;
    sc_signal<bool> trigger_event;

public:
    SC_HAS_PROCESS(Top);
    Top(sc_module_name name);
    void tests();
};

#endif // !TB_H
