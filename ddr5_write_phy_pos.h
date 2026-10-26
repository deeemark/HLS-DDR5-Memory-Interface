#ifndef DDR5_WRITE_PHY_POS_H
#define DDR5_WRITE_PHY_POS_H
#include <systemc.h>

SC_MODULE(ddr5_write_phy_pos) {
    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_in<bool> wr_start;
    sc_in<sc_biguint<256> > wr_data;
    sc_in<sc_uint<32> > wr_mask;

    sc_out<sc_uint<16> > rise_dq;
    sc_out<sc_uint<2> > rise_dm_n;
    sc_out<sc_uint<2> > rise_dqs;
    sc_out<sc_uint<16> > fall_dq_next;
    sc_out<sc_uint<2> > fall_dm_n_next;
    sc_out<sc_uint<2> > fall_dqs_next;
    sc_out<bool> dq_oe;
    sc_out<bool> dqs_oe;
    sc_out<bool> busy;
    sc_out<bool> done;

    sc_uint<4> state;
    sc_uint<5> wait_count;
    sc_biguint<256> data_reg;
    sc_uint<32> mask_reg;

    void run();
    SC_CTOR(ddr5_write_phy_pos) {
        SC_CTHREAD(run, clk.pos());
        reset_signal_is(rst, true);
    }
};
#endif
