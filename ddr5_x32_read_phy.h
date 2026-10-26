#ifndef DDR5_X32_READ_PHY_H
#define DDR5_X32_READ_PHY_H
#include <systemc.h>
#include "ddr5_read_phy.h"

// Two x16 read lanes are captured independently and re-interleaved into
// one 512-bit BL16 transaction (16 beats x 32 bits).
SC_MODULE(ddr5_x32_read_phy) {
    sc_in<bool> clk; sc_in<bool> rst;
    sc_in<bool> rd_start;
    sc_in<sc_uint<32> > dq_in;
    sc_out<sc_biguint<512> > rd_data;
    sc_out<bool> rd_valid,busy,capture_en;

    sc_signal<sc_uint<16> > lane0_dq_s,lane1_dq_s;
    sc_signal<sc_biguint<256> > lane0_data_s,lane1_data_s;
    sc_signal<bool> lane0_valid_s,lane1_valid_s,lane0_busy_s,lane1_busy_s,lane0_cap_s,lane1_cap_s;

    ddr5_read_phy *u_lane0;
    ddr5_read_phy *u_lane1;

    void split_and_merge();

    SC_CTOR(ddr5_x32_read_phy) {
        SC_METHOD(split_and_merge);
        sensitive << dq_in << lane0_data_s << lane1_data_s
                  << lane0_valid_s << lane1_valid_s
                  << lane0_busy_s << lane1_busy_s
                  << lane0_cap_s << lane1_cap_s;

        u_lane0 = new ddr5_read_phy("u_lane0");
        u_lane0->clk(clk); u_lane0->rst(rst); u_lane0->rd_start(rd_start); u_lane0->dq_in(lane0_dq_s);
        u_lane0->rd_data(lane0_data_s); u_lane0->rd_valid(lane0_valid_s); u_lane0->busy(lane0_busy_s); u_lane0->capture_en(lane0_cap_s);

        u_lane1 = new ddr5_read_phy("u_lane1");
        u_lane1->clk(clk); u_lane1->rst(rst); u_lane1->rd_start(rd_start); u_lane1->dq_in(lane1_dq_s);
        u_lane1->rd_data(lane1_data_s); u_lane1->rd_valid(lane1_valid_s); u_lane1->busy(lane1_busy_s); u_lane1->capture_en(lane1_cap_s);
    }
};
#endif
