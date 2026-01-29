#include "TB.h"

Top::Top(sc_module_name name) :sc_module(name)
, obj_PWM("Top")
, i_clk("clk_10ns", 1, SC_NS, 0.5, 0, SC_NS, true)
//(name, period, unit, duty_cycle, starrt_time, start_time_unit, posedge_first=true)
{
    obj_PWM.i_rst(i_rst);
    obj_PWM.i_clk(i_clk);
    obj_PWM.trigger_event(trigger_event);
    obj_PWM.pwm_out(pwm_out);
    obj_PWM.pwm_int(pwm_int);

    SC_THREAD(tests)
}

void Top::tests()
{
    uint32_t read_val = 0, write_val = 0xFFFFFFFF, addr;
#if 1
    cout << "------------------- READ WRITE TEST START -------------------" << endl;
    addr = PID_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != PID_READ_MASK_VAL)
    {
        cout << "PID REG READ WRITE FAILED" << endl;
        sc_stop();
    }

    addr = PCR_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != 0x1)
    {
        cout << "PCR REG READ WRITE FAILED" << endl;
        sc_stop();
    }


    addr = CFG_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != 0x7F)
    {
        cout << "CFG REG READ WRITE FAILED" << endl;
        sc_stop();
    }


    addr = START_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != 0x01)
    {
        cout << "START REG READ WRITE FAILED" << endl;
        sc_stop();
    }


    addr = RPT_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != 0xFF)
    {
        cout << "RPT REG READ WRITE FAILED" << endl;
        sc_stop();
    }

    addr = PER_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != 0xFFFFFFFF)
    {
        cout << "PER REG READ WRITE FAILED" << endl;
        sc_stop();
    }

    addr = PH1D_OFFSET_VAL;
    obj_PWM.write(addr, write_val);
    obj_PWM.read(addr, read_val);
    if (read_val != 0xFFFFFFFF)
    {
        cout << "PH1D REG READ WRITE FAILED" << endl;
        sc_stop();
    }

    cout << "------------------- READ WRITE TEST PASSED ------------------" << endl;
    cout << "------------------- RESET TEST START ------------------------" << endl;
    cout << "RESET APPLIED" << endl;
    i_rst = true;
    wait(SC_ZERO_TIME);
    i_rst = false;
    wait(SC_ZERO_TIME);

    addr = PID_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != PID_RESET_VAL)
    {
        cout << "PID REG RESET FAILED" << endl;
        sc_stop();
    }

    addr = PCR_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != PCR_RESET_VAL)
    {
        cout << "PCR REG RESET FAILED" << endl;
        sc_stop();
    }

    addr = CFG_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != CFG_RESET_VAL)
    {
        cout << "CFG REG RESET FAILED" << endl;
        sc_stop();
    }

    addr = START_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != START_RESET_VAL)
    {
        cout << "START REG RESET FAILED" << endl;
        sc_stop();
    }

    addr = RPT_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != RPT_RESET_VAL)
    {
        cout << "RPT REG RESET FAILED" << endl;
        sc_stop();
    }

    addr = PER_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != PER_RESET_VAL)
    {
        cout << "PER REG RESET FAILED" << endl;
        sc_stop();
    }

    addr = PH1D_OFFSET_VAL;
    obj_PWM.read(addr, read_val);
    if (read_val != PH1D_RESET_VAL)
    {
        cout << "PH1D REG RESET FAILED" << endl;
        sc_stop();
    }
#endif
    wait(2, SC_NS);
    cout << "------------------- RESET TEST PASSED -----------------------" << endl;

    cout << "------------------- ONE SHOT MODE START ---------------------" << endl;
    cout << "------------------- ONE SHOT MODE CONFIG --------------------" << endl;
    obj_PWM.write(RPT_OFFSET_VAL, 2<< RPT_BIT_POS); //RPT
    obj_PWM.write(PER_OFFSET_VAL, 5 << PER_BIT_POS); //PER+1
    obj_PWM.write(PH1D_OFFSET_VAL, 3 << PH1D_BIT_POS);

    obj_PWM.write(CFG_OFFSET_VAL, 1 << CFG_INTEN_BIT_POS | 0 << CFG_INACTOUT_BIT_POS | 1 << CFG_P1OUT_BIT_POS | ONE_SHOT_MODE);  //one shot mode, p1out =1, int=1;
    obj_PWM.write(START_OFFSET_VAL, 1<< START_BIT_POS);
    wait(15, SC_NS);
    bool int_val = pwm_out.read();
    if (int_val != 1)
    {
        cout << "ONE SHOT MODE FAILED" << endl;
        sc_stop();
    }
    wait(40, SC_NS);
    cout << "------------------- ONE SHOT MODE PASSED --------------------" << endl;

    std::cout << "ALL TESTS DONE" << std::endl;
    sc_stop();
}