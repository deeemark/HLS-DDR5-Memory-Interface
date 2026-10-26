#ifndef DDR5_READ_PHY_NEG_H
#define DDR5_READ_PHY_NEG_H
#include <systemc.h>
SC_MODULE(ddr5_read_phy_neg) {
    sc_in<bool> clk; sc_in<bool> rst; sc_in<bool> capture_en; sc_in<sc_uint<3> > odd_sel; sc_in<sc_uint<16> > dq_in;
    sc_out<sc_uint<16> > odd1,odd3,odd5,odd7,odd9,odd11,odd13,odd15;
    void run();
    SC_CTOR(ddr5_read_phy_neg) { SC_CTHREAD(run,clk.neg()); reset_signal_is(rst,true); }
};
#endif
