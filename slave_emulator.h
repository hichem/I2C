#include <systemc.h>

class slave_emulator : public sc_module
{
	public:
	sc_in<bool> clk;
	sc_in<bool> rst;
	
	sc_in<bool> SDA;
	sc_in<bool> SCL;
	
	sc_out<bool> SDA_out;
	sc_out<bool> SCL_out;
	
	private:
	enum states {idle, ack_1, ack_0};
	sc_signal<states> state;
	sc_signal<bool> send;
	
	sc_signal<bool> start_condition;
	sc_signal<bool> prev_SDA;
	sc_signal<bool> prev_SCL;
	sc_bv<8> data_reg;
	
	sc_signal<int> count;
	
	sc_signal<int> ack_count;
		
	void read();
	void track_start_condition();
	void ack_counter();
	void compute_state();
	void set_output();
	
	public:
	SC_HAS_PROCESS(slave_emulator);
	slave_emulator(sc_module_name name) : sc_module(name)
	{
		SC_METHOD(read);
			sensitive << clk.pos();
		SC_METHOD(track_start_condition);
			sensitive << clk.pos();
		SC_METHOD(ack_counter);
			sensitive << clk.pos();
		SC_METHOD(compute_state);
			sensitive << clk.pos();
		SC_METHOD(set_output);
			sensitive << state;
	}
};

void slave_emulator::read()
{
	if(rst==1)
	{
		data_reg="11111111";
		SCL_out=1;
		send=0;
		count=0;
		prev_SCL=1;
	}
	else
	{
		SCL_out=1;
		send=0;
		if(start_condition==1)
		{
			data_reg="11111111";
			count=0;
		}			
		else if((prev_SCL==0) && (SCL==1))		//a positive front edge of SCL
		{
			for(int i=0;i<7;i++)
				data_reg[i]=data_reg[i+1];
			data_reg[7]=(int)SDA;
			if(count==8)
			{				
				count=0;			
			}
			else if(count==7)
			{
				send=1;
				count=count+1;
			}
			else
				count=count+1;
		}
		else
			count=count;
		prev_SCL=SCL;
	}
}

void slave_emulator::compute_state()
{
	if(rst==1)
	{
		state=idle;
	}
	else
	{
		switch(state)
		{
			case idle: if(send==1)
						{
							state=ack_1;
							//printf("\nslave_emulator=ack_1");
						}
			break;
			case ack_1: if(ack_count==745)		//SCL high time (4) + data setup time (4.55)
							{
								state=ack_0;							
								//printf("\nslave_emulator=ack_0");
							}
			break;
			case ack_0: if(ack_count==1614)
						{
							state=idle;							
							//printf("\nslave_emulator=idle");
						}
			break;
		}
	}
}

void slave_emulator::set_output()
{
	switch(state)
	{
		case idle: SDA_out=1;
		break;
		case ack_1:  SDA_out=1;
		break;
		case ack_0: SDA_out=0;
		break;
	}
}

void slave_emulator::ack_counter()
{
	if(send==1)
	{
		ack_count=0;
	}
	else
	{
		if(ack_count==1614)		//2*SCL high time (8 ns) + SCL low time (4.7) + data hold time (3.45)
			ack_count=0;
		else
			ack_count=ack_count+1;
	}
}

void slave_emulator::track_start_condition()
{
	if(rst==1)
	{
		start_condition=0;
		prev_SDA=SDA;
	}
	else
	{
		start_condition=0;
		if((SCL==1) && (prev_SDA==1) && (SDA==0))
		{
			start_condition=1;
			//printf("\ndetected start condition (slave)");
		}
		prev_SDA=SDA;
	}
}
