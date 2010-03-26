#include <systemc.h>
#include "i2c_master_module.h"
#include "i2cmodule.h"
#include "i2c_bus.h"
#include "transactor.h"

int sc_main(int argc, char **argv)
{
	sc_signal<sc_logic> clk;
	sc_signal<sc_logic> rst;
	sc_signal<sc_logic> rst_slave;
	
	//I2C interface
	sc_signal<sc_logic> SDA;
	sc_signal<sc_logic> SDA_master;
	sc_signal<sc_logic> SDA_slave;
	sc_signal<sc_logic> SCL;
	sc_signal<sc_logic> SCL_master;
	sc_signal<sc_logic> SCL_slave;
	
	//master RTL interface
	sc_signal<bool> I2C_mode;
	sc_signal<bool> transaction_type;
	sc_signal<bool> start_transaction;
	sc_signal<sc_bv<7> > slave_addr;
	sc_signal<sc_lv<8> > data;
	sc_signal<bool> micro_data_valid;			//the micro has a new byte
	sc_signal<bool> master_ready;				//the master is ready for receiving data
	sc_signal<bool> master_data_valid;			//the master has a new byte from the slave
	sc_signal<bool> micro_ready;				//the micro is ready to receive data	
	sc_signal<bool> stop_read;					//ends the transaction (reception)
	
	//slave RTL interface
	sc_signal<sc_logic> csi2c;
	sc_signal<sc_lv<8> > slave_din;
	sc_signal<sc_lv<8> > aout;
	sc_signal<sc_lv<8> > dout;
	sc_signal<sc_logic> ldreg;
	sc_signal<sc_logic> rdreg;
	sc_signal<sc_logic> sdaio;
	
	//modules
	
	/********************
		  transactor
	********************/
	transactor trans("transactor");
	trans.I2C_mode(I2C_mode);
	trans.transaction_type(transaction_type);
	trans.start_transaction(start_transaction);
	trans.slave_addr(slave_addr);
	trans.data(data);
	trans.micro_data_valid(micro_data_valid);
	trans.master_ready(master_ready);
	trans.master_data_valid(master_data_valid);
	trans.micro_ready(micro_ready);
	trans.stop_read(stop_read);
	
	/********************
	    data generator
	********************/
	data_generator data_gen("data generator");
	data_gen.port(trans);
	
	/********************
		  I2C master
	********************/
	i2c_master_module master("I2C master", 100);
	master.clk(clk);
	master.rst(rst);
	master.I2C_mode(I2C_mode);
	master.start_transaction(start_transaction);
	master.transaction_type(transaction_type);
	master.slave_addr(slave_addr);
	master.data(data);
	master.micro_data_valid(micro_data_valid);
	master.master_ready(master_ready);
	master.master_data_valid(master_data_valid);
	master.micro_ready(micro_ready);
	master.stop_read(stop_read);
	master.SDA(SDA);
	master.SDA_out(SDA_master);
	master.SCL(SCL);
	master.SCL_out(SCL_master);
	
	/********************
		  I2C slave
	********************/	
	i2cmodule slave("I2C slave");
	slave.i2crst(rst_slave);
	slave.clk(clk);
	slave.sdam_s(SDA);
	slave.scl(SCL);
	slave.csi2c(csi2c);
	slave.din(slave_din);
	slave.aout(aout);
	slave.dout(dout);
	slave.ldreg(ldreg);
	slave.rdreg(rdreg);
	slave.sdas_m(SDA_slave);
	slave.sdaio(sdaio);
	
	/********************
		  I2C bus
	********************/
	i2c_bus bus("I2C bus");
	bus.SDA(SDA);
	bus.SDA1(SDA_master);
	bus.SDA2(SDA_slave);
	bus.SCL(SCL);
	bus.SCL1(SCL_master);
	bus.SCL2(SCL_slave);
	
	//tracing
	sc_trace_file * trace_file;
	trace_file = sc_create_vcd_trace_file("my_trace");
		
	sc_trace(trace_file, SDA, "SDA");
	sc_trace(trace_file, SCL, "SCL");
	sc_trace(trace_file, SDA_master, "SDA_master");
	sc_trace(trace_file, SCL_master, "SCL_master");
	sc_trace(trace_file, SDA_slave, "SDA_slave");
	sc_trace(trace_file, master_ready, "master_ready");
	sc_trace(trace_file, data, "data");
	sc_trace(trace_file, aout, "dout");
	
	//Simulation
	csi2c=(sc_logic)0;
	slave_din="00000000";
	
	clk=(sc_logic)1;
	rst=(sc_logic)1;
	rst_slave=(sc_logic)0;
	sc_start(5);
	
	clk=(sc_logic)0;
	rst=(sc_logic)1;
	rst_slave=(sc_logic)0;
	sc_start(5);
	
	clk=(sc_logic)1;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	sc_start(5);
	
	clk=(sc_logic)0;
	rst=(sc_logic)0;
	rst_slave=(sc_logic)1;
	sc_start(5);
	
	for (int i=0;i<250000;i++)
	{
		clk=(sc_logic)1;
		sc_start(5);
		clk=(sc_logic)0;
		sc_start(5);
	}
	
	sc_close_vcd_trace_file(trace_file);
	return 0;
}
