#ifndef DDR5_CONTROLLER_H
#define DDR5_CONTROLLER_H

#include <systemc.h>

SC_MODULE(ddr5_controller_x32) {
    sc_in<bool> clk;
    sc_in<bool> rst;

    // Host side: buffered 64-byte transactions (8-entry FCFS FIFO).
    sc_in<bool> req_valid;
    sc_in<bool> req_write;
    sc_in<sc_uint<26> > req_addr;
    sc_in<sc_biguint<512> > req_wdata;
    sc_in<sc_uint<64> > req_wmask;

    sc_out<bool> req_ready;
    sc_out<bool> rsp_valid;
    sc_out<sc_biguint<512> > rsp_rdata;
    sc_out<bool> init_done;

    // DDR5 command/address interface.
    sc_out<bool> ddr_reset_n;
    sc_out<bool> ddr_cs_n;
    sc_out<sc_uint<14> > ddr_ca;

    // Logical PHY interface.
    sc_out<bool> wr_start;
    sc_out<sc_biguint<512> > wr_data;
    sc_out<sc_uint<64> > wr_mask;

    sc_out<bool> rd_start;
    sc_in<bool> rd_valid;
    sc_in<sc_biguint<512> > rd_data;

    // DDR5-3200B, tCK = 0.625 ns.
    static const unsigned T_RCD       = 26;
    static const unsigned T_RP        = 26;
    static const unsigned T_RAS       = 52;
    static const unsigned T_RC        = 78;
    static const unsigned T_WR        = 48;
    static const unsigned T_RTP       = 12;
    static const unsigned T_CCD_L     = 8;
    static const unsigned T_CCD_L_WR  = 32;
    static const unsigned T_DLLK      = 1024;

    // 16 Gb normal-refresh baseline.
    static const unsigned T_REFI      = 6240; // 3.9 us
    static const unsigned T_RFC1      = 472;  // 295 ns

    // Power-up / reset initialization.
    static const unsigned T_INIT1     = 320000;  // 200 us
    static const unsigned T_INIT3     = 6400000; // 4 ms
    static const unsigned T_INIT4     = 3200;    // 2 us
    static const unsigned T_INIT5     = 3;
    static const unsigned T_XPR       = T_RFC1;
    static const unsigned T_MRD       = 23;
    static const unsigned T_MPC_CS    = 4;
    static const unsigned T_ZQCAL     = 1600;
    static const unsigned T_ZQLAT     = 48;

    // Functional mode registers.
    static const unsigned MR0_VALUE   = 0x08;
    static const unsigned MR2_VALUE   = 0x00;
    static const unsigned MR3_VALUE   = 0x00;
    static const unsigned MR4_VALUE   = 0x00;
    static const unsigned MR5_VALUE   = 0x20;
    static const unsigned MR6_VALUE   = 0x00;
    static const unsigned MR8_VALUE   = 0x08;
    static const unsigned MR10_VALUE  = 0x2D;

    // MPC opcodes.
    static const unsigned MPC_DLL_RESET  = 0x02;
    static const unsigned MPC_ZQ_LATCH   = 0x04;
    static const unsigned MPC_ZQ_START   = 0x05;
    static const unsigned MPC_SET_1N     = 0x09;
    static const unsigned MPC_TDLLK_3200 = 0x80;

    // State encoding (HLS timing): the wait-counter logic keys off class bits
    // instead of decoding the full state value.
    //   bit7 (0x80) : init wait-down state (init_wait_count_r decrements)
    //   bit6 (0x40) : loads T_MRD-1 into init_wait_count_r
    //   bit4 (0x10) : op wait-down state (op_wait_count_r decrements)
    // Remaining bits are a unique ordinal.  Reset state = 0x00.
    enum state_t {
        ST_RESET_LOW        = 0x00,
        ST_INIT3_CSLOW      = 0x01,
        ST_MR0_0 = 0x02, ST_MR2_0 = 0x03, ST_MR3_0 = 0x04, ST_MR4_0 = 0x05,
        ST_MR5_0 = 0x06, ST_MR6_0 = 0x07, ST_MR8_0 = 0x08, ST_MR10_0 = 0x09,
        ST_READY            = 0x0A,
        ST_CAPTURE          = 0x0B,
        ST_ACT_0            = 0x0C,
        ST_ACT_1            = 0x0D,
        ST_WR_0             = 0x0E,
        ST_RD_0             = 0x0F,

        // Op wait-down states
        ST_WAIT_RCD         = 0x10,
        ST_WAIT_WRITE_DONE  = 0x11,
        ST_WAIT_READ_RP     = 0x12,
        ST_REFRESH_WAIT     = 0x13,

        // Plain command / load states
        ST_WR_1             = 0x20,
        ST_RD_1             = 0x21,
        ST_WAIT_READ_DATA   = 0x22,
        ST_REFRESH_CMD      = 0x23,
        ST_MPC_DLL          = 0x24,
        ST_MPC_ZQ_START     = 0x25,
        ST_MPC_ZQ_LATCH     = 0x26,

        // MRD-load states
        ST_MR0_1 = 0x40, ST_MR2_1 = 0x41, ST_MR3_1 = 0x42, ST_MR4_1 = 0x43,
        ST_MR5_1 = 0x44, ST_MR6_1 = 0x45, ST_MR8_1 = 0x46, ST_MR10_1 = 0x47,
        ST_MPC_TDLLK        = 0x48,
        ST_MPC_SET1N        = 0x49,

        // Init wait-down states
        ST_INIT4_EXIT       = 0x80,
        ST_INIT5_NOP        = 0x81,
        ST_XPR_WAIT         = 0x82,
        ST_MPC_TDLLK_WAIT   = 0x83,
        ST_DLLK_WAIT        = 0x84,
        ST_ZQCAL_WAIT       = 0x85,
        ST_ZQLAT_WAIT       = 0x86,
        ST_MPC_SET1N_WAIT   = 0x87,
        ST_MR0_WAIT = 0x88, ST_MR2_WAIT = 0x89, ST_MR3_WAIT = 0x8A,
        ST_MR4_WAIT = 0x8B, ST_MR5_WAIT = 0x8C, ST_MR6_WAIT = 0x8D,
        ST_MR8_WAIT = 0x8E, ST_MR10_WAIT = 0x8F
    };

    state_t state_r;

    // Host request FIFO. Explicit register slots are used instead of
    // dynamically indexed C/C++ arrays so CWB manual scheduling does not
    // infer pipeline memory for the queue.
    static const unsigned QUEUE_DEPTH = 8;

    //stores the queue elements
    sc_uint<26> q_addr0, q_addr1, q_addr2, q_addr3;
    sc_uint<26> q_addr4, q_addr5, q_addr6, q_addr7;
    sc_biguint<512> q_wdata0, q_wdata1, q_wdata2, q_wdata3;
    sc_biguint<512> q_wdata4, q_wdata5, q_wdata6, q_wdata7;
    sc_uint<64> q_wmask0, q_wmask1, q_wmask2, q_wmask3;
    sc_uint<64> q_wmask4, q_wmask5, q_wmask6, q_wmask7;
    bool q_write0, q_write1, q_write2, q_write3;
    bool q_write4, q_write5, q_write6, q_write7;

    //circular queue pointers
    sc_uint<3> q_head_r;
    sc_uint<3> q_tail_r;
    sc_uint<4> q_count_r;

    // Split countdown timers for HLS.
    // Long initialization delays are isolated from the short operational waits
    // so the single-cycle manual schedule does not build one large state-
    // dependent mux into a 23-bit register.
    sc_uint<23> long_wait_count_r; // T_INIT1 / T_INIT3 only
    sc_uint<12> init_wait_count_r; // T_INIT4, XPR, MRD, DLLK, ZQCAL, ZQLAT
    sc_uint<9>  op_wait_count_r;   // tRCD, write completion, tRP, tRFC1

    sc_uint<13> refresh_count_r;   // countdown timer, max = 6,240
    sc_uint<3>  mpc_count_r;       // max = 4

    bool refresh_due_r;

    sc_uint<26> addr_r;
    sc_biguint<512> wdata_r;
    sc_uint<64> wmask_r;
    bool write_r;

    sc_uint<16> row_r;
    sc_uint<2> bg_r;
    sc_uint<2> ba_r;
    sc_uint<6> col_r;

    void drive_des();
    void drive_nop();
    void drive_mpc(sc_uint<8> op);
    void drive_mrw0(sc_uint<8> mra);
    void drive_mrw1(sc_uint<8> op);
    void drive_act0();
    void drive_act1();
    void drive_wr0();
    void drive_wr1();
    void drive_rd0();
    void drive_rd1();
    void drive_refab();

    void controller_thread();

    SC_CTOR(ddr5_controller_x32) {
        SC_CTHREAD(controller_thread, clk.pos());
        reset_signal_is(rst, true);
    }
};

#endif
