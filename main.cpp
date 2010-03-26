#include <systemc.h>
//#include "bus_watch.h"
//#include "clock_generator.h"
//#include "transmit_shift_register.h"
//#include "SCL_reader.h"
//#include "i2c_master_core.h"
#include "i2c_bus.h"
#include "slave_emulator.h"
#include "i2c_master_module.h"
#include "i2cmodule.h"

int sc_main(int argc, char **argv)
{
	//external signals
	sc_clock clock("CLOCK", 10, 0.5, 0.0);
	sc_signal<sc_logic> clk;
	sc_signal<sc_logic> rst;
	sc_signal<sc_logic> rst_slave;
	sc_signal<bool> start_transaction;
	sc_signal<sc_logic> SDA;
	sc_signal<sc_logic> SDA_master;
	sc_signal<sc_logic> SDA_slave;
	sc_signal<sc_logic> SCL;
	sc_signal<sc_logic> SCL_master;
	sc_signal<sc_logic> SCL_slave;
	sc_signal<bool> I2C_mode;
	sc_signal<bool> transaction_type;
	sc_signal<sc_bv<7> > slave_addr;
	sc_signal<sc_lv<8> > data;
	sc_signal<bool> micro_data_valid;			//the micro has a new byte
	sc_signal<bool> master_ready;				//the master is ready for receiving data
	sc_signal<bool> master_data_valid;			//the master has a new byte from the slave
	sc_signal<bool> micro_ready;				//the micro is ready to receive data	
	sc_signal<bool> stop_read;					//ends the transaction (reception)
	
	
	sc_signal<sc_logic> csi2c;
	sc_signal<sc_lv<8> > slave_din;
	sc_signal<sc_lv<8> > aout;
	sc_signal<sc_lv<8> > dout;
	sc_signal<sc_logic> ldreg;
	sc_signal<sc_logic> rdreg;
	sc_signal<sc_logic> sdaio;
	
	
	/*********************
			I2C bus
	*********************/
	
	i2c_bus bus("i2c_bus");
	
	bus.SDA1(SDA_master);
	bus.SDA2(SDA_slave);
	bus.SCL1(SCL_master);
	bus.SCL2(SCL_slave);
	bus.SDA(SDA);
	bus.SCL(SCL);


	/***********************
		the master module
	***********************/
	
	i2c_master_module master("I2C master", 100);
	
	master.clk(clk);
	master.rst(rst);
	master.start_transaction(start_transaction);
	master.SDA(SDA);
	master.SDA_out(SDA_master);
	master.SCL(SCL);
	master.SCL_out(SCL_master);
	master.slave_addr(slave_addr);
	master.data(data);
	master.transaction_type(transaction_type);
	master.micro_ready(micro_ready);
	master.stop_read(stop_read);
	master.micro_data_valid(micro_data_valid);
	master.master_ready(master_ready);
	master.master_data_valid(master_data_valid);
	master.I2C_mode(I2C_mode);
	

	
	/******************
		   slaves
	******************/
	
	//slave_emulator slave("slave");
	//slave(clk, rst, SDA, SCL, SDA_slave, SCL_slave);
	
	i2cmodule i2c_slave("slave i2c");
	//i2c_slave(rst_slave, clk, SDA, SCL, csi2c, slave_din, aout, dout, ldreg, rdreg,
	//		SDA_slave, sdaio); 
	
	i2c_slave.i2crst(rst_slave);
	i2c_slave.clk(clk);
	i2c_slave.sdam_s(SDA);
	i2c_slave.scl(SCL);
	i2c_slave.csi2c(csi2c);
	i2c_slave.din(slave_din);
	i2c_slave.aout(aout);
	i2c_slave.dout(dout);
	i2c_slave.ldreg(ldreg);
	i2c_slave.rdreg(rdreg);
	i2c_slave.sdas_m(SDA_slave);
	i2c_slave.sdaio(sdaio);
	
	//tracing
	sc_trace_file * trace_file;
	trace_file = sc_create_vcd_trace_file("my_trace");
	//sc_trace(trace_file, clk, "clk");
	//sc_trace(trace_file, rst, "rst");
	sc_trace(trace_file, SDA, "SDA");
	sc_trace(trace_file, SCL, "SCL");
	sc_trace(trace_file, SDA_master, "SDA_master");
	sc_trace(trace_file, SCL_master, "SCL_master");
	sc_trace(trace_file, SDA_slave, "SDA_slave");
	sc_trace(trace_file, aout, "aout");
	sc_trace(trace_file, dout, "dout");
	sc_trace(trace_file, ldreg, "ldreg");
	sc_trace(trace_file, rdreg, "rdreg");
	sc_trace(trace_file, sdaio, "sdaio");
	sc_trace(trace_file, data, "data");
	sc_trace(trace_file, master_data_valid, "master_data_valid");
	sc_trace(trace_file, master_ready, "master_ready");
	//sc_trace(trace_file, ACK, "ACK");
	//sc_trace(trace_file, SCL_falling_edge, "SCL_falling_edge");
	//sc_trace(trace_file, SCL_slave, "SCL_slave");
	//sc_trace(trace_file, setup_data, "setup_data");
	//sc_trace(trace_file, remove_data, "remove_data");
	//sc_trace(trace_file, rst_clock_generation, "rst_clock_generation");
	
	/*******************************************	
		I2C slave and master initialization
	*******************************************/
	//clk=(sc_logic)1;
	rst=(sc_logic)1;
	rst_slave=(sc_logic)0;
	sc_start(5);

	//clk=(sc_logic)0;
	rst=(sc_logic)1;
	rst_slave=(sc_logic)0;
	sc_start(5);

	/*******************************************
		Simulation of a write transaction
	*******************************************/

	
	//clk=(sc_logic)1;
	clk=(sc_logic)clock;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	I2C_mode=0;
	transaction_type=0;
	start_transaction=1;
	slave_addr="0011100";
	data="11110000";
	micro_data_valid=1;
	csi2c=(sc_logic)0;
	slave_din="00000000";
	sc_start(5);
	
	//clk=(sc_logic)0;
	clk=(sc_logic)clock;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	I2C_mode=0;
	transaction_type=0;
	start_transaction=1;
	slave_addr="0011100";
	data="11110000";
	micro_data_valid=1;
	csi2c=(sc_logic)0;
	slave_din="00000000";
	sc_start(5);

	micro_data_valid=0;
	start_transaction=0;
	//int j=0;
	/*for (int i=0;i<40000;i++)
	{		
		clk=(sc_logic)1;		
		sc_start(5);		
		clk=(sc_logic)0;
		if(master_ready.read()==1)
		{
			if(j==0)			
				data="10101010";
			else if(j==1)
				data="10000001";
			j=j+1;
			micro_data_valid=1;
		}
		else
			micro_data_valid=0;
		sc_start(5);		
	}*/
	for(int i=0;i<15000;i++)
	{
		//clk=(sc_logic)1;
		clk=(sc_logic)clock;		
		sc_start(5);		
		//clk=(sc_logic)0;
		clk=(sc_logic)clock;
		sc_start(5);
	}
	micro_data_valid=1;
	data="10101010";
	//clk=(sc_logic)1;
	clk=(sc_logic)clock;
	sc_start(5);
	micro_data_valid=0;
	data="10101010";
	//clk=(sc_logic)0;
	clk=(sc_logic)clock;
	sc_start(5);
	for(int i=0;i<10000;i++)
	{
		//clk=(sc_logic)1;	
		clk=(sc_logic)clock;	
		sc_start(5);		
		//clk=(sc_logic)0;
		clk=(sc_logic)clock;
		sc_start(5);
	}
	micro_data_valid=1;
	data="10000001";
	//clk=(sc_logic)1;
	clk=(sc_logic)clock;
	sc_start(5);
	micro_data_valid=0;
	data="10000001";
	//clk=(sc_logic)0;
	clk=(sc_logic)clock;
	sc_start(5);
	/*clk=(sc_logic)1;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	I2C_mode=1;
	transaction_type=0;
	start_transaction=1;
	slave_addr="0011100";
	data="11010100";
	micro_data_valid=1;
	csi2c=(sc_logic)0;
	slave_din="00000000";
	sc_start(5);
	
	clk=(sc_logic)0;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	I2C_mode=1;
	transaction_type=0;
	start_transaction=1;
	slave_addr="0011100";
	data="11010100";
	micro_data_valid=1;
	csi2c=(sc_logic)0;
	slave_din="00000000";
	sc_start(5);
	
	micro_data_valid=0;
	start_transaction=0;*/
	
	
	/****************************************
		Simulation of a read transaction
	****************************************/
	
	/*
	clk=(sc_logic)1;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	I2C_mode=0;
	transaction_type=1;
	start_transaction=1;
	slave_addr="0011100";
	data="00000000";
	micro_data_valid=0;
	micro_ready=1;
	stop_read=0;
	csi2c=(sc_logic)0;
	slave_din="11001010";
	sc_start(5);
	
	clk=(sc_logic)0;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	I2C_mode=0;
	transaction_type=1;
	start_transaction=1;
	slave_addr="0011100";
	data="00000000";
	micro_data_valid=0;
	micro_ready=1;
	stop_read=0;
	csi2c=(sc_logic)0;
	slave_din="11001010";
	sc_start(5);
	stop_read=1;
	start_transaction=0;
	*/
	for (int i=0;i<20000;i++)
	{
		//clk=(sc_logic)1;
		clk=(sc_logic)clock;
		sc_start(5);
		//clk=(sc_logic)0;
		clk=(sc_logic)clock;
		sc_start(5);
	}
	
	
	sc_close_vcd_trace_file(trace_file);
	return 0;
}

