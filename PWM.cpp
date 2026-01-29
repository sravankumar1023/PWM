#include "PWM.h"

PWM_registers::PWM_registers(string a_name, uint32_t a_offset, uint32_t a_reset_value, uint32_t a_write_mask, uint32_t a_read_mask)
        : name(a_name)
        , m_reset_value(a_reset_value)
        , m_offset(a_offset)
        , m_write_mask(a_write_mask)
        , m_read_mask(a_read_mask)
{
     //cout << "PWM_reg constructor" << endl;
     m_value = m_reset_value;
}

void PWM_registers::write(const uint32_t& a_value)
{
    m_value &= ~m_write_mask;
    m_value |= (a_value & m_write_mask);
}

void PWM_registers::read(uint32_t& a_value)
{
    a_value = m_value & m_read_mask;
}

void PWM_registers::reset()
{
   m_value = m_reset_value;
}


PWM::PWM(sc_module_name name) :sc_module(name)
        , PID("PID", PID_OFFSET_VAL, PID_RESET_VAL, PID_WRITE_MASK_VAL, PID_READ_MASK_VAL)
        , PCR("PCR", PCR_OFFSET_VAL, PCR_RESET_VAL, PCR_WRITE_MASK_VAL, PCR_READ_MASK_VAL)
        , CFG("CFG", CFG_OFFSET_VAL, CFG_RESET_VAL, CFG_WRITE_MASK_VAL, CFG_READ_MASK_VAL)
        , START("START", START_OFFSET_VAL, START_RESET_VAL, START_WRITE_MASK_VAL, START_READ_MASK_VAL)
        , RPT("RPT", RPT_OFFSET_VAL, RPT_RESET_VAL, RPT_WRITE_MASK_VAL, RPT_READ_MASK_VAL)
        , PER("PER", PER_OFFSET_VAL, PER_RESET_VAL, PER_WRITE_MASK_VAL, PER_READ_MASK_VAL)
        , PH1D("PH1D", PH1D_OFFSET_VAL, PH1D_RESET_VAL, PH1D_WRITE_MASK_VAL, PH1D_READ_MASK_VAL)
    {
        cout << "PWM constructor" << endl;
        SC_METHOD(reset)
        sensitive << i_rst;
        dont_initialize();

        SC_METHOD(pwm_log);
        sensitive << i_clk.pos();
        dont_initialize();
    }

void PWM::write(const uint32_t& address, const uint32_t& a_value)
{
        // cout << hex << "write addr : " << address << " : a_value : " << a_value << endl;
        switch (address)
        {
        case PID_OFFSET_VAL:
            PID.write(a_value);
            break;
        case PCR_OFFSET_VAL:
            PCR.write(a_value);
            break;
        case CFG_OFFSET_VAL:
            CFG.write(a_value);
            break;
        case START_OFFSET_VAL:
            START.write(a_value);
            break;
        case RPT_OFFSET_VAL:
            RPT.write(a_value);
            break;
        case PER_OFFSET_VAL:
            PER.write(a_value);
            break;
        case PH1D_OFFSET_VAL:
            PH1D.write(a_value);
            break;

        default:
            cout << "Register Not Found" << endl;
    }
}

void PWM::read(const uint32_t& address, uint32_t& a_value)
{
        switch (address)
        {
        case PID_OFFSET_VAL:
            PID.read(a_value);
            break;
        case PCR_OFFSET_VAL:
            PCR.read(a_value);
            break;
        case CFG_OFFSET_VAL:
            CFG.read(a_value);
            break;
        case START_OFFSET_VAL:
            START.read(a_value);
            break;
        case RPT_OFFSET_VAL:
            RPT.read(a_value);
            break;
        case PER_OFFSET_VAL:
            PER.read(a_value);
            break;
        case PH1D_OFFSET_VAL:
            PH1D.read(a_value);
            break;

        default:
            cout << "Register Not Found" << endl;
        }
        //  cout << hex << "read addr : " << address << " : a_value : " << a_value << endl;
}

