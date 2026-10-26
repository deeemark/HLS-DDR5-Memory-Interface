#ifndef DDR5_WRITE_PHY_NEG_H
#define DDR5_WRITE_PHY_NEG_H
#include <systemc.h>
SC_MODULE(ddr5_write_phy_neg) {
    sc_in<bool> clk; sc_in<bool> rst;
    sc_in<sc_uint<16> > fall_dq_next;
    sc_in<sc_uint<2> > fall_dm_n_next;
    sc_in<sc_uint<2> > fall_dqs_next;
    sc_out<sc_uint<16> > fall_dq;
    sc_out<sc_uint<2> > fall_dm_n;
    sc_out<sc_uint<2> > fall_dqs;
    void run();
    SC_CTOR(ddr5_write_phy_neg) { SC_CTHREAD(run, clk.neg()); reset_signal_is(rst,true); }
};
#endif
