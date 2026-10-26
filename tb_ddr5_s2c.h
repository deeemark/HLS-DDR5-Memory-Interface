#ifndef DDR5_S2C_BENCH_H
#define DDR5_S2C_BENCH_H
#include <systemc.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "ddr5_dual_x32_integrated.h"
#include "ddr5_x32_memory_model.h"

SC_MODULE(tb_ddr5_dual_x32_integrated) {
    sc_clock clk;
    sc_signal<bool> rst;

    sc_signal<bool> req_valid, req_write, req_ready, rsp_valid, rsp_subch, init_done;
    sc_signal<sc_uint<27> > req_addr;
    sc_signal<sc_biguint<512> > req_wdata, rsp_rdata;
    sc_signal<sc_uint<64> > req_wmask;

    sc_signal<bool> sc0_ddr_reset_n, sc0_ddr_cs_n, sc0_dq_oe, sc0_dqs_oe;
    sc_signal<sc_uint<14> > sc0_ddr_ca;
    sc_signal<sc_uint<32> > sc0_dq_out, sc0_dq_in;
    sc_signal<sc_uint<4> > sc0_dm_n_out, sc0_dqs_out;

    sc_signal<bool> sc1_ddr_reset_n, sc1_ddr_cs_n, sc1_dq_oe, sc1_dqs_oe;
    sc_signal<sc_uint<14> > sc1_ddr_ca;
    sc_signal<sc_uint<32> > sc1_dq_out, sc1_dq_in;
    sc_signal<sc_uint<4> > sc1_dm_n_out, sc1_dqs_out;

    ddr5_dual_x32_integrated dut;
    ddr5_x32_memory_model mem0, mem1;

    sc_biguint<512> pattern0, pattern1;
    bool overlap_seen;

    bool done;
    bool pass;
    unsigned expected_reads;
    unsigned received_reads;
    unsigned errors;
    bool trace_clk_value;
    sc_signal<bool> trace_clk;

    void trace_clock_gate();

    static sc_biguint<512> make_pattern(unsigned seed);

    bool enqueue(bool subch, bool wr, sc_uint<26> local_addr,
                 sc_biguint<512> data, sc_uint<64> mask);

    void overlap_monitor();

    void recv();

    static std::string canonical_hex(const char *s);

    bool compare_results();

    void send();

    SC_CTOR(tb_ddr5_dual_x32_integrated)
        : clk("clk", sc_time(625,SC_PS)), dut("dut"), mem0("mem0"), mem1("mem1") {
        dut.clk(clk); dut.rst(rst);
        dut.req_valid(req_valid); dut.req_write(req_write); dut.req_addr(req_addr); dut.req_wdata(req_wdata); dut.req_wmask(req_wmask);
        dut.req_ready(req_ready); dut.rsp_valid(rsp_valid); dut.rsp_rdata(rsp_rdata); dut.rsp_subch(rsp_subch); dut.init_done(init_done);

        dut.sc0_ddr_reset_n(sc0_ddr_reset_n); dut.sc0_ddr_cs_n(sc0_ddr_cs_n); dut.sc0_ddr_ca(sc0_ddr_ca);
        dut.sc0_dq_out(sc0_dq_out); dut.sc0_dm_n_out(sc0_dm_n_out); dut.sc0_dqs_out(sc0_dqs_out); dut.sc0_dq_oe(sc0_dq_oe); dut.sc0_dqs_oe(sc0_dqs_oe); dut.sc0_dq_in(sc0_dq_in);
        dut.sc1_ddr_reset_n(sc1_ddr_reset_n); dut.sc1_ddr_cs_n(sc1_ddr_cs_n); dut.sc1_ddr_ca(sc1_ddr_ca);
        dut.sc1_dq_out(sc1_dq_out); dut.sc1_dm_n_out(sc1_dm_n_out); dut.sc1_dqs_out(sc1_dqs_out); dut.sc1_dq_oe(sc1_dq_oe); dut.sc1_dqs_oe(sc1_dqs_oe); dut.sc1_dq_in(sc1_dq_in);

        mem0.clk(clk); mem0.rst(rst); mem0.ddr_reset_n(sc0_ddr_reset_n); mem0.ddr_cs_n(sc0_ddr_cs_n); mem0.ddr_ca(sc0_ddr_ca);
        mem0.dq_out(sc0_dq_out); mem0.dm_n_out(sc0_dm_n_out); mem0.dq_oe(sc0_dq_oe); mem0.dq_in(sc0_dq_in);
        mem1.clk(clk); mem1.rst(rst); mem1.ddr_reset_n(sc1_ddr_reset_n); mem1.ddr_cs_n(sc1_ddr_cs_n); mem1.ddr_ca(sc1_ddr_ca);
        mem1.dq_out(sc1_dq_out); mem1.dm_n_out(sc1_dm_n_out); mem1.dq_oe(sc1_dq_oe); mem1.dq_in(sc1_dq_in);

        SC_THREAD(send);
        SC_THREAD(recv);
        SC_METHOD(trace_clock_gate);
        sensitive << clk << init_done;
        SC_THREAD(overlap_monitor);
    }
};
#endif
