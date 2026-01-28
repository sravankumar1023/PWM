// Code your design here.
// Uncomment the next line for SystemC modules.
#include <systemc>
#include <cstdint>

//OFFSET VALES
#define PID_OFFSET_VAL 0x00
#define PCR_OFFSET_VAL 0x04
#define CFG_OFFSET_VAL 0x08
#define START_OFFSET_VAL 0x0C
#define RPT_OFFSET_VAL 0x10
#define PER_OFFSET_VAL 0x14
#define PH1D_OFFSET_VAL 0x18

//reset values
#define PID_RESET_VAL 0x42071100
#define PCR_RESET_VAL 0x00
#define CFG_RESET_VAL 0x00
#define START_RESET_VAL 0x00
#define RPT_RESET_VAL 0x00
#define PER_RESET_VAL 0x00
#define PH1D_RESET_VAL 0x00

//WRITE mask values
#define PID_WRITE_MASK_VAL 0x00
#define PCR_WRITE_MASK_VAL 0x01
#define CFG_WRITE_MASK_VAL 0x7F
#define START_WRITE_MASK_VAL 0x01
#define RPT_WRITE_MASK_VAL 0xFF
#define PER_WRITE_MASK_VAL 0xFFFFFFFF
#define PH1D_WRITE_MASK_VAL 0xFFFFFFFF

//READ mask values
#define PID_READ_MASK_VAL 0x42071100
#define PCR_READ_MASK_VAL 0x01
#define CFG_READ_MASK_VAL 0x3007F
#define START_READ_MASK_VAL 0x00
#define RPT_READ_MASK_VAL 0xFF
#define PER_READ_MASK_VAL 0xFFFFFFFF
#define PH1D_READ_MASK_VAL 0xFFFFFFFF


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
  PWM_registers(string a_name, uint32_t a_offset, uint32_t a_reset_value, uint32_t a_write_mask, uint32_t a_read_mask)
    : name(a_name)
  	, m_reset_value(a_reset_value)
    , m_offset(a_offset)
    , m_write_mask(a_write_mask)
    , m_read_mask(a_read_mask)
   {
    //cout << "PWM_reg constructor" << endl;
    m_value = m_reset_value;
   }
  
   void write(const uint32_t &a_value)
   {
     m_value &=~m_write_mask;
     m_value |= (a_value & m_write_mask);
   }

   void read(uint32_t &a_value)
   {
     a_value = m_value & m_read_mask;
   }
  
	void reset()
    {
	  m_value = m_reset_value; 
    }
};

class PWM: public sc_module
{
  PWM_registers PID;  //offset/address 00H
  PWM_registers PCR;  //offset/address 04H
  PWM_registers CFG;  //offset/address 08H
  PWM_registers START;//offset/address 0CH
  PWM_registers RPT;  //offset/address 10H
  PWM_registers PER;  //offset/address 14H
  PWM_registers PH1D; //offset/address 18H
  
  /* ---------- Internal Purpose ---------- */
  sc_uint<32> period_cnt;
  sc_uint<32> periods_left;
  sc_uint<32> phase_cnt;
  sc_uint<8>  rpt_cnt;
   
  public: 
  sc_in<bool> i_rst; 
  sc_in<bool> i_clk;
  sc_in<bool> trigger_event; // VPIF & GPIO event trigger
  
  sc_out<bool> pwm_out;
  sc_out<bool> pwm_int;
  
  bool running;
  bool last_trigger;
  
  
  
  enum pwm_state { DISABLE=0, ONE_SHOT_MODE=1, CONTINUOUS_MODE=2 };
  SC_HAS_PROCESS(PWM);
  PWM(sc_module_name name):sc_module(name)
   , PID("PID",     PID_OFFSET_VAL, PID_RESET_VAL, PID_WRITE_MASK_VAL, PID_READ_MASK_VAL)
   , PCR("PCR",     PCR_OFFSET_VAL, PCR_RESET_VAL, PCR_WRITE_MASK_VAL, PCR_READ_MASK_VAL)
   , CFG("CFG",     CFG_OFFSET_VAL, CFG_RESET_VAL, CFG_WRITE_MASK_VAL, CFG_READ_MASK_VAL)
   , START("START", START_OFFSET_VAL, START_RESET_VAL, START_WRITE_MASK_VAL, START_READ_MASK_VAL)
   , RPT("RPT",     RPT_OFFSET_VAL, RPT_RESET_VAL, RPT_WRITE_MASK_VAL, RPT_READ_MASK_VAL)
   , PER("PER",     PER_OFFSET_VAL, PER_RESET_VAL, PER_WRITE_MASK_VAL, PER_READ_MASK_VAL)  
   , PH1D("PH1D",   PH1D_OFFSET_VAL, PH1D_RESET_VAL, PH1D_WRITE_MASK_VAL, PH1D_READ_MASK_VAL)  
  {
     cout << "PWM constructor" << endl;
     SC_METHOD(reset)
     sensitive << i_rst;
     dont_initialize();
     
     SC_THREAD(pwm_log);
     sensitive << i_clk.pos();
     //dont_initialize();
  }
  
