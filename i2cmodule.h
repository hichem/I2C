#include "systemc.h"

class i2cmodule : public ncsc_foreign_module {
public:
	sc_in < sc_logic > i2crst;
	sc_in < sc_logic > clk;
	sc_in < sc_logic > sdam_s;
	sc_in < sc_logic > scl;
	sc_in < sc_logic > csi2c;
	sc_in < sc_lv <8> > din;
	sc_out < sc_lv <8> > aout;
	sc_out < sc_lv <8> > dout;
	sc_out < sc_logic > ldreg;
	sc_out < sc_logic > rdreg;
	sc_out < sc_logic > sdas_m;
	sc_out < sc_logic > sdaio;

	i2cmodule(
		sc_module_name nm
	) : ncsc_foreign_module(nm)
		, i2crst("i2crst")
		, clk("clk")
		, sdam_s("sdam_s")
		, scl("scl")
		, csi2c("csi2c")
		, din("din")
		, aout("aout")
		, dout("dout")
		, ldreg("ldreg")
		, rdreg("rdreg")
		, sdas_m("sdas_m")
		, sdaio("sdaio")

	{
	}

	const char* hdl_name() const { return "i2cmodule"; }
};

