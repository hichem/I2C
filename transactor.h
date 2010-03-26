#include <systemc.h>

typedef sc_uint<7> addr_t;
typedef sc_uint<8> data_t;

class rw_if : virtual public sc_interface
{
	public:
	
		virtual void read(addr_t *, data_t *)=0;
		virtual void write(data_t *, addr_t *)=0;
        virtual void wait_for_reset()=0;
};

class rtl_if : public sc_module
{
	public:
		sc_out<sc_bv<7> > slave_addr;
		sc_inout<sc_lv<8> > data;
		sc_out<bool> start_transaction;
		sc_out<bool> transaction_type;
		sc_out<bool> micro_ready;
		sc_out<bool> stop_read;
		sc_out<bool> micro_data_valid;
		sc_in<bool> master_ready;
		sc_in<bool> master_data_valid;
		sc_out<bool> I2C_mode;	
};



class transactor : public rw_if,
				   public rtl_if
{		
	public:
		SC_CTOR(transactor)
		{
		}
		
		void read(addr_t *, data_t *);
		void write(data_t *, addr_t *);
        void wait_for_reset();
};

void transactor::wait_for_reset()
{
   wait(master_ready->posedge_event());
   printf("\ndone");
}

void transactor::read(addr_t *addr, data_t *dat)
{
	start_transaction=0;
	transaction_type=0;
	micro_data_valid=0;
}

void transactor::write(data_t *dat, addr_t *addr)
{
	slave_addr= *addr;
	data= *dat;
	start_transaction=1;
	transaction_type=0;
	micro_data_valid=1;
	I2C_mode=0;
	wait(master_ready->posedge_event());
}

class data_generator : public sc_module
{
	public:
		sc_port<rw_if> port;
	
	private:
		void main();
	
	public:
		SC_HAS_PROCESS(data_generator);
		data_generator(sc_module_name name) : sc_module(name)
		{
			SC_THREAD(main);
		}		
};

void data_generator::main()
{
	addr_t addr = 28;
	port->wait_for_reset();
	for(int i=0; i<20; i++)
	{
		data_t data=i;
		port->write(&data, &addr);
		printf("\nByte N°%d sended",i);
	}
}

