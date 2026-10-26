#include "tb_ddr5_s2c.h"

int sc_main(int argc,char**argv) {
    tb_ddr5_dual_x32_integrated tb("tb");
#ifdef WAVE_DUMP
    sc_trace_file *trace = sc_create_vcd_trace_file("trace_behav");
    // Trace the gated clock so the long mandatory DDR5 initialization
    // does not generate millions of unnecessary clock transitions in VCD.
    sc_trace(trace,tb.trace_clk,"clk_after_init");
    sc_trace(trace,tb.rst,"rst");
    sc_trace(trace,tb.init_done,"init_done");
    sc_trace(trace,tb.req_valid,"req_valid");
    sc_trace(trace,tb.req_ready,"req_ready");
    sc_trace(trace,tb.req_write,"req_write");
    sc_trace(trace,tb.req_addr,"req_addr");
    sc_trace(trace,tb.req_wdata,"req_wdata");
    sc_trace(trace,tb.req_wmask,"req_wmask");
    sc_trace(trace,tb.rsp_valid,"rsp_valid");
    sc_trace(trace,tb.rsp_rdata,"rsp_rdata");
    sc_trace(trace,tb.rsp_subch,"rsp_subch");
    sc_trace(trace,tb.sc0_ddr_cs_n,"sc0_ddr_cs_n");
    sc_trace(trace,tb.sc0_ddr_ca,"sc0_ddr_ca");
    sc_trace(trace,tb.sc1_ddr_cs_n,"sc1_ddr_cs_n");
    sc_trace(trace,tb.sc1_ddr_ca,"sc1_ddr_ca");
    sc_trace(trace,tb.sc0_dq_out,"sc0_dq_out");
    sc_trace(trace,tb.sc1_dq_out,"sc1_dq_out");
    sc_trace(trace,tb.sc0_dq_in,"sc0_dq_in");
    sc_trace(trace,tb.sc1_dq_in,"sc1_dq_in");
    sc_trace(trace,tb.sc0_dq_oe,"sc0_dq_oe");
    sc_trace(trace,tb.sc1_dq_oe,"sc1_dq_oe");
#endif
    sc_start();
#ifdef WAVE_DUMP
    sc_close_vcd_trace_file(trace);
#endif
    return tb.done && tb.pass ? 0 : 1;
}
