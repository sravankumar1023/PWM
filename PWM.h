#ifndef PWM_H
#define PWM_H

#include <systemc>
#include <cstdint>
#include "MACRO.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;


class PWM_registers
{
private:
    uint32_t m_value;
    uint32_t m_reset_value;
    uint32_t m_offset;
    uint32_t m_write_mask;
    uint32_t m_read_mask;
    string name;

public:
    SC_HAS_PROCESS(PWM_registers);
    PWM_registers(string a_name, uint32_t a_offset, uint32_t a_reset_value, uint32_t a_write_mask, uint32_t a_read_mask);
    void write(const uint32_t& a_value);
    void read(uint32_t& a_value);
    void reset();
};

class PWM : public sc_module
{
    PWM_registers PID;  //offset/address 00H
    PWM_registers PCR;  //offset/address 04H
    PWM_registers CFG;  //offset/address 08H
    PWM_registers START;//offset/address 0CH
    PWM_registers RPT;  //offset/address 10H
    PWM_registers PER;  //offset/address 14H
    PWM_registers PH1D; //offset/address 18H

    /* ---------- Internal Purpose ---------- */
    uint32_t period_cnt;
    uint32_t periods_left;
    uint32_t phase_cnt;
    uint32_t  rpt_cnt;

public:
    sc_in<bool> i_rst;
    sc_in<bool> i_clk;
    sc_in<bool> trigger_event; // VPIF & GPIO event trigger

    sc_out<bool> pwm_out;
    sc_out<bool> pwm_int;

    bool running;
    bool last_trigger;

    SC_HAS_PROCESS(PWM);
    PWM(sc_module_name name);
    void write(const uint32_t& address, const uint32_t& a_value);
    void read(const uint32_t& address, uint32_t& a_value);
    void reset();
    void pwm_log();
};


#endif // !PWM_H
