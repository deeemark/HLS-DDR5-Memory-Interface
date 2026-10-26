#ifndef DDR5_X32_WRITE_PHY_H
#define DDR5_X32_WRITE_PHY_H
#include <systemc.h>
#include "ddr5_write_phy.h"

// One DDR5 32-bit subchannel = two x16 devices/lanes operating in lockstep.
// The controller presents one 512-bit BL16 transaction (16 beats x 32 bits).
// This wrapper de-interleaves each 32-bit beat into two 16-bit lane streams.
SC_MODULE(ddr5_x32_write_phy) {
    sc_in<bool> clk; sc_in<bool> rst;
    sc_in<bool> wr_start;
    sc_in<sc_biguint<512> > wr_data;
    sc_in<sc_uint<64> > wr_mask;

    sc_out<sc_uint<32> > dq_out;
    sc_out<sc_uint<4> > dm_n_out, dqs_out;
    sc_out<bool> dq_oe, dqs_oe, busy, done;

    sc_signal<sc_biguint<256> > lane0_data_s, lane1_data_s;
    sc_signal<sc_uint<32> > lane0_mask_s, lane1_mask_s;
    sc_signal<sc_uint<16> > lane0_dq_s, lane1_dq_s;
    sc_signal<sc_uint<2> > lane0_dm_s, lane1_dm_s, lane0_dqs_s, lane1_dqs_s;
    sc_signal<bool> lane0_dq_oe_s,lane1_dq_oe_s,lane0_dqs_oe_s,lane1_dqs_oe_s;
    sc_signal<bool> lane0_busy_s,lane1_busy_s,lane0_done_s,lane1_done_s;

    ddr5_write_phy *u_lane0;
    ddr5_write_phy *u_lane1;

    void split_and_combine();

    SC_CTOR(ddr5_x32_write_phy) {
        SC_METHOD(split_and_combine);
        sensitive << wr_data << wr_mask
                  << lane0_dq_s << lane1_dq_s
                  << lane0_dm_s << lane1_dm_s
                  << lane0_dqs_s << lane1_dqs_s
                  << lane0_dq_oe_s << lane1_dq_oe_s
                  << lane0_dqs_oe_s << lane1_dqs_oe_s
                  << lane0_busy_s << lane1_busy_s
                  << lane0_done_s << lane1_done_s;

        u_lane0 = new ddr5_write_phy("u_lane0");
        u_lane0->clk(clk); u_lane0->rst(rst); u_lane0->wr_start(wr_start);
        u_lane0->wr_data(lane0_data_s); u_lane0->wr_mask(lane0_mask_s);
        u_lane0->dq_out(lane0_dq_s); u_lane0->dm_n_out(lane0_dm_s); u_lane0->dqs_out(lane0_dqs_s);
        u_lane0->dq_oe(lane0_dq_oe_s); u_lane0->dqs_oe(lane0_dqs_oe_s); u_lane0->busy(lane0_busy_s); u_lane0->done(lane0_done_s);

        u_lane1 = new ddr5_write_phy("u_lane1");
        u_lane1->clk(clk); u_lane1->rst(rst); u_lane1->wr_start(wr_start);
        u_lane1->wr_data(lane1_data_s); u_lane1->wr_mask(lane1_mask_s);
        u_lane1->dq_out(lane1_dq_s); u_lane1->dm_n_out(lane1_dm_s); u_lane1->dqs_out(lane1_dqs_s);
        u_lane1->dq_oe(lane1_dq_oe_s); u_lane1->dqs_oe(lane1_dqs_oe_s); u_lane1->busy(lane1_busy_s); u_lane1->done(lane1_done_s);
    }
};
#endif
