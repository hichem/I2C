#include <systemc.h>

class i2c_master_core : public sc_module
{
	public:
		sc_in<sc_logic> clk;
		sc_in<sc_logic> rst;
		sc_in<bool> start_transaction;
		sc_in<bool> transaction_type;		//master sending(1) or receiving(0) data
		sc_in<sc_bv<7> > slave_addr;
		sc_inout<sc_lv<8> > data_inout;
		sc_in<bool> micro_data_valid;				//means that there is a new byte
		sc_in<bool> byte_loaded;			//means that the byte in the data_reg has been loaded into the shift register
		sc_in<bool> bus_free;				//the I2C bus is free
		sc_in<sc_logic> ACK;					//the slave has acknowledged the sent byte
		sc_in<bool> start_condition_end;	//means that SCL hold time after the start condition is finished
		sc_in<bool> transaction_terminated;	//means that the stop condition is over
		sc_in<bool> lost_arbitration;
		sc_in<bool> SCL_falling_edge;
		sc_in<bool> stop_read;
		sc_in<bool> micro_ready;
		sc_in<sc_lv<8> > data_from_reader;
		sc_in<bool> data_valid_from_reader;
		
		sc_out<bool> launch_shift_register;			//launch the shift register
		sc_out<bool> enable_clock_generator;		//launch the clock generator
		sc_out<bool> rst_clock_generation;			//reset the clock generator
		sc_out<bool> master_ready;					//the master is ready to receive data from outside
		sc_out<bool> send_ack;
		sc_out<bool> master_data_valid;				//the I2C master has a new byte from the slave
		sc_out<bool> master_state;				//master transmitter "0" or receiver"1"
		sc_out<sc_lv<8> > data_out;			//data to shift register
		sc_out<sc_bv<2> > flag;		/* 00   send a byte
									   01	begin a new transaction (start bit)
									   10	start another transaction (without 
									   		loosing the bus control)
									   11	stop the transaction and make the 
									   		bus free
									*/
	private:
		enum states {idle, wait_on_bus, start, send_byte, receive_byte, restart, stop};
		sc_signal<states> state;
		
		//registers to save the slave address and the data
		sc_bv<8> addr_reg;
		sc_lv<8> data_reg;
		sc_signal<bool> data_reg_change;
		sc_signal<bool> transaction_reg;
		sc_signal<bool> master_ready_reg;
		sc_signal<bool> master_data_valid_reg;
		
		void compute_state();
		void set_output();
		void reg_control();
		
	public:
		SC_HAS_PROCESS(i2c_master_core);
		i2c_master_core(sc_module_name name) : sc_module(name)
		{
			SC_METHOD(compute_state);
				sensitive << clk.pos();
			SC_METHOD(set_output);
				sensitive << state << transaction_reg << data_reg_change;
			SC_METHOD(reg_control);
				sensitive << clk.pos();
		}
};

