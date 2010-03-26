#include <systemc.h>
#include "bus_watch.h"
#include "clock_generator.h"
#include "transmit_shift_register.h"
#include "SCL_reader.h"
#include "i2c_master_core.h"

class i2c_master_module : public sc_module
{
	public:
	sc_in<sc_logic> clk;
	sc_in<sc_logic> rst;
	sc_in<bool> start_transaction;
	
	//Interface with the bus
	sc_in<sc_logic> SDA;
	sc_out<sc_logic> SDA_out;
	sc_in<sc_logic> SCL;
	sc_out<sc_logic> SCL_out;
	
	//Interface with the data processing unit
	sc_in<sc_bv<7> > slave_addr;
	sc_inout<sc_lv<8> > data;
	sc_in<bool> transaction_type;
	sc_in<bool> micro_ready;			//the I2C master is ready to receive the slave's incoming byte
	sc_in<bool> stop_read;				//a signal to end the transaction (reception)
	
	//handshake
	sc_in<bool> micro_data_valid;			//the data processing unit has data for the slave
	sc_out<bool> master_ready;				//the I2C master is ready to get new data
	sc_out<bool> master_data_valid;			//the I2C master has a new byte from the slave
	
	sc_in<bool> I2C_mode;
	
	private:
	
	sc_signal<bool> master_state;		//"0" : master transmitter, "1" : master receiver
	sc_signal<bool> bus_free;
	sc_signal<bool> lost_arbitration;		
	sc_signal<sc_bv<2> > flag;
	sc_signal<bool> setup_data;
	sc_signal<bool> remove_data;	
	sc_signal<bool> SCL_delayed;
	sc_signal<sc_lv<8> > data_to_transmission_unit;
	sc_signal<sc_logic> ACK;
	sc_signal<bool> start_condition_end;
	sc_signal<bool> transaction_terminated;
	sc_signal<bool> launch_shift_register;
	sc_signal<bool> start_clock_generation;
	sc_signal<bool> data_received;
	sc_signal<bool> SCL_falling_edge;	
	sc_signal<bool> rst_clock_generation;
	sc_signal<bool> byte_loaded;
	sc_signal<bool> send_ack;
	sc_signal<sc_lv<8> > data_from_reader;
	sc_signal<bool> data_valid_from_reader;
	
	
	/***************
	     modules
	****************/
	bus_watch  				    *bus_control_unit;
	i2c_master_core 			*control_unit;
	SCL_reader 					*data_read_unit;
	clock_generator 			*clock_generation_unit;
	transmit_shift_register		*transmission_unit;
	
	
	public:
	SC_HAS_PROCESS(i2c_master_module);
	i2c_master_module(sc_module_name name, int clk_f) : sc_module(name)
	{
		//modules' construction
		bus_control_unit = new bus_watch("bus watcher", clk_f);
		control_unit = new i2c_master_core("control unit");
		data_read_unit = new SCL_reader("data read unit");
		clock_generation_unit = new clock_generator("clock generation", clk_f);
		transmission_unit = new transmit_shift_register("transmission_unit", clk_f);
		
		
		/***********************
			Bus control unit
		***********************/		
		//input ports
		bus_control_unit->clk(clk);
		bus_control_unit->rst(rst);
		bus_control_unit->SDA(SDA);
		bus_control_unit->SCL(SCL);
		bus_control_unit->SDA_in(SDA_out);
		bus_control_unit->SCL_in(SCL_out);	
		bus_control_unit->master_state(master_state);
		bus_control_unit->I2C_mode(I2C_mode);
		//output ports					
		bus_control_unit->bus_free(bus_free);
		bus_control_unit->lost_arbitration(lost_arbitration);
		bus_control_unit->SCL_delayed(SCL_delayed);
						
		/***********************
			 Clock generator
		***********************/
		//input ports	
		clock_generation_unit->clk(clk);
		clock_generation_unit->rst(rst);
		clock_generation_unit->rst_core(rst_clock_generation);
		clock_generation_unit->I2C_mode(I2C_mode);
		clock_generation_unit->start(start_clock_generation);
		clock_generation_unit->SCL_delayed(SCL_delayed);		
		//output ports
		clock_generation_unit->SCL(SCL_out);
		clock_generation_unit->SCL_falling_edge(SCL_falling_edge);
		clock_generation_unit->setup_data(setup_data);
		clock_generation_unit->remove_data(remove_data);
						
		/***********************
			 Shift register
		***********************/
		//input ports
		transmission_unit->clk(clk);
		transmission_unit->rst(rst);
		transmission_unit->launch(launch_shift_register);
		transmission_unit->data_in(data_to_transmission_unit);
		transmission_unit->flag(flag);
		transmission_unit->setup_data(setup_data);
		transmission_unit->remove_data(remove_data);
		transmission_unit->I2C_mode(I2C_mode);
		transmission_unit->SCL_falling_edge(SCL_falling_edge);
		transmission_unit->send_ack(send_ack);
		//output ports
		transmission_unit->SDA(SDA_out);
		transmission_unit->start_condition_end(start_condition_end);
		transmission_unit->byte_loaded(byte_loaded);
		transmission_unit->transaction_terminated(transaction_terminated);
		
						
		/***********************
			  control unit
		***********************/
		//input output ports
		control_unit->data_inout(data);
		
		//input ports
		control_unit->clk(clk);
		control_unit->rst(rst);
		control_unit->start_transaction(start_transaction);
		control_unit->transaction_type(transaction_type);
		control_unit->slave_addr(slave_addr);		
		control_unit->byte_loaded(byte_loaded);
		control_unit->bus_free(bus_free);
		control_unit->ACK(ACK);
		control_unit->start_condition_end(start_condition_end);
		control_unit->transaction_terminated(transaction_terminated);
		control_unit->lost_arbitration(lost_arbitration);
		control_unit->SCL_falling_edge(SCL_falling_edge);
		control_unit->stop_read(stop_read);
		control_unit->micro_data_valid(micro_data_valid);
		control_unit->micro_ready(micro_ready);
		control_unit->data_from_reader(data_from_reader);
		control_unit->data_valid_from_reader(data_valid_from_reader);		
		//output ports
		control_unit->launch_shift_register(launch_shift_register);
		control_unit->enable_clock_generator(start_clock_generation);
		control_unit->rst_clock_generation(rst_clock_generation);
		control_unit->master_ready(master_ready);
		control_unit->send_ack(send_ack);
		control_unit->master_data_valid(master_data_valid);
		control_unit->master_state(master_state);
		control_unit->data_out(data_to_transmission_unit);
		control_unit->flag(flag);
			
		/***********************
			 Data read unit
		***********************/
		//input ports
		data_read_unit->clk(clk);
		data_read_unit->rst(rst);
		data_read_unit->SCL(SCL);
		data_read_unit->SDA(SDA);
		//output ports
		data_read_unit->data_out(data_from_reader);
		data_read_unit->ACK(ACK);
		data_read_unit->data_valid(data_valid_from_reader);
	}	
};
