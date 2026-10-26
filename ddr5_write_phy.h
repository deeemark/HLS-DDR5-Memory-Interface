#ifndef DDR5_WRITE_PHY_H
#define DDR5_WRITE_PHY_H
#include <systemc.h>
#include "ddr5_write_phy_pos.h"
#include "ddr5_write_phy_neg.h"
#include "ddr5_write_phy_output_mux.h"

SC_MODULE(ddr5_write_phy) {
    sc_in<bool> clk; sc_in<bool> rst;
    sc_in<bool> wr_start;
    sc_in<sc_biguint<256> > wr_data;
    sc_in<sc_uint<32> > wr_mask;
    sc_out<sc_uint<16> > dq_out;
    sc_out<sc_uint<2> > dm_n_out, dqs_out;
    sc_out<bool> dq_oe, dqs_oe, busy, done;

    sc_signal<sc_uint<16> > rise_dq_s, fall_dq_next_s, fall_dq_s;
    sc_signal<sc_uint<2> > rise_dm_n_s, fall_dm_n_next_s, fall_dm_n_s;
    sc_signal<sc_uint<2> > rise_dqs_s, fall_dqs_next_s, fall_dqs_s;
    ddr5_write_phy_pos *u_pos; ddr5_write_phy_neg *u_neg; ddr5_write_phy_output_mux *u_mux;

    SC_CTOR(ddr5_write_phy) {
        u_pos=new ddr5_write_phy_pos("u_pos");
        u_pos->clk(clk); u_pos->rst(rst); u_pos->wr_start(wr_start); u_pos->wr_data(wr_data); u_pos->wr_mask(wr_mask);
        u_pos->rise_dq(rise_dq_s); u_pos->rise_dm_n(rise_dm_n_s); u_pos->rise_dqs(rise_dqs_s);
        u_pos->fall_dq_next(fall_dq_next_s); u_pos->fall_dm_n_next(fall_dm_n_next_s); u_pos->fall_dqs_next(fall_dqs_next_s);
        u_pos->dq_oe(dq_oe); u_pos->dqs_oe(dqs_oe); u_pos->busy(busy); u_pos->done(done);
        u_neg=new ddr5_write_phy_neg("u_neg");
        u_neg->clk(clk); u_neg->rst(rst); u_neg->fall_dq_next(fall_dq_next_s); u_neg->fall_dm_n_next(fall_dm_n_next_s); u_neg->fall_dqs_next(fall_dqs_next_s);
        u_neg->fall_dq(fall_dq_s); u_neg->fall_dm_n(fall_dm_n_s); u_neg->fall_dqs(fall_dqs_s);
        u_mux=new ddr5_write_phy_output_mux("u_mux");
        u_mux->clk(clk); u_mux->rise_dq(rise_dq_s); u_mux->fall_dq(fall_dq_s); u_mux->rise_dm_n(rise_dm_n_s); u_mux->fall_dm_n(fall_dm_n_s);
        u_mux->rise_dqs(rise_dqs_s); u_mux->fall_dqs(fall_dqs_s); u_mux->dq_out(dq_out); u_mux->dm_n_out(dm_n_out); u_mux->dqs_out(dqs_out);
    }
};
#endif
