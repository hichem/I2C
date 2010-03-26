#include <systemc.h>

class transmit_shift_register : public sc_module
{
	public:
	sc_in<sc_logic> clk;
	sc_in<sc_logic> rst;
	sc_in<bool> launch;
	sc_in<sc_lv<8> > data_in;
	sc_in<sc_bv<2> > flag;
	sc_in<bool> setup_data;		//enable data setup on SDA
	sc_in<bool> remove_data;	//end of data setup on SDA
	sc_in<bool> I2C_mode;
	sc_in<bool> SCL_falling_edge;
	sc_in<bool> send_ack;
	
	sc_out<sc_logic> SDA;
	sc_out<bool> start_condition_end;	//means the SCL hold time after the start condition is over 
	sc_out<bool> byte_loaded;			//means that the content of the data reg has been loaded into the shift reg
	sc_out<bool> transaction_terminated;//the stop condition is over	
		
	private:
	enum states {idle, ready, start, load, block, shift, restart_setup, stop};
	sc_signal<states> state;
	sc_lv<8> reg;
	sc_signal<int> count;
	
	sc_signal<bool> rst_setup_counter;
	sc_signal<bool> valid_setup_counter;
	sc_signal<int> count_setup_counter;
	int max_setup_count;
	
	sc_signal<bool> rst_stop_counter;
	sc_signal<bool> valid_stop_counter;
	sc_signal<int> count_stop_counter;
	int max_stop_count;
	
	const int clk_freq;
	
	void compute_state();
	void set_output();
	void restart_setup_counter();
	void stop_counter();
	void set_max_count();
	
	public:
		SC_HAS_PROCESS(transmit_shift_register);
		transmit_shift_register(sc_module_name name, int clk_f) :
				sc_module(name), clk_freq(clk_f)
		{
			SC_METHOD(compute_state);
				sensitive << clk.pos();
			SC_METHOD(set_output);
				sensitive << state;
			SC_METHOD(restart_setup_counter);
				sensitive << clk.pos();
			SC_METHOD(stop_counter);
				sensitive << clk.pos();	
			SC_METHOD(set_max_count);
				sensitive << I2C_mode;
		}
};

void transmit_shift_register::compute_state()
{
	if(rst==(sc_logic)1)
	{	
		state=idle;
		start_condition_end=0;
		transaction_terminated=0;
		rst_setup_counter=1;
		rst_stop_counter=1;	
		count=0;	
		//printf("\nshift_reg=idle");
	}
	else
	{
		rst_setup_counter=0;
		rst_stop_counter=0;
		start_condition_end=0;
		transaction_terminated=0;
		count=count;
		switch(state)
		{
			case idle: if(launch==1)
						{
							state=ready;
							//printf("\nshift_reg=ready");
						}
			break;
			case ready: if(flag.read()=="00")
				   		{
						   	state=load;
							//printf("\nshift_reg=load");
					    }
				  		else if(flag.read()=="01")	//start a new transaction
					    {
							state=start;
							//printf("\nshift_reg=start");
				   		}
				   		else if(flag.read()=="10")
				   		{
					   		state=restart_setup;
					   		rst_setup_counter=1;
					   		//printf("\nshift_reg=restart_setup");
				   		}
				   		else
				   		{
				   			state=stop;
							rst_stop_counter=1;
							//printf("\nshift_reg=stop");
				   		}
			break;
			case start: if(setup_data==1)
				    	{
				    		state=load;
							start_condition_end=1;
				    		//printf("\nshift_reg=load");
				    	}
			break;
			
			case load: state=block;
				   	   //printf("\nshift_reg=block");
			break;			
			
			case block: if(remove_data==1)
						{	
							//cout << reg << endl;						
							state=shift;
							//printf("\nshift_reg=shift");
							if(count==8)
							{
								count=0;
								state=ready;
							}
							else if(count==7)
							{
								count=count+1;
								if(send_ack==1)
									reg[6]=0;
							}
							else
							{
								count=count+1;
							}
						}
						/*else if(SCL_falling_edge==1)
						{
							state=ready;
							//printf("\nshift_reg=ready");
						}*/
			break;
			
			case shift: state=block;
						//printf("\nshift_reg=block");
			break;

			case restart_setup: if(valid_setup_counter==1)
				    			{
									state=start;
									//printf("\nshift_reg=start");
				    			}
			break;
			
			case stop: if(valid_stop_counter==1)
				   		{
							state=idle;
							transaction_terminated=1;
							//printf("\nshift_reg=idle");
				  		 }
			break;
		}
	}
}

void transmit_shift_register::set_output()
{
	byte_loaded=0;
	switch(state)
	{
		case idle: SDA=(sc_logic)1;
		break;
		case ready: SDA=(sc_logic)1;
		break;
		case load: reg=data_in;				   
				   byte_loaded=1;
			   	   SDA=(sc_logic)1;
		break;
		case start: SDA=(sc_logic)0;
		break;
		case block: SDA=(sc_logic)reg[7].to_bool();
		break;
		case shift: SDA=(sc_logic)reg[7].to_bool();
			    	for(int i=6;i>=0;i--)
						reg[i+1]=reg[i];
			    	reg[0]=(sc_logic)1;
		break;
		case restart_setup: SDA=(sc_logic)1;
		break;
		case stop: SDA=(sc_logic)0;
		break;
	}
}

void transmit_shift_register::set_max_count()
{
	if(I2C_mode==0)
	{
		max_setup_count=clk_freq * 4.7;
		max_stop_count=clk_freq * 4;
	}
	else
	{
		max_setup_count=clk_freq * 0.6;
		max_stop_count=clk_freq * 0.6;
	}
}

void transmit_shift_register::restart_setup_counter()
{
	if(rst_setup_counter==1)
	{
		valid_setup_counter=0;
		count_setup_counter=0;
	}
	else
	{
		if(count_setup_counter==max_setup_count-1)
		{
			count_setup_counter=0;
			valid_setup_counter=1;
		}
		else
		{
			count_setup_counter=count_setup_counter+1;
			valid_setup_counter=0;
		}
	}
}

void transmit_shift_register::stop_counter()
{
	if(rst_stop_counter==1)
	{
		valid_stop_counter=0;
		count_stop_counter=0;
	}
	else
	{
		if(count_stop_counter==max_stop_count-1)
		{
			count_stop_counter=0;
			valid_stop_counter=1;
		}
		else
		{
			count_stop_counter=count_stop_counter+1;
			valid_stop_counter=0;
		}
	}
}

