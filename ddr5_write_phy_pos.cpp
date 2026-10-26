#include "ddr5_write_phy_pos.h"

// Logical DDR5-3200 x16 BL16 write PHY.
// Controller contract: wr_start carries a complete 256-bit burst and 32 byte masks.
// DDR side: 16 x16 beats, one on every CK edge. WL=24 CK.
// The controller and this PHY are separate posedge CTHREADs, so wr_start is observed
// one CK after the controller asserts it. Waiting WL-1=23 more CK places beat0 at
// WRITE + 24 CK. MR8 in the project configuration selects a 2 CK write preamble.

#define W_IDLE   0
#define W_WAIT   1
#define W_B0     2
#define W_B1     3
#define W_B2     4
#define W_B3     5
#define W_B4     6
#define W_B5     7
#define W_B6     8
#define W_B7     9
#define W_POST  10

static sc_uint<16> wbeat16(const sc_biguint<256>& v, unsigned beat)
{
    return (sc_uint<16>)v.range(beat*16+15, beat*16);
}

static sc_uint<2> wdm_n(const sc_uint<32>& mask, unsigned beat)
{
    sc_uint<2> m;
    // Controller mask convention: 1 = mask this byte. DDR logical DM_n is active low.
    m[0] = mask[beat*2]   ? 0 : 1;
    m[1] = mask[beat*2+1] ? 0 : 1;
    return m;
}

void ddr5_write_phy_pos::run()
{
    state=W_IDLE; wait_count=0; data_reg=0; mask_reg=0;
    rise_dq.write(0); rise_dm_n.write(3); rise_dqs.write(0);
    fall_dq_next.write(0); fall_dm_n_next.write(3); fall_dqs_next.write(0);
    dq_oe.write(false); dqs_oe.write(false); busy.write(false); done.write(false);
    wait();

    while (1) {
        done.write(false);

        if (state==W_IDLE) {
            rise_dq.write(0); rise_dm_n.write(3); rise_dqs.write(0);
            fall_dq_next.write(0); fall_dm_n_next.write(3); fall_dqs_next.write(0);
            dq_oe.write(false); dqs_oe.write(false); busy.write(false);
            if (wr_start.read()) {
                data_reg=wr_data.read(); mask_reg=wr_mask.read();
                wait_count=23; // WL-1 because wr_start is observed one CK later.
                state=W_WAIT; busy.write(true);
            }
        } else if (state==W_WAIT) {
            rise_dq.write(0); rise_dm_n.write(3); rise_dqs.write(0);
            fall_dq_next.write(0); fall_dm_n_next.write(3); fall_dqs_next.write(0);
            dq_oe.write(false); busy.write(true);
            // Logical 2 CK DQS preamble immediately before beat0.
            dqs_oe.write(wait_count<=2);
            if (wait_count<=1) {
                rise_dq.write(wbeat16(data_reg,0)); rise_dm_n.write(wdm_n(mask_reg,0)); rise_dqs.write(3);
                fall_dq_next.write(wbeat16(data_reg,1)); fall_dm_n_next.write(wdm_n(mask_reg,1)); fall_dqs_next.write(0);
                dq_oe.write(true); dqs_oe.write(true); state=W_B0; wait_count=0;
            } else wait_count=wait_count-1;
        } else if (state==W_B0) {
            rise_dq.write(wbeat16(data_reg,2)); rise_dm_n.write(wdm_n(mask_reg,2)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,3)); fall_dm_n_next.write(wdm_n(mask_reg,3)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B1;
        } else if (state==W_B1) {
            rise_dq.write(wbeat16(data_reg,4)); rise_dm_n.write(wdm_n(mask_reg,4)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,5)); fall_dm_n_next.write(wdm_n(mask_reg,5)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B2;
        } else if (state==W_B2) {
            rise_dq.write(wbeat16(data_reg,6)); rise_dm_n.write(wdm_n(mask_reg,6)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,7)); fall_dm_n_next.write(wdm_n(mask_reg,7)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B3;
        } else if (state==W_B3) {
            rise_dq.write(wbeat16(data_reg,8)); rise_dm_n.write(wdm_n(mask_reg,8)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,9)); fall_dm_n_next.write(wdm_n(mask_reg,9)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B4;
        } else if (state==W_B4) {
            rise_dq.write(wbeat16(data_reg,10)); rise_dm_n.write(wdm_n(mask_reg,10)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,11)); fall_dm_n_next.write(wdm_n(mask_reg,11)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B5;
        } else if (state==W_B5) {
            rise_dq.write(wbeat16(data_reg,12)); rise_dm_n.write(wdm_n(mask_reg,12)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,13)); fall_dm_n_next.write(wdm_n(mask_reg,13)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B6;
        } else if (state==W_B6) {
            rise_dq.write(wbeat16(data_reg,14)); rise_dm_n.write(wdm_n(mask_reg,14)); rise_dqs.write(3);
            fall_dq_next.write(wbeat16(data_reg,15)); fall_dm_n_next.write(wdm_n(mask_reg,15)); fall_dqs_next.write(0);
            dq_oe.write(true); dqs_oe.write(true); busy.write(true); state=W_B7;
        } else if (state==W_B7) {
            // Falling beat15 occurred in the preceding half-cycle.
            rise_dq.write(0); rise_dm_n.write(3); rise_dqs.write(0);
            fall_dq_next.write(0); fall_dm_n_next.write(3); fall_dqs_next.write(0);
            dq_oe.write(false); dqs_oe.write(true); busy.write(true); state=W_POST;
        } else {
            rise_dq.write(0); rise_dm_n.write(3); rise_dqs.write(0);
            fall_dq_next.write(0); fall_dm_n_next.write(3); fall_dqs_next.write(0);
            dq_oe.write(false); dqs_oe.write(false); busy.write(false); done.write(true); state=W_IDLE;
        }
        wait();
    }
}
