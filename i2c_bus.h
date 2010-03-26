#include <systemc.h>

class i2c_bus : public sc_module
{
	public:
	sc_in<sc_logic> SDA1;
	sc_in<sc_logic> SCL1;
	sc_in<sc_logic> SDA2;
	sc_in<sc_logic> SCL2;
	
	sc_out<sc_logic> SDA;
	sc_out<sc_logic> SCL;

	private:
	void process();	

	public:
	SC_HAS_PROCESS(i2c_bus);
	i2c_bus(sc_module_name name) : sc_module(name)
	{
		SC_METHOD(process);
			sensitive << SDA1 << SDA2 << SCL1;
	}
};

void i2c_bus::process()
	{
		SDA= SDA1 & SDA2;
		//SCL=SCL1 & SCL2;
		SCL=SCL1;
	}