   void write(const uint32_t & address,const uint32_t &a_value)
   {
    // cout << hex << "write addr : " << address << " : a_value : " << a_value << endl;
     switch(address)
     {
       case 0x00 :  
       				PID.write(a_value);
     				break;
       case 0x04 :  
       				PCR.write(a_value);
     				break;
       case 0x08 :  
       				CFG.write(a_value);
     				break;
       case 0x0C :  
       				START.write(a_value);
     				break;
       case 0x10 :
       				RPT.write(a_value);
     				break;   
       case 0x14 :
       				PER.write(a_value);
     				break;   
       case 0x18 :
 			        PH1D.write(a_value);
     				break; 
  		
       default :
       			cout << "Register Not Found" << endl;
     }
   }
  
   void read(const uint32_t & address,uint32_t &a_value)
   {
      switch(address)
     {
       case 0x00 :  
       				PID.read(a_value);
     				break;
       case 0x04 :  
       				PCR.read(a_value);
     				break;
       case 0x08 :  
       				CFG.read(a_value);
     				break;
       case 0x0C :  
       				START.read(a_value);
     				break;
       case 0x10 :
       				RPT.read(a_value);
     				break;   
       case 0x14 :
       				PER.read(a_value);
     				break;   
       case 0x18 :
 			        PH1D.read(a_value);
     				break; 
  		
       default :
       			cout << "Register Not Found" << endl;
     }
     
   //  cout << hex << "read addr : " << address << " : a_value : " << a_value << endl;
   }
	
   void reset()
   {
     if(i_rst.read() == 1)
     {
         std::cout << "RESET IS TRIGGERED"<< std::endl;
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
   	     phase_cnt  = 0;
         rpt_cnt    = 0;
     	 last_trigger = false;
     }
   }
  
   void pwm_log()
   {
      /* --------pwm_log -------- */
     while(1)
     {
      wait();
      std::cout<< "pwm_log" << endl;
      uint32_t a_cfg = 0, a_start = 0, a_rpt_cnt=0, a_per=0, a_ph1d=0;
      CFG.read(a_cfg); //reading the registers
      RPT.read(a_rpt_cnt);
      START.read(a_start);
      PER.read(a_per);
      PH1D.read(a_ph1d);
     
      bool INTEN = (a_cfg>>6) & 0x1;
      bool INACTOUT = (a_cfg>>5) & 0x1;
      bool P1OUT = (a_cfg>>4) & 0x1;
      uint32_t EVTRIG = (a_cfg>>2) &0x03;
  	  uint32_t MODE = a_cfg  & 0x03; 
      
     if (a_start & !running){
         bool start_ok = true;
   		 START.write(false);
         pwm_int.write(false);
         period_cnt = 0;
        
         if (MODE == ONE_SHOT_MODE) {
            periods_left = a_rpt_cnt+1; // RPT+1 periods
            std::cout<< "ONE SHOT MODE" << endl;
            running = true;
           
         } else if (MODE == CONTINUOUS_MODE) {
            periods_left = 0;
            running = true;
            
         } else if (MODE == DISABLE){
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
               	period_cnt  = 0;
               	phase_cnt   = 0;
               	running     = true;
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
          CFG.write(0<<17);
          return;
     }
     

     if (!running) {
         pwm_out.write(INACTOUT);
         CFG.write(0<<17);
         return;
     }
     
     std::cout<< "pwm_log middle " << endl;
     
     const sc_uint<32> per_plus_1 = (sc_uint<32>)(a_per + 1); 
     const sc_uint<32> ph1d_ticks = a_ph1d;
     
     const bool first_phase = (period_cnt < ph1d_ticks);
     const bool level = first_phase ? (bool)P1OUT : (bool)!P1OUT;
         
      // Output logic
     if (period_cnt < a_ph1d)
          pwm_out.write(level);
     else
          pwm_out.write(!level);
                                        
      period_cnt = period_cnt + 1;
      if (period_cnt >= per_plus_1) {
            period_cnt = 0;

      if (MODE == ONE_SHOT_MODE) {
        	CFG.write(1<<17);
    	    if (periods_left > 0) periods_left = periods_left - 1;

            if (periods_left == 0) {
                  running = false;
                  pwm_out.write(INACTOUT); 
          }
        
          if (INTEN) 
        	 pwm_int.write(true);
      }
        
      // Continuous mode
      if (MODE == CONTINUOUS_MODE) {
         	  CFG.write(1<<17);
              if (INTEN)
                  pwm_int.write(true);
       		}  
     	}
     
     std::cout<< "pwm_log end" << endl;
   	}
  }
};

class Top: public sc_module	
{
  PWM obj_PWM;
  sc_clock i_clk;
  
