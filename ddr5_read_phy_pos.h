#ifndef DDR5_READ_PHY_POS_H
#define DDR5_READ_PHY_POS_H
#include <systemc.h>
SC_MODULE(ddr5_read_phy_pos) {
    sc_in<bool> clk; sc_in<bool> rst; sc_in<bool> rd_start; sc_in<sc_uint<16> > dq_in;
    sc_in<sc_uint<16> > odd1,odd3,odd5,odd7,odd9,odd11,odd13,odd15;
    sc_out<bool> neg_capture_en; sc_out<sc_uint<3> > odd_sel;
    sc_out<sc_biguint<256> > rd_data; sc_out<bool> rd_valid, busy, capture_en;
    sc_uint<4> state; sc_uint<5> wait_count;
    sc_uint<16> even0,even2,even4,even6,even8,even10,even12,even14;
    void run();
    SC_CTOR(ddr5_read_phy_pos) { SC_CTHREAD(run,clk.pos()); reset_signal_is(rst,true); }
};
#endif
