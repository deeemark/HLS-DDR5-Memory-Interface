#ifndef DDR5_READ_PHY_H
#define DDR5_READ_PHY_H
#include <systemc.h>
#include "ddr5_read_phy_pos.h"
#include "ddr5_read_phy_neg.h"
SC_MODULE(ddr5_read_phy) {
    sc_in<bool> clk; sc_in<bool> rst; sc_in<bool> rd_start; sc_in<sc_uint<16> > dq_in;
    sc_out<sc_biguint<256> > rd_data; sc_out<bool> rd_valid,busy,capture_en;
    sc_signal<bool> neg_capture_en_s; sc_signal<sc_uint<3> > odd_sel_s;
    sc_signal<sc_uint<16> > odd1_s,odd3_s,odd5_s,odd7_s,odd9_s,odd11_s,odd13_s,odd15_s;
    ddr5_read_phy_pos *u_pos; ddr5_read_phy_neg *u_neg;
    SC_CTOR(ddr5_read_phy) {
        u_pos=new ddr5_read_phy_pos("u_pos"); u_pos->clk(clk); u_pos->rst(rst); u_pos->rd_start(rd_start); u_pos->dq_in(dq_in);
        u_pos->odd1(odd1_s); u_pos->odd3(odd3_s); u_pos->odd5(odd5_s); u_pos->odd7(odd7_s); u_pos->odd9(odd9_s); u_pos->odd11(odd11_s); u_pos->odd13(odd13_s); u_pos->odd15(odd15_s);
        u_pos->neg_capture_en(neg_capture_en_s); u_pos->odd_sel(odd_sel_s); u_pos->rd_data(rd_data); u_pos->rd_valid(rd_valid); u_pos->busy(busy); u_pos->capture_en(capture_en);
        u_neg=new ddr5_read_phy_neg("u_neg"); u_neg->clk(clk); u_neg->rst(rst); u_neg->capture_en(neg_capture_en_s); u_neg->odd_sel(odd_sel_s); u_neg->dq_in(dq_in);
        u_neg->odd1(odd1_s); u_neg->odd3(odd3_s); u_neg->odd5(odd5_s); u_neg->odd7(odd7_s); u_neg->odd9(odd9_s); u_neg->odd11(odd11_s); u_neg->odd13(odd13_s); u_neg->odd15(odd15_s);
    }
};
#endif