  sc_signal<bool> i_rst; 
  sc_signal<bool> pwm_out;
  sc_signal<bool> pwm_int;
  sc_signal<bool> trigger_event;
  
  public:   
  SC_HAS_PROCESS(Top);
  Top(sc_module_name name):sc_module(name)
   , obj_PWM("Top")
   , i_clk("clk_10ns", 10, SC_NS,0.5, 0, SC_NS, true)
          //(name, period, unit, duty_cycle, starrt_time, start_time_unit, posedge_first=true)
  {
    obj_PWM.i_rst(i_rst);
    obj_PWM.i_clk(i_clk);
    obj_PWM.trigger_event(trigger_event);
    obj_PWM.pwm_out(pwm_out);
    obj_PWM.pwm_int(pwm_int);
    
    SC_THREAD(tests)
  }
  
  void tests()
  {
    enum MODE { DISABLE=0, ONE_SHOT_MODE=1, CONTINUOUS_MODE=2 };
    enum ADDR { PID=0x00, PCR=0x04, CFG=0x08, START=0x0C, RPT=0x10, PER=0x14, PH1D=0x18 };
   	uint32_t read_val =0, write_val = 0xFFFFFFFF, addr;
    
    cout << "------------------- READ WRITE TEST START -------------------" << endl;
    addr = PID;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0x42071100)
    {
      cout << "PID REG READ WRITE FAILED" << endl;
      sc_stop();
    }
    
    addr = PCR;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0x1)
    {
      cout << "PCR REG READ WRITE FAILED" << endl;
      sc_stop();
    }
    
    
    addr = CFG;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0x7F)
    {
      cout << "CFG REG READ WRITE FAILED" << endl;
      sc_stop();
    }
    
    
    addr = START;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "START REG READ WRITE FAILED" << endl;
      sc_stop();
    }
    
    
    addr = RPT;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0xFF)
    {
      cout << "RPT REG READ WRITE FAILED" << endl;
      sc_stop();
    }
    
    addr = PER;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0xFFFFFFFF)
    {
      cout << "PER REG READ WRITE FAILED" << endl;
      sc_stop();
    }
    
    addr = PH1D;
    obj_PWM.write(addr,write_val);
    obj_PWM.read (addr, read_val);
    if(read_val != 0xFFFFFFFF)
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
    
    addr = PID;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x42071100)
    {
      cout << "PID REG RESET FAILED" << endl;
      sc_stop();
    }
    
    addr = PCR;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "PCR REG RESET FAILED" << endl;
      sc_stop();
    }
    
    addr = CFG;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "CFG REG RESET FAILED" << endl;
      sc_stop();
    }
    
    addr = START;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "START REG RESET FAILED" << endl;
      sc_stop();
    }
    
    addr = RPT;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "RPT REG RESET FAILED" << endl;
      sc_stop();
    }
    
    addr = PER;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "PER REG RESET FAILED" << endl;
      sc_stop();
    }
    
    addr = PH1D;
    obj_PWM.read (addr, read_val);
    if(read_val != 0x00)
    {
      cout << "PH1D REG RESET FAILED" << endl;
      sc_stop();
    }
    wait(10, SC_NS);
    cout << "------------------- RESET TEST PASSED -----------------------" << endl;
    
    cout<<"------------------- ONE SHOT MODE START ---------------------" << endl;
    cout<<"------------------- ONE SHOT MODE CONFIG --------------------" << endl;
    obj_PWM.write(RPT, 2); //RPT
    obj_PWM.write(PER, 9); //PER+1
    obj_PWM.write(PH1D, 3);
    
    obj_PWM.write(CFG, 1<<6 | 0<<5 | 1<<4 | ONE_SHOT_MODE);  //one shot mode, p1out =1, int=1;
    obj_PWM.write(START, 1);
    wait(SC_ZERO_TIME);wait(SC_ZERO_TIME);
    wait(10, SC_NS);
    cout<<"------------------- ONE SHOT MODE PASSED --------------------" << endl;
    std::cout << "ALL TESTS DONE" << std::endl;
    sc_stop();
  }
};


int sc_main(int argc, char *argv[])
{
  Top obj_TOP("Top"); 
  sc_start();
  return 0;
}