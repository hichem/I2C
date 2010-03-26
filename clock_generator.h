#include <systemc.h>

class clock_generator : public sc_module
{
	public:
		sc_in<sc_logic> clk;
		sc_in<sc_logic> rst;
		sc_in<bool> rst_core;
		sc_in<bool> I2C_mode;
		sc_in<bool> start;
		sc_in<bool> SCL_delayed;
		
		sc_out<sc_logic> SCL;
		sc_out<bool> SCL_falling_edge;
		sc_out<bool> setup_data;	//enable data setup on SDA
		sc_out<bool> remove_data;	//end of data setup on SDA
		
	private:
		enum states {idle, Hold, SCL_LOW, SCL_HIGH};
		sc_signal<states> state;
		
		/*input signals of a 9 counter
		SCL is composed of 9 clock edges:
					- 8 bits data + Ack
					- 7 bits address + bit R/W + Ack
		*/
		
		sc_signal<int> low_count;
		sc_signal<bool> rst_low_counter;
		sc_signal<bool> low_valid;
		sc_signal<bool> low_enable;
		int SCL_low_count;
		
		sc_signal<int> high_count;
		sc_signal<bool> rst_high_counter;
		sc_signal<bool> rst_async_high_counter;
		sc_signal<bool> high_valid;
		int SCL_high_count;
		
		sc_signal<int> SCL_hold_count;
		sc_signal<bool> rst_SCL_hold_counter;
		sc_signal<bool> SCL_hold_valid;
		int max_SCL_hold_count;
		
		sc_signal<int> count;
		sc_signal<bool> rst_SCL_counter;
		
		int data_setup_count;
		int data_hold_count;
		const int clk_freq;
		
	void compute_state();
	void set_output();
	void low_time_counter();
	void high_time_counter();
	void set_count();
	void SCL_hold_counter();
	void SCL_counter();
		
	public:
		SC_HAS_PROCESS(clock_generator);
		clock_generator(sc_module_name name, int clk_f) :
			sc_module(name), clk_freq(clk_f)
		{
			SC_METHOD(compute_state);
				sensitive << clk.pos();
			SC_METHOD(set_output);
				sensitive << state << SCL_delayed;
			SC_METHOD(low_time_counter);
				sensitive << clk.pos();
			SC_METHOD(high_time_counter);
				sensitive << clk.pos() << rst_async_high_counter;
			SC_METHOD(set_count);
				sensitive << I2C_mode;
			SC_METHOD(SCL_hold_counter);
				sensitive << clk.pos();
			SC_METHOD(SCL_counter);
				sensitive << clk.pos();
		}
};

void clock_generator::compute_state()
{
	if((rst==(sc_logic)1) || (rst_core==1))
	{
		state=idle;
		rst_SCL_hold_counter=1;
		rst_low_counter=1;
		rst_high_counter=1;
		rst_SCL_counter=1;
		//printf("\nclock_state=idle");
	}
	else
	{
		rst_SCL_hold_counter=0;
		rst_low_counter=0;
		rst_high_counter=0;
		rst_SCL_counter=0;
		switch(state)
		{
			case idle: if(start==1)
				   	   {
							state=Hold;
							rst_SCL_hold_counter=1;							
							//printf("\nclock_state=hold");
				       }
			break;
			case Hold: if(SCL_hold_valid==1)
			       	   {
							state=SCL_LOW;
							rst_low_counter=1;
							rst_high_counter=1;
							rst_SCL_counter=1;
							//printf("\nclock_state=scl_low");
				   	   }
			break;
			case SCL_LOW: if(low_valid==1)
						  {
								state=SCL_HIGH;
								rst_high_counter=1;
								//printf("\nclock_state=scl_high");
						  }
			break;
			case SCL_HIGH: if(high_valid==1)
				       		{					   		
				       			state=SCL_LOW;
								rst_low_counter=1;
								//printf("\nclock_state=scl_low");
				       		}
			break;
		}
	}
}

void clock_generator::set_output()
{
	rst_async_high_counter=0;
	switch(state)
	{
		case idle: SCL=(sc_logic)1;
		break;
		case Hold: SCL=(sc_logic)1;
		break;
		case SCL_LOW: SCL=(sc_logic)0;
			   		  if(SCL_delayed==1)		//if the slave has delayed the clock
							rst_async_high_counter=1;	//we reset the hight time counter
		break;
		case SCL_HIGH: 
			           SCL=(sc_logic)1;
		break;
	}
}

void clock_generator::set_count()
{
	if(I2C_mode==0)		//standard mode
	{
		SCL_low_count=clk_freq * 4.7;
		SCL_high_count=clk_freq * 4;
		data_setup_count=clk_freq * 4.3;	//4.55
		data_hold_count=clk_freq * 7.45;
		max_SCL_hold_count=clk_freq * 4;
	}
	else				//fast mode
	{
		SCL_low_count=clk_freq * 1.3;
		SCL_high_count=clk_freq * 0.6;
		data_setup_count=clk_freq * 1.2;
		data_hold_count=clk_freq * 1.5;
		max_SCL_hold_count=clk_freq * 0.6;
	}
}

void clock_generator::low_time_counter()
{
	setup_data=0;
	if(rst_low_counter==1)
	{
		low_count=0;
		low_valid=0;
	}
	else
	{
		low_valid=0;
		if(low_count==data_setup_count-1)
		{
			setup_data=1;
			low_count=low_count+1;
		}
		else if(low_count>=SCL_low_count-1)
		{
			low_count=0;
			low_valid=1;
		}
		else
		{
			low_count=low_count+1;
		}
	}
}

void clock_generator::SCL_counter()
{
	SCL_falling_edge=0;
	if(rst_SCL_counter==1)
	{
		count=0;
	}
	else
	{
		if(high_count==data_hold_count-1)
		{
			if(count==8)
			{
				count=0;
				SCL_falling_edge=1;
			}
			else
			{
				count=count+1;
			}
		}
		else
			count=count;
	}
}

void clock_generator::high_time_counter()
{
	remove_data=0;	
	if((rst_high_counter==1) || (rst_async_high_counter==1))
	{
		high_count=0;
		high_valid=0;
	}
	else
	{
		high_valid=0;
		if(high_count==SCL_high_count-1)
		{			
			high_valid=1;
			high_count=high_count+1;							
		}
		else if(high_count>=data_hold_count-1)
		{
			high_count=0;
			remove_data=1;						
		}
		else
		{
			high_count=high_count+1;
		}
	}
}

void clock_generator::SCL_hold_counter()
{
	if(rst_SCL_hold_counter==1)
	{
		SCL_hold_valid=0;
		SCL_hold_count=0;
	}
	else
	{
		if(SCL_hold_count==max_SCL_hold_count-1)
		{
			SCL_hold_valid=1;
			SCL_hold_count=0;
		}
		else
		{
			SCL_hold_count=SCL_hold_count+1;			
			SCL_hold_valid=0;
		}
	}
}
