#ifndef DDR5_X32_INTEGRATED_H
#define DDR5_X32_INTEGRATED_H
#include <systemc.h>
#include "ddr5_controller_x32.h"
#include "ddr5_x32_write_phy.h"
#include "ddr5_x32_read_phy.h"

SC_MODULE(ddr5_x32_integrated) {
    sc_in<bool> clk; sc_in<bool> rst;
    sc_in<bool> req_valid, req_write;
    sc_in<sc_uint<26> > req_addr;
    sc_in<sc_biguint<512> > req_wdata;
    sc_in<sc_uint<64> > req_wmask;
    sc_out<bool> req_ready, rsp_valid, init_done;
    sc_out<sc_biguint<512> > rsp_rdata;

    sc_out<bool> ddr_reset_n, ddr_cs_n;
    sc_out<sc_uint<14> > ddr_ca;

    // One 32-bit DDR5 subchannel, implemented as two parallel x16 lanes.
    sc_out<sc_uint<32> > dq_out;
    sc_out<sc_uint<4> > dm_n_out, dqs_out;
    sc_out<bool> dq_oe, dqs_oe;
    sc_in<sc_uint<32> > dq_in;

    sc_signal<bool> phy_wr_start_s, phy_rd_start_s, phy_rd_valid_s;
    sc_signal<sc_biguint<512> > phy_wr_data_s, phy_rd_data_s;
    sc_signal<sc_uint<64> > phy_wr_mask_s;
    sc_signal<bool> wr_busy_s,wr_done_s,rd_busy_s,rd_capture_en_s;

    ddr5_controller_x32 *u_ctrl;
    ddr5_x32_write_phy *u_wrphy;
    ddr5_x32_read_phy *u_rdphy;

    SC_CTOR(ddr5_x32_integrated) {
        u_ctrl=new ddr5_controller_x32("u_ctrl");
        u_ctrl->clk(clk); u_ctrl->rst(rst); u_ctrl->req_valid(req_valid); u_ctrl->req_write(req_write); u_ctrl->req_addr(req_addr);
        u_ctrl->req_wdata(req_wdata); u_ctrl->req_wmask(req_wmask); u_ctrl->req_ready(req_ready); u_ctrl->rsp_valid(rsp_valid); u_ctrl->rsp_rdata(rsp_rdata); u_ctrl->init_done(init_done);
        u_ctrl->ddr_reset_n(ddr_reset_n); u_ctrl->ddr_cs_n(ddr_cs_n); u_ctrl->ddr_ca(ddr_ca);
        u_ctrl->wr_start(phy_wr_start_s); u_ctrl->wr_data(phy_wr_data_s); u_ctrl->wr_mask(phy_wr_mask_s);
        u_ctrl->rd_start(phy_rd_start_s); u_ctrl->rd_valid(phy_rd_valid_s); u_ctrl->rd_data(phy_rd_data_s);

        u_wrphy=new ddr5_x32_write_phy("u_wrphy");
        u_wrphy->clk(clk); u_wrphy->rst(rst); u_wrphy->wr_start(phy_wr_start_s); u_wrphy->wr_data(phy_wr_data_s); u_wrphy->wr_mask(phy_wr_mask_s);
        u_wrphy->dq_out(dq_out); u_wrphy->dm_n_out(dm_n_out); u_wrphy->dqs_out(dqs_out); u_wrphy->dq_oe(dq_oe); u_wrphy->dqs_oe(dqs_oe);
        u_wrphy->busy(wr_busy_s); u_wrphy->done(wr_done_s);

        u_rdphy=new ddr5_x32_read_phy("u_rdphy");
        u_rdphy->clk(clk); u_rdphy->rst(rst); u_rdphy->rd_start(phy_rd_start_s); u_rdphy->dq_in(dq_in);
        u_rdphy->rd_data(phy_rd_data_s); u_rdphy->rd_valid(phy_rd_valid_s); u_rdphy->busy(rd_busy_s); u_rdphy->capture_en(rd_capture_en_s);
    }
};
#endif
