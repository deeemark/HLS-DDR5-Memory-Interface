#ifndef DDR5_DUAL_X32_ROUTER_H
#define DDR5_DUAL_X32_ROUTER_H
#include <systemc.h>

// Shared host-side front end for two independent 32-bit DDR5 subchannels.
// req_addr[26] selects the subchannel; req_addr[25:0] is the unchanged
// local BL16 transaction address used by a single x32 controller.
SC_MODULE(ddr5_dual_x32_router) {
    sc_in<bool> clk, rst;

    sc_in<bool> req_valid, req_write;
    sc_in<sc_uint<27> > req_addr;
    sc_in<sc_biguint<512> > req_wdata;
    sc_in<sc_uint<64> > req_wmask;
    sc_out<bool> req_ready;

    sc_out<bool> rsp_valid;
    sc_out<sc_biguint<512> > rsp_rdata;
    sc_out<bool> rsp_subch;

    // Subchannel 0 request/response side.
    sc_out<bool> s0_req_valid, s0_req_write;
    sc_out<sc_uint<26> > s0_req_addr;
    sc_out<sc_biguint<512> > s0_req_wdata;
    sc_out<sc_uint<64> > s0_req_wmask;
    sc_in<bool> s0_req_ready, s0_rsp_valid;
    sc_in<sc_biguint<512> > s0_rsp_rdata;

    // Subchannel 1 request/response side.
    sc_out<bool> s1_req_valid, s1_req_write;
    sc_out<sc_uint<26> > s1_req_addr;
    sc_out<sc_biguint<512> > s1_req_wdata;
    sc_out<sc_uint<64> > s1_req_wmask;
    sc_in<bool> s1_req_ready, s1_rsp_valid;
    sc_in<sc_biguint<512> > s1_rsp_rdata;

    bool pending0_r, pending1_r;
    sc_biguint<512> pending_data0_r, pending_data1_r;
    bool prefer1_r;
    //directs request to the correct subchannel
    void route_req() {
        const bool sel = req_addr.read()[26]; //subchannel chosen by sel
        const bool v = req_valid.read();

        s0_req_valid.write(v && !sel);
        s1_req_valid.write(v &&  sel);
        s0_req_write.write(req_write.read());
        s1_req_write.write(req_write.read());
        s0_req_addr.write(req_addr.read().range(25,0));
        s1_req_addr.write(req_addr.read().range(25,0));
        s0_req_wdata.write(req_wdata.read());
        s1_req_wdata.write(req_wdata.read());
        s0_req_wmask.write(req_wmask.read());
        s1_req_wmask.write(req_wmask.read());
        req_ready.write(sel ? s1_req_ready.read() : s0_req_ready.read());
    }

    // Responses from the two independent controllers may arrive together.
    // Keep one pending register per source and serialize them onto the shared
    // host response port. rsp_subch identifies the source subchannel.
    void collect_rsp() {
        //is the router holding a response froma subchannel?
        pending0_r = false;
        pending1_r = false;
        pending_data0_r = 0;
        pending_data1_r = 0;
        prefer1_r = false;
        rsp_valid.write(false);
        rsp_rdata.write(0);
        rsp_subch.write(false);
        wait();

        while (true) {
            bool p0 = pending0_r;
            bool p1 = pending1_r;
            sc_biguint<512> d0 = pending_data0_r;
            sc_biguint<512> d1 = pending_data1_r;

            rsp_valid.write(false);

            // Retire one already-pending response first. Round-robin only
            // matters when both are present.
            if (p0 && p1) {
                if (prefer1_r) {
                    rsp_valid.write(true); rsp_rdata.write(d1); rsp_subch.write(true);
                    p1 = false; prefer1_r = false;
                } else {
                    rsp_valid.write(true); rsp_rdata.write(d0); rsp_subch.write(false);
                    p0 = false; prefer1_r = true;
                }
            } else if (p0) {
                rsp_valid.write(true); rsp_rdata.write(d0); rsp_subch.write(false);
                p0 = false; prefer1_r = true;
            } else if (p1) {
                rsp_valid.write(true); rsp_rdata.write(d1); rsp_subch.write(true);
                p1 = false; prefer1_r = false;
            }

            // Capture new child response pulses after retiring a slot.
            if (s0_rsp_valid.read()) {
                d0 = s0_rsp_rdata.read();
                p0 = true;
            }
            if (s1_rsp_valid.read()) {
                d1 = s1_rsp_rdata.read();
                p1 = true;
            }

            pending0_r = p0;
            pending1_r = p1;
            pending_data0_r = d0;
            pending_data1_r = d1;
            wait();
        }
    }

    SC_CTOR(ddr5_dual_x32_router) {
        SC_METHOD(route_req);
        sensitive << req_valid << req_write << req_addr << req_wdata << req_wmask
                  << s0_req_ready << s1_req_ready;

        SC_CTHREAD(collect_rsp, clk.pos());
        reset_signal_is(rst, true);
    }
};
#endif
