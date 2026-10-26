#ifndef DDR5_DUAL_X32_INTEGRATED_H
#define DDR5_DUAL_X32_INTEGRATED_H
#include <systemc.h>
#include "ddr5_x32_integrated.h"
#include "ddr5_dual_x32_router.h"

SC_MODULE(ddr5_dual_x32_integrated) {
    sc_in<bool> clk, rst;

    // Shared host port.
    sc_in<bool> req_valid, req_write;
    sc_in<sc_uint<27> > req_addr; //26 bit addresses per channel and 1 bit to choose subchannel
    sc_in<sc_biguint<512> > req_wdata; //32bits x 16 transfers = 512
    sc_in<sc_uint<64> > req_wmask; //covers which bytes to mask
    sc_out<bool> req_ready;
    sc_out<bool> rsp_valid;
    sc_out<sc_biguint<512> > rsp_rdata; 
    sc_out<bool> rsp_subch;
    sc_out<bool> init_done;

    // Independent command/address and x32 data paths for subchannel 0.
    sc_out<bool> sc0_ddr_reset_n, sc0_ddr_cs_n;
    sc_out<sc_uint<14> > sc0_ddr_ca;
    sc_out<sc_uint<32> > sc0_dq_out;
    sc_out<sc_uint<4> > sc0_dm_n_out, sc0_dqs_out;
    sc_out<bool> sc0_dq_oe, sc0_dqs_oe;
    sc_in<sc_uint<32> > sc0_dq_in;

    // Independent command/address and x32 data paths for subchannel 1.
    sc_out<bool> sc1_ddr_reset_n, sc1_ddr_cs_n;
    sc_out<sc_uint<14> > sc1_ddr_ca;
    sc_out<sc_uint<32> > sc1_dq_out;
    sc_out<sc_uint<4> > sc1_dm_n_out, sc1_dqs_out;
    sc_out<bool> sc1_dq_oe, sc1_dqs_oe;
    sc_in<sc_uint<32> > sc1_dq_in;

    sc_signal<bool> s0_req_valid_s, s0_req_write_s, s0_req_ready_s, s0_rsp_valid_s, s0_init_done_s;
    sc_signal<sc_uint<26> > s0_req_addr_s;
    sc_signal<sc_biguint<512> > s0_req_wdata_s, s0_rsp_rdata_s;
    sc_signal<sc_uint<64> > s0_req_wmask_s;

    sc_signal<bool> s1_req_valid_s, s1_req_write_s, s1_req_ready_s, s1_rsp_valid_s, s1_init_done_s;
    sc_signal<sc_uint<26> > s1_req_addr_s;
    sc_signal<sc_biguint<512> > s1_req_wdata_s, s1_rsp_rdata_s;
    sc_signal<sc_uint<64> > s1_req_wmask_s;

    ddr5_dual_x32_router *u_router;
    ddr5_x32_integrated *u_sc0;
    ddr5_x32_integrated *u_sc1;

    //only write when both subchannels are completely initialized
    void combine_init() {
        init_done.write(s0_init_done_s.read() && s1_init_done_s.read());
    }

    SC_CTOR(ddr5_dual_x32_integrated) {
        u_router = new ddr5_dual_x32_router("u_router");
        u_router->clk(clk); u_router->rst(rst);
        u_router->req_valid(req_valid); u_router->req_write(req_write); u_router->req_addr(req_addr);
        u_router->req_wdata(req_wdata); u_router->req_wmask(req_wmask); u_router->req_ready(req_ready);
        u_router->rsp_valid(rsp_valid); u_router->rsp_rdata(rsp_rdata); u_router->rsp_subch(rsp_subch);
        u_router->s0_req_valid(s0_req_valid_s); u_router->s0_req_write(s0_req_write_s); u_router->s0_req_addr(s0_req_addr_s);
        u_router->s0_req_wdata(s0_req_wdata_s); u_router->s0_req_wmask(s0_req_wmask_s); u_router->s0_req_ready(s0_req_ready_s);
        u_router->s0_rsp_valid(s0_rsp_valid_s); u_router->s0_rsp_rdata(s0_rsp_rdata_s);
        u_router->s1_req_valid(s1_req_valid_s); u_router->s1_req_write(s1_req_write_s); u_router->s1_req_addr(s1_req_addr_s);
        u_router->s1_req_wdata(s1_req_wdata_s); u_router->s1_req_wmask(s1_req_wmask_s); u_router->s1_req_ready(s1_req_ready_s);
        u_router->s1_rsp_valid(s1_rsp_valid_s); u_router->s1_rsp_rdata(s1_rsp_rdata_s);

        u_sc0 = new ddr5_x32_integrated("u_sc0");
        u_sc0->clk(clk); u_sc0->rst(rst);
        u_sc0->req_valid(s0_req_valid_s); u_sc0->req_write(s0_req_write_s); u_sc0->req_addr(s0_req_addr_s);
        u_sc0->req_wdata(s0_req_wdata_s); u_sc0->req_wmask(s0_req_wmask_s); u_sc0->req_ready(s0_req_ready_s);
        u_sc0->rsp_valid(s0_rsp_valid_s); u_sc0->rsp_rdata(s0_rsp_rdata_s); u_sc0->init_done(s0_init_done_s);
        u_sc0->ddr_reset_n(sc0_ddr_reset_n); u_sc0->ddr_cs_n(sc0_ddr_cs_n); u_sc0->ddr_ca(sc0_ddr_ca);
        u_sc0->dq_out(sc0_dq_out); u_sc0->dm_n_out(sc0_dm_n_out); u_sc0->dqs_out(sc0_dqs_out);
        u_sc0->dq_oe(sc0_dq_oe); u_sc0->dqs_oe(sc0_dqs_oe); u_sc0->dq_in(sc0_dq_in);

        u_sc1 = new ddr5_x32_integrated("u_sc1");
        u_sc1->clk(clk); u_sc1->rst(rst);
        u_sc1->req_valid(s1_req_valid_s); u_sc1->req_write(s1_req_write_s); u_sc1->req_addr(s1_req_addr_s);
        u_sc1->req_wdata(s1_req_wdata_s); u_sc1->req_wmask(s1_req_wmask_s); u_sc1->req_ready(s1_req_ready_s);
        u_sc1->rsp_valid(s1_rsp_valid_s); u_sc1->rsp_rdata(s1_rsp_rdata_s); u_sc1->init_done(s1_init_done_s);
        u_sc1->ddr_reset_n(sc1_ddr_reset_n); u_sc1->ddr_cs_n(sc1_ddr_cs_n); u_sc1->ddr_ca(sc1_ddr_ca);
        u_sc1->dq_out(sc1_dq_out); u_sc1->dm_n_out(sc1_dm_n_out); u_sc1->dqs_out(sc1_dqs_out);
        u_sc1->dq_oe(sc1_dq_oe); u_sc1->dqs_oe(sc1_dqs_oe); u_sc1->dq_in(sc1_dq_in);

        SC_METHOD(combine_init);
        sensitive << s0_init_done_s << s1_init_done_s;
    }
};
#endif
