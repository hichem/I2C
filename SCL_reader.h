#include <systemc.h>

class SCL_reader : public sc_module
{
	public:
	sc_in<sc_logic> clk;
	sc_in<sc_logic> rst;
	sc_in<sc_logic> SCL;
	sc_in<sc_logic> SDA;
	
	sc_out<sc_lv<8> > data_out;
	sc_out<sc_logic> ACK;
	sc_out<bool> data_valid;
	
	private:
	sc_signal<bool> start_condition;
	sc_signal<sc_logic> prev_SDA;
	sc_signal<sc_logic> prev_SCL;
	sc_lv<8> data_reg;
	sc_signal<sc_logic> ack_reg;

	sc_signal<int> count;	

	void read_SCL();
	void track_start_condition();

	public:
	SC_HAS_PROCESS(SCL_reader);
	SCL_reader(sc_module_name name):
		sc_module(name)
	{
		SC_METHOD(track_start_condition);
			sensitive << clk.pos();
		SC_METHOD(read_SCL);
			sensitive << clk.pos() << start_condition;
	}
};

void SCL_reader::track_start_condition()
{
	if(rst==(sc_logic)1)
	{
		start_condition=0;
		prev_SDA=SDA;
	}
	else
	{
		start_condition=0;
		if((SCL==(sc_logic)1) && (prev_SDA==(sc_logic)1) && (SDA==(sc_logic)0))
		{
			start_condition=1;
			//printf("\ndetected start condition (master)");
		}
		prev_SDA=SDA;
	}
}

void SCL_reader::read_SCL()
{
	sc_lv<8> wire;
	
	if(rst==(sc_logic)1)
	{
		count=0;
		data_reg="11111111";
		data_valid=0;
		prev_SCL=(sc_logic)1;
		ack_reg=(sc_logic)1;
		ACK=(sc_logic)1;
	}
	else
	{
		ack_reg=ack_reg;
		data_reg=data_reg;
		data_valid=0;
		if(start_condition==1)
		{
			//printf("\nstart condition tracked (master)");
			count=0;
			data_reg="11111111";
		}
		else if((prev_SCL==(sc_logic)0) && (SCL==(sc_logic)1))		//a positive front edge of SCL
		{
			if(count==8)
			{			
				count=0;
				ack_reg=SDA;
				data_reg="11111111";
			}
			else if(count==7)
			{
				//data_out=data_reg;
				for(int i=0;i<7;i++)
					wire[i+1]=data_reg[i];
				wire[0]=SDA;
				data_out=wire;
				data_valid=1;
				count=count+1;
			}
			else
			{
				for(int i=6;i>=0;i--)
					data_reg[i+1]=data_reg[i];
				data_reg[0]=SDA;
				count=count+1;
			}
		}
		else
		{
			
			count=count;
		}
		prev_SCL=SCL;
		ACK=ack_reg;
	}
}
