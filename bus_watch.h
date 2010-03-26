#include <systemc.h>

class bus_watch : public sc_module
{
	public:
	sc_in<sc_logic> clk;					//the I2C master's internal clock
	sc_in<sc_logic> rst;
	sc_in<sc_logic> SDA;					//the I2C bus SDA
	sc_in<sc_logic> SCL;					//the I2C bus SCL
	sc_in<sc_logic> SDA_in;					//the internal sent data (master transmitter)
	sc_in<sc_logic> SCL_in;					//the internal generated SCL
	sc_in<bool> master_state;
	sc_in<bool> I2C_mode;
	
	sc_out<bool> bus_free;				//the I2C bus is free
	sc_out<bool> lost_arbitration;		//data on the bus isn't the same as sent data
	sc_out<bool> SCL_delayed;			//the slave has delayed the transfer
	
	private:
		enum states {IDLE, BUS_FREE, BUS_BUSY, STOP_DETECTED};
		sc_signal<states> state;
		
		sc_signal<bool> start_condition;
		sc_signal<bool> stop_condition;
		sc_signal<sc_logic> prev_SDA;

		sc_signal<int> stop_count;
		sc_signal<int> SCL_cycle_count;
		int max_stop_count;
		int SCL_cycle;

		const int clk_freq;
	
	private:
	void compute_state();
	void set_output();
	void track_start_stop_condition();
	void set_max_counters();
	
	public:
		SC_HAS_PROCESS(bus_watch);
		bus_watch(sc_module_name name, int clk_f) :
				sc_module(name), clk_freq(clk_f)
		{
			SC_METHOD(compute_state);
				sensitive << clk.pos();
			SC_METHOD(set_output);
				sensitive << state;
			SC_METHOD(track_start_stop_condition);
				sensitive << clk.pos();
			SC_METHOD(set_max_counters);
				sensitive << I2C_mode;
		}		
};

void bus_watch::track_start_stop_condition()
{
	if(rst==(sc_logic)1)
	{
		stop_condition=0;
		start_condition=0;
		prev_SDA=(sc_logic)0;
	}
	else
	{
		stop_condition=0;
		start_condition=0;
		if((SCL==(sc_logic)1) && (prev_SDA==(sc_logic)0) && (SDA==(sc_logic)1))
		{
			stop_condition=1;
		}
		else if((SCL==(sc_logic)1) && (prev_SDA==(sc_logic)1) && (SDA==(sc_logic)0))
		{
			start_condition=1;
		}
		prev_SDA=SDA;
	}
}


void bus_watch::compute_state()
{
	lost_arbitration=0;
	SCL_delayed=0;
	if(rst==(sc_logic)1)
	{
		state=IDLE;
		stop_count=0;
		SCL_cycle_count=0;
	}
	else
	{
		stop_count=stop_count;
		SCL_cycle_count=SCL_cycle_count;
		switch(state)
		{
			case IDLE: if((SCL==(sc_logic)0) || (SDA==(sc_logic)0))
					   {
						   state=BUS_BUSY;
						   //printf("\nbus_state=bus_busy");
					   }
					   else
					   {
						   SCL_cycle_count=SCL_cycle_count+1;
						   if(SCL_cycle_count==SCL_cycle)
						   {
							   state=BUS_FREE;
							   //printf("\nbus_state=bus_free");
							   SCL_cycle_count=0;
						   }
					   }
			break;
			case BUS_BUSY: if(stop_condition==1)
						   {
							   state=STOP_DETECTED;
							   //printf("\nbus_state=stop_detected");
							   stop_count=0;
						   }
			break;
			case STOP_DETECTED: stop_count=stop_count+1;
								if(stop_count==max_stop_count)
								{
									stop_count=0;
									state=BUS_FREE;
									//printf("\nbus_state=bus_free");
								}
			break;
			case BUS_FREE: if(start_condition==1)
						   {
							   state=BUS_BUSY;
							   //printf("\nbus_state=bus_busy");
						   }
			break;
		}
	}
}

void bus_watch::set_output()
{
	switch(state)
	{
		case IDLE: bus_free=0;
		break;
		case BUS_BUSY: bus_free=0;
		break;
		case STOP_DETECTED: bus_free=0;
		break;
		case BUS_FREE: bus_free=1;
		break;
	}
}

void bus_watch::set_max_counters()
{
	if(I2C_mode==0)
	{
		max_stop_count = clk_freq * 4.7;
		SCL_cycle = clk_freq * 10;
	}
	else
	{
		max_stop_count = clk_freq * 4.7;
		SCL_cycle = clk_freq * 10;
	}
}