void PWM::reset()
{
     if (i_rst.read() == 1)
     {
            std::cout << "RESET IS TRIGGERED" << std::endl;
            PID.reset();
            PCR.reset();
            CFG.reset();
            START.reset();
            RPT.reset();
            PER.reset();
            PH1D.reset();

            running = false;
            period_cnt = 0;
            periods_left = 0;
            phase_cnt = 0;
            rpt_cnt = 0;
            last_trigger = false;
     }
}

void PWM::pwm_log()
{
    /* --------pwm_log -------- */
   
        //std::cout << "pwm_log" << endl;
        uint32_t a_cfg = 0, a_start = 0, a_rpt_cnt = 0, a_per = 0, a_ph1d = 0;
        CFG.read(a_cfg); //reading the registers
        RPT.read(a_rpt_cnt);
        START.read(a_start);
        PER.read(a_per);
        PH1D.read(a_ph1d);

        bool INTEN = (a_cfg >> CFG_INTEN_BIT_POS) & CFG_INTEN_BIT_MAKS_VAL;
        bool INACTOUT = (a_cfg >> CFG_INACTOUT_BIT_POS) & CFG_INACTOUT_BIT_MAKS_VAL;
        bool P1OUT = (a_cfg >> CFG_P1OUT_BIT_POS) & CFG_P1OUT_BIT__MAKS_VAL;
        uint32_t EVTRIG = (a_cfg >> CFG_EVTRIG_BIT_POS) & CFG_EVTRIG_BIT_MAKS_VAL;
        uint32_t MODE = (a_cfg >> CFG_MODE_BIT_POS) & CFG_MODE_BIT_MAKS_VAL;

        if (a_start) {
            bool start_ok = true;
            START.write(false);
            pwm_int.write(false);
            period_cnt = 0;

            if (MODE == ONE_SHOT_MODE) {
                periods_left = a_rpt_cnt + 1; // RPT+1 periods
                std::cout << "ONE SHOT MODE" << endl;
                running = true;

            }
            else if (MODE == CONTINUOUS_MODE) {
                periods_left = 0;
                running = true;

            }
            else if (MODE == DISABLE) {
                periods_left = 0;
                running = false;

            }

            if (EVTRIG != 0) {
                bool trig = trigger_event.read();
                start_ok = false;

                if (EVTRIG == 1 && trig && !last_trigger)
                    start_ok = true;
                else if (EVTRIG == 2 && !trig && last_trigger)
                    start_ok = true;

                if (start_ok) {
                    PER.read(a_per);
                    PH1D.read(a_ph1d);
                    RPT.read(a_rpt_cnt);
                    periods_left = 0;
                    period_cnt = 0;
                    phase_cnt = 0;
                    running = true;
                }
            }
        }
        last_trigger = trigger_event.read();
        if (MODE == DISABLE) {
            running = false;
            period_cnt = 0;
            periods_left = 0;
            pwm_out.write(INACTOUT);
            pwm_int.write(false);
            CFG.write(0 << CFG_OPST_BIT_POS);
            return;
        }


        if (!running) {
            pwm_out.write(INACTOUT);
            CFG.write(0 << CFG_OPST_BIT_POS);
            return;
        }

        //std::cout << "pwm_log middle " << endl;

        const uint32_t per_plus_1 = (a_per + 1);
        const uint32_t ph1d_ticks = a_ph1d;

        const bool first_phase = (period_cnt < ph1d_ticks);
        const bool level = first_phase ? (bool)P1OUT : (bool)!P1OUT;
        pwm_out.write(level);
     
        period_cnt = period_cnt + 1;
        if (period_cnt >= per_plus_1) {
            period_cnt = 0;

            if (MODE == ONE_SHOT_MODE) {
                CFG.write(1 << CFG_OPST_BIT_POS | a_cfg);
                if (periods_left > 0) 
                    periods_left = periods_left - 1;

                if (periods_left == 0) {
                    running = false;
                    pwm_out.write(INACTOUT);

                    if (INTEN)
                        pwm_int.write(true);
                }
            }

            // Continuous mode
            if (MODE == CONTINUOUS_MODE) {
                CFG.write(1 << CFG_OPST_BIT_POS | a_cfg);
                if (INTEN)
                    pwm_int.write(true);
            }
        }
       //std::cout << "pwm_log end" << endl;
}