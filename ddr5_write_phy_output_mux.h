#ifndef DDR5_WRITE_PHY_OUTPUT_MUX_H
#define DDR5_WRITE_PHY_OUTPUT_MUX_H
#include <systemc.h>
SC_MODULE(ddr5_write_phy_output_mux) {
    sc_in<bool> clk;
    sc_in<sc_uint<16> > rise_dq, fall_dq;
    sc_in<sc_uint<2> > rise_dm_n, fall_dm_n, rise_dqs, fall_dqs;
    sc_out<sc_uint<16> > dq_out;
    sc_out<sc_uint<2> > dm_n_out, dqs_out;
    void mux_proc();
    SC_CTOR(ddr5_write_phy_output_mux) { SC_METHOD(mux_proc); sensitive<<clk<<rise_dq<<fall_dq<<rise_dm_n<<fall_dm_n<<rise_dqs<<fall_dqs; }
};
#endif
