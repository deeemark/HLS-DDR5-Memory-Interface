#include "ddr5_controller_x32.h"

void ddr5_controller_x32::drive_des() {
    ddr_cs_n.write(true);
    ddr_ca.write(0);
}

void ddr5_controller_x32::drive_nop() {
    sc_uint<14> ca = 0;
    ca[0]=1; ca[1]=1; ca[2]=1; ca[3]=1; ca[4]=1;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_mpc(sc_uint<8> op) {
    sc_uint<14> ca = 0;
    ca[0]=1; ca[1]=1; ca[2]=1; ca[3]=1; ca[4]=0;
    for (int i=0; i<8; ++i) ca[5+i] = op[i];
    ca[13]=0;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_mrw0(sc_uint<8> mra) {
    sc_uint<14> ca = 0;
    ca[0]=1; ca[1]=0; ca[2]=1; ca[3]=0; ca[4]=0;
    for (int i=0; i<8; ++i) ca[5+i] = mra[i];
    ca[13]=0;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_mrw1(sc_uint<8> op) {
    sc_uint<14> ca = 0;
    for (int i=0; i<8; ++i) ca[i] = op[i];
    ca[10] = 0;
    ddr_cs_n.write(true);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_act0() {
    sc_uint<14> ca = 0;
    ca[0]=0; ca[1]=0;
    ca[2]=row_r[0]; ca[3]=row_r[1]; ca[4]=row_r[2]; ca[5]=row_r[3];
    ca[6]=ba_r[0]; ca[7]=ba_r[1];
    ca[8]=bg_r[0]; ca[9]=bg_r[1];
    ca[10]=0;
    ca[11]=0; ca[12]=0; ca[13]=0;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_act1() {
    sc_uint<14> ca = 0;
    ca[0]=row_r[4];  ca[1]=row_r[5];
    ca[2]=row_r[6];  ca[3]=row_r[7];
    ca[4]=row_r[8];  ca[5]=row_r[9];
    ca[6]=row_r[10]; ca[7]=row_r[11];
    ca[8]=row_r[12]; ca[9]=row_r[13];
    ca[10]=row_r[14]; ca[11]=row_r[15];
    ca[12]=0; ca[13]=0;
    ddr_cs_n.write(true);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_wr0() {
    sc_uint<14> ca = 0;
    ca[0]=1; ca[1]=0; ca[2]=1; ca[3]=1; ca[4]=0;
    ca[5]=1;
    ca[6]=ba_r[0]; ca[7]=ba_r[1];
    ca[8]=bg_r[0]; ca[9]=bg_r[1];
    ca[10]=0; ca[11]=0; ca[12]=0; ca[13]=0;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_wr1() {
    sc_uint<14> ca = 0;
    ca[0]=0;
    ca[1]=0;
    ca[2]=col_r[0];
    ca[3]=col_r[1];
    ca[4]=col_r[2];
    ca[5]=col_r[3];
    ca[6]=col_r[4];
    ca[7]=col_r[5];
    ca[8]=0;
    ca[9]=0;
    ca[10]=0;
    ca[11]=0;
    ca[12]=0; ca[13]=0;
    ddr_cs_n.write(true);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_rd0() {
    sc_uint<14> ca = 0;
    ca[0]=1; ca[1]=0; ca[2]=1; ca[3]=1; ca[4]=1;
    ca[5]=1;
    ca[6]=ba_r[0]; ca[7]=ba_r[1];
    ca[8]=bg_r[0]; ca[9]=bg_r[1];
    ca[10]=0; ca[11]=0; ca[12]=0; ca[13]=0;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_rd1() {
    sc_uint<14> ca = 0;
    ca[0]=0;
    ca[1]=0;
    ca[2]=col_r[0];
    ca[3]=col_r[1];
    ca[4]=col_r[2];
    ca[5]=col_r[3];
    ca[6]=col_r[4];
    ca[7]=col_r[5];
    ca[8]=0;
    ca[9]=0;
    ca[10]=0;
    ca[11]=0; ca[12]=0; ca[13]=0;
    ddr_cs_n.write(true);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::drive_refab() {
    sc_uint<14> ca = 0;
    ca[0]=1; ca[1]=1; ca[2]=0; ca[3]=0; ca[4]=1;
    ca[5]=0;
    ca[6]=0; ca[7]=0;
    ca[8]=0;
    ca[9]=1;
    ca[10]=0;
    ca[11]=0; ca[12]=0; ca[13]=0;
    ddr_cs_n.write(false);
    ddr_ca.write(ca);
}

void ddr5_controller_x32::controller_thread() {
    req_ready.write(false);
    rsp_valid.write(false);
    rsp_rdata.write(0);
    init_done.write(false);

    ddr_reset_n.write(false);
    drive_des();

    wr_start.write(false);
    wr_data.write(0);
    wr_mask.write(0);
    rd_start.write(false);

    state_r = ST_RESET_LOW;

    // Countdown timers. A wait state transitions when its counter reaches 0.
    long_wait_count_r = T_INIT1 - 1;
    init_wait_count_r = 0;
    op_wait_count_r = 0;
    refresh_count_r = T_REFI - 1;
    mpc_count_r = 0;
    refresh_due_r = false;

    addr_r = 0;
    wdata_r = 0;
    wmask_r = 0;
    write_r = false;

    row_r = 0;
    bg_r = 0;
    ba_r = 0;
    col_r = 0;

    q_head_r = 0;
    q_tail_r = 0;
    q_count_r = 0;

    q_addr0 = 0; q_addr1 = 0; q_addr2 = 0; q_addr3 = 0;
    q_addr4 = 0; q_addr5 = 0; q_addr6 = 0; q_addr7 = 0;
    q_wdata0 = 0; q_wdata1 = 0; q_wdata2 = 0; q_wdata3 = 0;
    q_wdata4 = 0; q_wdata5 = 0; q_wdata6 = 0; q_wdata7 = 0;
    q_wmask0 = 0; q_wmask1 = 0; q_wmask2 = 0; q_wmask3 = 0;
    q_wmask4 = 0; q_wmask5 = 0; q_wmask6 = 0; q_wmask7 = 0;
    q_write0 = false; q_write1 = false; q_write2 = false; q_write3 = false;
    q_write4 = false; q_write5 = false; q_write6 = false; q_write7 = false;

    wait();

    while (true) {
        state_t next_state = state_r;

        bool long_wait_load = false;
        sc_uint<23> long_wait_load_value = 0;
        bool long_wait_dec = false;

        bool refresh_load = false;
        sc_uint<13> refresh_load_value = 0;
        bool refresh_dec = false;
        bool set_refresh_due = false;
        bool clear_refresh_due = false;

        bool mpc_clear = false;
        bool mpc_inc = false;

        // FIFO actions for this controller cycle.
        bool fifo_push = false;
        bool fifo_pop = false;

        // ---- init_wait_count_r control: derived from state class bits ----
        const unsigned st_bits = (unsigned)state_r;
        const bool iw_zero  = (init_wait_count_r == 0);
        const bool mpc_hit  = (mpc_count_r == T_MPC_CS-1);
        const bool cls_wait = (st_bits & 0x80) != 0;            // wait-down state
        const bool cls_mrd  = (st_bits & 0x40) != 0;            // loads T_MRD-1
        const bool long_zero = (long_wait_count_r == 0);

        // Load qualifiers (mutually exclusive; each state belongs to one).
        // The counter is idle (not read) in INIT3 and in all MPC/MR command
        // states, so it is reloaded with the same constant on EVERY cycle of
        // those states.  The value present on the transition cycle is
        // identical to before, but the load no longer waits on the 23-bit
        // long_wait_count_r==0 compare or on mpc_count_r.
        const bool ld_mrd   = cls_mrd;                          // MR*_1, MPC_TDLLK, MPC_SET1N
        const bool ld_init4 = (state_r == ST_INIT3_CSLOW);
        const bool ld_init5 = (state_r == ST_INIT4_EXIT)   && iw_zero;
        const bool ld_xpr   = (state_r == ST_INIT5_NOP)    && iw_zero;
        const bool ld_dllk  = (state_r == ST_MPC_DLL);
        const bool ld_zqcal = (state_r == ST_MPC_ZQ_START);
        const bool ld_zqlat = (state_r == ST_MPC_ZQ_LATCH);
        const bool iw_load  = ((ld_mrd || ld_init4) || (ld_init5 || ld_xpr)) ||
                              ((ld_dllk || ld_zqcal) || ld_zqlat);
        const bool iw_dec   = cls_wait && !iw_zero;

        // op_wait_count_r control
        // op_wait_count_r is idle in ACT_1 / WR_1 / WAIT_READ_DATA /
        // REFRESH_CMD, so it reloads its constant on every cycle there.
        // T_RCD-1 == T_RP-1 == 25, so ACT_1 and WAIT_READ_DATA share one term.
        const bool op_zero   = (op_wait_count_r == 0);
        const bool op_cls    = (st_bits & 0x10) != 0;           // op wait-down state
        const bool ld_op25   = (state_r == ST_ACT_1) || (state_r == ST_WAIT_READ_DATA);
        const bool ld_op105  = (state_r == ST_WR_1);
        const bool ld_op471  = (state_r == ST_REFRESH_CMD);
        const bool op_load   = (ld_op25 || ld_op105) || ld_op471;
        const bool op_dec    = op_cls && !op_zero;

        rsp_valid.write(false);
        wr_start.write(false);
        rd_start.write(false);
        drive_des();

        // This edge consumes a request iff the FIFO has capacity. The externally
        // visible req_ready for this edge was published at the PREVIOUS edge,
        // after the previous queue update. Do not write req_ready here using
        // the old count: that would make it one transaction behind at full/empty
        // boundaries and violate ready/valid acceptance.
        if (init_done.read() && q_count_r < QUEUE_DEPTH && req_valid.read())
            fifo_push = true;

        // Periodic refresh countdown.  No wide threshold compare is needed.
        if (init_done.read() && !refresh_due_r) {
            if (refresh_count_r == 0) {
                set_refresh_due = true;
            } else {
                refresh_dec = true;
            }
        }

        switch (state_r) {
        case ST_RESET_LOW:
            ddr_reset_n.write(false);
            ddr_cs_n.write(false);

            if (long_wait_count_r == 0) {
                long_wait_load = true;
                long_wait_load_value = T_INIT3 - 1;
                next_state = ST_INIT3_CSLOW;
            } else {
                long_wait_dec = true;
            }
            break;

        case ST_INIT3_CSLOW:
            ddr_reset_n.write(true);
            ddr_cs_n.write(false);

            if (long_wait_count_r == 0) {
                next_state = ST_INIT4_EXIT;
            } else {
                long_wait_dec = true;
            }
            break;

        case ST_INIT4_EXIT:
            ddr_reset_n.write(true);
            ddr_cs_n.write(true);
            ddr_ca.write(0x3FFF);

            if (iw_zero) {
                next_state = ST_INIT5_NOP;
            }
            break;

        case ST_INIT5_NOP:
            drive_nop();

            if (iw_zero) {
                next_state = ST_XPR_WAIT;
            }
            break;

        case ST_XPR_WAIT:
            if (iw_zero) {
                mpc_clear = true;
                next_state = ST_MPC_TDLLK;
            }
            break;

        case ST_MPC_TDLLK:
            drive_mpc(MPC_TDLLK_3200);
            if (mpc_hit) {
                mpc_clear = true;
                next_state = ST_MPC_TDLLK_WAIT;
            } else {
                mpc_inc = true;
            }
            break;

        case ST_MPC_TDLLK_WAIT:
            if (iw_zero) {
                mpc_clear = true;
                next_state = ST_MPC_DLL;
            }
            break;

        case ST_MPC_DLL:
            drive_mpc(MPC_DLL_RESET);
            if (mpc_hit) {
                mpc_clear = true;
                next_state = ST_DLLK_WAIT;
            } else {
                mpc_inc = true;
            }
            break;

        case ST_DLLK_WAIT:
            if (iw_zero) {
                mpc_clear = true;
                next_state = ST_MPC_ZQ_START;
            }
            break;

        case ST_MPC_ZQ_START:
            drive_mpc(MPC_ZQ_START);
            if (mpc_hit) {
                mpc_clear = true;
                next_state = ST_ZQCAL_WAIT;
            } else {
                mpc_inc = true;
            }
            break;

        case ST_ZQCAL_WAIT:
            if (iw_zero) {
                mpc_clear = true;
                next_state = ST_MPC_ZQ_LATCH;
            }
            break;

        case ST_MPC_ZQ_LATCH:
            drive_mpc(MPC_ZQ_LATCH);
            if (mpc_hit) {
                mpc_clear = true;
                next_state = ST_ZQLAT_WAIT;
            } else {
                mpc_inc = true;
            }
            break;

        case ST_ZQLAT_WAIT:
            if (iw_zero) {
                mpc_clear = true;
                next_state = ST_MPC_SET1N;
            }
            break;

        case ST_MPC_SET1N:
            drive_mpc(MPC_SET_1N);
            if (mpc_hit) {
                mpc_clear = true;
                next_state = ST_MPC_SET1N_WAIT;
            } else {
                mpc_inc = true;
            }
            break;

        case ST_MPC_SET1N_WAIT:
            if (iw_zero) {
                next_state = ST_MR0_0;
            }
            break;

        case ST_MR0_0:
            drive_mrw0(0x00);
            next_state = ST_MR0_1;
            break;
        case ST_MR0_1:
            drive_mrw1(MR0_VALUE);
            next_state = ST_MR0_WAIT;
            break;
        case ST_MR0_WAIT:
            if (iw_zero) next_state = ST_MR2_0;
            break;

        case ST_MR2_0:
            drive_mrw0(0x02);
            next_state = ST_MR2_1;
            break;
        case ST_MR2_1:
            drive_mrw1(MR2_VALUE);
            next_state = ST_MR2_WAIT;
            break;
        case ST_MR2_WAIT:
            if (iw_zero) next_state = ST_MR3_0;
            break;

        case ST_MR3_0:
            drive_mrw0(0x03);
            next_state = ST_MR3_1;
            break;
        case ST_MR3_1:
            drive_mrw1(MR3_VALUE);
            next_state = ST_MR3_WAIT;
            break;
        case ST_MR3_WAIT:
            if (iw_zero) next_state = ST_MR4_0;
            break;

        case ST_MR4_0:
            drive_mrw0(0x04);
            next_state = ST_MR4_1;
            break;
        case ST_MR4_1:
            drive_mrw1(MR4_VALUE);
            next_state = ST_MR4_WAIT;
            break;
        case ST_MR4_WAIT:
            if (iw_zero) next_state = ST_MR5_0;
            break;

        case ST_MR5_0:
            drive_mrw0(0x05);
            next_state = ST_MR5_1;
            break;
        case ST_MR5_1:
            drive_mrw1(MR5_VALUE);
            next_state = ST_MR5_WAIT;
            break;
        case ST_MR5_WAIT:
            if (iw_zero) next_state = ST_MR6_0;
            break;

        case ST_MR6_0:
            drive_mrw0(0x06);
            next_state = ST_MR6_1;
            break;
        case ST_MR6_1:
            drive_mrw1(MR6_VALUE);
            next_state = ST_MR6_WAIT;
            break;
        case ST_MR6_WAIT:
            if (iw_zero) next_state = ST_MR8_0;
            break;

        case ST_MR8_0:
            drive_mrw0(0x08);
            next_state = ST_MR8_1;
            break;
        case ST_MR8_1:
            drive_mrw1(MR8_VALUE);
            next_state = ST_MR8_WAIT;
            break;
        case ST_MR8_WAIT:
            if (iw_zero) next_state = ST_MR10_0;
            break;

        case ST_MR10_0:
            drive_mrw0(0x0A);
            next_state = ST_MR10_1;
            break;
        case ST_MR10_1:
            drive_mrw1(MR10_VALUE);
            next_state = ST_MR10_WAIT;
            break;
        case ST_MR10_WAIT:
            if (iw_zero) {
                refresh_load = true;
                refresh_load_value = T_REFI - 1;
                clear_refresh_due = true;
                init_done.write(true);
                next_state = ST_READY;
            }
            break;

        case ST_READY:
            init_done.write(true);

            if (refresh_due_r) {
                next_state = ST_REFRESH_CMD;
            } else if (q_count_r != 0) {
                // FCFS baseline: always consume the oldest queued request.
                next_state = ST_CAPTURE;
            }
            break;

        case ST_CAPTURE:
            init_done.write(true);

            // Latch the FIFO head into the existing single-request command
            // registers. Explicit slot selection avoids pipeline-memory
            // inference under CWB manual scheduling.
            switch (q_head_r) {
            case 0:
                addr_r=q_addr0; wdata_r=q_wdata0; wmask_r=q_wmask0; write_r=q_write0; break;
            case 1:
                addr_r=q_addr1; wdata_r=q_wdata1; wmask_r=q_wmask1; write_r=q_write1; break;
            case 2:
                addr_r=q_addr2; wdata_r=q_wdata2; wmask_r=q_wmask2; write_r=q_write2; break;
            case 3:
                addr_r=q_addr3; wdata_r=q_wdata3; wmask_r=q_wmask3; write_r=q_write3; break;
            case 4:
                addr_r=q_addr4; wdata_r=q_wdata4; wmask_r=q_wmask4; write_r=q_write4; break;
            case 5:
                addr_r=q_addr5; wdata_r=q_wdata5; wmask_r=q_wmask5; write_r=q_write5; break;
            case 6:
                addr_r=q_addr6; wdata_r=q_wdata6; wmask_r=q_wmask6; write_r=q_write6; break;
            default:
                addr_r=q_addr7; wdata_r=q_wdata7; wmask_r=q_wmask7; write_r=q_write7; break;
            }

            row_r = addr_r.range(25,10);
            bg_r  = addr_r.range(9,8);
            ba_r  = addr_r.range(7,6);
            col_r = addr_r.range(5,0);

            fifo_pop = true;
            next_state = ST_ACT_0;
            break;

        case ST_ACT_0:
            drive_act0();
            next_state = ST_ACT_1;
            break;

        case ST_ACT_1:
            drive_act1();
            next_state = ST_WAIT_RCD;
            break;

        case ST_WAIT_RCD:
            if (op_zero) {
                next_state = write_r ? ST_WR_0 : ST_RD_0;
            }
            break;

        case ST_WR_0:
            drive_wr0();
            next_state = ST_WR_1;
            break;

        case ST_WR_1:
            drive_wr1();
            wr_data.write(wdata_r);
            wr_mask.write(wmask_r);
            wr_start.write(true);

            next_state = ST_WAIT_WRITE_DONE;
            break;

        case ST_WAIT_WRITE_DONE:
            if (op_zero) {
                next_state = ST_READY;
            }
            break;

        case ST_RD_0:
            drive_rd0();
            next_state = ST_RD_1;
            break;

        case ST_RD_1:
            drive_rd1();
            rd_start.write(true);
            next_state = ST_WAIT_READ_DATA;
            break;

        case ST_WAIT_READ_DATA:
            if (rd_valid.read()) {
                rsp_rdata.write(rd_data.read());
                rsp_valid.write(true);

                next_state = ST_WAIT_READ_RP;
            }
            break;

        case ST_WAIT_READ_RP:
            if (op_zero) {
                next_state = ST_READY;
            }
            break;

        case ST_REFRESH_CMD:
            drive_refab();

            clear_refresh_due = true;
            refresh_load = true;
            refresh_load_value = T_REFI - 1;

            next_state = ST_REFRESH_WAIT;
            break;

        case ST_REFRESH_WAIT:
            if (op_zero) {
                next_state = ST_READY;
            }
            break;

        default:
            next_state = ST_READY;
            break;
        }

        // Single, flat write site for init_wait_count_r: AND-OR constant
        // select (no nested per-state priority mux) then one load/dec select.
        {
            // Per-bit balanced OR trees of the (mutually exclusive) load
            // qualifiers: log2 depth instead of a serial OR chain.
            sc_uint<12> iw_val = 0;
            iw_val[0] = ((ld_init4 || ld_xpr) || (ld_dllk || (ld_zqcal || ld_zqlat)));
            iw_val[1] = ((ld_mrd || (ld_init4 || ld_init5)) || ((ld_xpr || ld_dllk) || (ld_zqcal || ld_zqlat)));
            iw_val[2] = ((ld_mrd || (ld_init4 || ld_xpr)) || (ld_dllk || (ld_zqcal || ld_zqlat)));
            iw_val[3] = ((ld_init4 || ld_dllk) || (ld_zqcal || ld_zqlat));
            iw_val[4] = ((ld_mrd || ld_init4) || (ld_xpr || (ld_dllk || ld_zqcal)));
            iw_val[5] = ((ld_init4 || ld_dllk) || (ld_zqcal || ld_zqlat));
            iw_val[6] = (ld_init4 || (ld_xpr || ld_dllk));
            iw_val[7] = (ld_xpr || ld_dllk);
            iw_val[8] = (ld_xpr || ld_dllk);
            iw_val[9] = (ld_dllk || ld_zqcal);
            iw_val[10] = (ld_init4 || ld_zqcal);
            iw_val[11] = ld_init4;

            if (iw_load)
                init_wait_count_r = iw_val;
            else if (iw_dec)
                init_wait_count_r = init_wait_count_r - 1;
        }

        // One write site per counter, but each counter now serves a much
        // smaller set of states/constants than the old shared 23-bit timer.
        if (long_wait_load)
            long_wait_count_r = long_wait_load_value;
        else if (long_wait_dec)
            long_wait_count_r = long_wait_count_r - 1;

        {
            sc_uint<9> op_val = 0;
            op_val[0] = (ld_op25 || (ld_op105 || ld_op471));
            op_val[1] = ld_op471;
            op_val[2] = ld_op471;
            op_val[3] = (ld_op25 || ld_op105);
            op_val[4] = (ld_op25 || ld_op471);
            op_val[5] = ld_op105;
            op_val[6] = (ld_op105 || ld_op471);
            op_val[7] = ld_op471;
            op_val[8] = ld_op471;

            if (op_load)
                op_wait_count_r = op_val;
            else if (op_dec)
                op_wait_count_r = op_wait_count_r - 1;
        }

        if (mpc_clear)
            mpc_count_r = 0;
        else if (mpc_inc)
            mpc_count_r = mpc_count_r + 1;

        if (refresh_load)
            refresh_count_r = refresh_load_value;
        else if (refresh_dec)
            refresh_count_r = refresh_count_r - 1;

        if (clear_refresh_due)
            refresh_due_r = false;
        else if (set_refresh_due)
            refresh_due_r = true;

        // Request FIFO update. Explicitly handle simultaneous enqueue/dequeue
        // so the queue count changes exactly once per controller cycle.
        if (fifo_push) {
            switch (q_tail_r) {
            case 0:
                q_addr0=req_addr.read(); q_wdata0=req_wdata.read(); q_wmask0=req_wmask.read(); q_write0=req_write.read(); break;
            case 1:
                q_addr1=req_addr.read(); q_wdata1=req_wdata.read(); q_wmask1=req_wmask.read(); q_write1=req_write.read(); break;
            case 2:
                q_addr2=req_addr.read(); q_wdata2=req_wdata.read(); q_wmask2=req_wmask.read(); q_write2=req_write.read(); break;
            case 3:
                q_addr3=req_addr.read(); q_wdata3=req_wdata.read(); q_wmask3=req_wmask.read(); q_write3=req_write.read(); break;
            case 4:
                q_addr4=req_addr.read(); q_wdata4=req_wdata.read(); q_wmask4=req_wmask.read(); q_write4=req_write.read(); break;
            case 5:
                q_addr5=req_addr.read(); q_wdata5=req_wdata.read(); q_wmask5=req_wmask.read(); q_write5=req_write.read(); break;
            case 6:
                q_addr6=req_addr.read(); q_wdata6=req_wdata.read(); q_wmask6=req_wmask.read(); q_write6=req_write.read(); break;
            default:
                q_addr7=req_addr.read(); q_wdata7=req_wdata.read(); q_wmask7=req_wmask.read(); q_write7=req_write.read(); break;
            }
            q_tail_r = q_tail_r + 1;
        }

        if (fifo_pop)
            q_head_r = q_head_r + 1;

        if (fifo_push && !fifo_pop)
            q_count_r = q_count_r + 1;
        else if (!fifo_push && fifo_pop)
            q_count_r = q_count_r - 1;

        state_r = next_state;

        // Registered ready must describe FIFO space available at the NEXT edge.
        // q_count_r now includes this edge's enqueue/dequeue actions.
        req_ready.write(init_done.read() && (q_count_r < QUEUE_DEPTH));

        wait();
    }
}