void i2c_master_core::compute_state()
{			
	transaction_reg=transaction_reg;
	addr_reg=addr_reg;
	if(rst==(sc_logic)1)
	{
		state=idle;	
		addr_reg="0000000";				
		transaction_reg=1;		
		enable_clock_generator=0;
		rst_clock_generation=0;
		printf("\ncore_state=idle");
	}
	else
	{
		enable_clock_generator=0;
		rst_clock_generation=0;
		switch(state)
		{
			case idle: if(start_transaction==1)
						{
							state=wait_on_bus;
							transaction_reg=transaction_type;
							for(int i=0;i<7;i++)
									addr_reg[i+1]=(slave_addr.read())[i];
							if(transaction_type==0)
							{								
								addr_reg[0]=0;																
							}
							else
							{
								addr_reg[0]=1;
							}
							//enable_clock_generator=1;
							printf("\ncore_state=wait_on_bus");
						}
			break;
			case wait_on_bus: if(bus_free==1)
							  {
							  	state=start;
								enable_clock_generator=1;
								printf("\ncore_state=start");
							  }
			break;
			case start: if(SCL_falling_edge==1)
						{
							if(ACK==(sc_logic)0)
							{
								if(transaction_reg==0)
								{
									state=send_byte;
									printf("\ncore_state=send_byte1");
								}
								else
								{
									state=receive_byte;
									printf("\ncore_state=receive byte");
								}
							}
							else
							{
								state=restart;
								rst_clock_generation=1;
								printf("\ncore_state=restart1");
							}
						}
			break;
			case send_byte: if(SCL_falling_edge==1)
							{
								if((ACK==(sc_logic)1) || (master_ready_reg==1))
								{
									state=stop;
									rst_clock_generation=1;
									printf("\ncore_state=stop");
								}
								else				//the slave acknowledged the data byte
								{
									state=send_byte;
									printf("\ncore_state=send_byte2");
								}
							}
			break;
			case receive_byte: if(SCL_falling_edge==1)
								{
									if(ACK==(sc_logic)1)
									{
										state=stop;
										printf("\ncore_state=stop");
									}
									else
									{
										state=receive_byte;
										printf("\ncore_state=receive byte");
									}			
								}
			break;
			case restart:	if(start_condition_end==1)
							{
								enable_clock_generator=1;
							}
							if(SCL_falling_edge==1)
							{
								if(ACK==(sc_logic)0)
								{
									state=send_byte;
									printf("\ncore_state=send_byte3");
								}
								else
								{
									rst_clock_generation=1;
									state=stop;
									printf("\ncore_state=stop");
								}
							}
			break;
			case stop: if(transaction_terminated==1)
						{
							state=idle;
							printf("\ncore_state=idle");
						}
			break;
		}		
	}
}

void i2c_master_core::set_output()
{
	flag="00";
	launch_shift_register=0;
	data_out="11111111";
	send_ack=0;
	switch(state)
	{
		case idle:
		break;
		case wait_on_bus:
		break;
		case start:	flag="01";					//sending the addresss
					launch_shift_register=1;
					data_out=addr_reg;
		break;
		case send_byte: flag="00";					//sending the databytes
						data_out=data_reg;	
						cout << data_reg << endl;				
		break;
		case receive_byte: if((transaction_reg==1) && (stop_read==0))
							{
								send_ack=1;
								data_out="11111111";
							}
		break;
		case restart: flag="10";					//resending the address
			      	  data_out=addr_reg;
		break;
		case stop: flag="11";						//generating the stop condition
		break;
	}
}

void i2c_master_core::reg_control()
{		
	master_data_valid_reg=master_data_valid_reg;
	master_ready_reg=master_ready_reg;
	if(rst==(sc_logic)1)
	{		
		data_reg="11111111";
		data_reg_change=0;
		master_ready_reg=1;
		master_ready=1;
		master_data_valid_reg=0;
	}
	else
	{
		data_reg=data_reg;
		data_reg_change=0;
		master_ready= master_ready_reg;
		master_data_valid=master_data_valid_reg;
		if((transaction_type==0) && (micro_data_valid==1) && (master_ready_reg==1))
		{								//the data processing unit has delivered a new byte
			data_reg=data_inout.read();
			data_reg_change=1;
			//printf("\ndata=%d",(int)data_inout.read());
			//cout << data_reg << endl;
			master_ready_reg=0;
			master_ready=0;
		}
		/*if(state==idle)
		{
			master_ready_reg=1;
			master_ready=1;
		}*/
		if((state==send_byte) && (byte_loaded==1))	//the byte in the data reg is loaded into the shift register
		{
			master_ready_reg=1;
			master_ready=1;
		}
		if((transaction_reg==1) && (data_valid_from_reader==1) 
				&& (master_data_valid_reg==0) && (state==receive_byte))
		{											//the reading unit got a new byte
			data_reg=data_from_reader;
			master_data_valid_reg=1;
			master_data_valid=1;
		}
		if(master_data_valid_reg==1)								//output the slave's new byte
		{
			data_inout=data_reg;
		}
		if((master_data_valid_reg==1) && (micro_ready==1))				//the data processing unit (micro)
		{																//got the slave's byte
			master_data_valid_reg=0;
		}
	}
}
