#include "ddr5_x32_write_phy.h"
void ddr5_x32_write_phy::split_and_combine() {
    sc_biguint<512> d = wr_data.read();
    sc_uint<64> m = wr_mask.read();
    sc_biguint<256> l0 = 0, l1 = 0;
    sc_uint<32> m0 = 0, m1 = 0;
#define SPLIT_BEAT(i) \
    l0.range((i)*16+15,(i)*16)=d.range((i)*32+15,(i)*32); \
    l1.range((i)*16+15,(i)*16)=d.range((i)*32+31,(i)*32+16); \
    m0.range((i)*2+1,(i)*2)=m.range((i)*4+1,(i)*4); \
    m1.range((i)*2+1,(i)*2)=m.range((i)*4+3,(i)*4+2)
    SPLIT_BEAT(0);  SPLIT_BEAT(1);  SPLIT_BEAT(2);  SPLIT_BEAT(3);
    SPLIT_BEAT(4);  SPLIT_BEAT(5);  SPLIT_BEAT(6);  SPLIT_BEAT(7);
    SPLIT_BEAT(8);  SPLIT_BEAT(9);  SPLIT_BEAT(10); SPLIT_BEAT(11);
    SPLIT_BEAT(12); SPLIT_BEAT(13); SPLIT_BEAT(14); SPLIT_BEAT(15);
#undef SPLIT_BEAT
    lane0_data_s.write(l0); lane1_data_s.write(l1);
    lane0_mask_s.write(m0); lane1_mask_s.write(m1);

    sc_uint<32> dq=0; dq.range(15,0)=lane0_dq_s.read(); dq.range(31,16)=lane1_dq_s.read(); dq_out.write(dq);
    sc_uint<4> dm=0; dm.range(1,0)=lane0_dm_s.read(); dm.range(3,2)=lane1_dm_s.read(); dm_n_out.write(dm);
    sc_uint<4> ds=0; ds.range(1,0)=lane0_dqs_s.read(); ds.range(3,2)=lane1_dqs_s.read(); dqs_out.write(ds);
    dq_oe.write(lane0_dq_oe_s.read() && lane1_dq_oe_s.read());
    dqs_oe.write(lane0_dqs_oe_s.read() && lane1_dqs_oe_s.read());
    busy.write(lane0_busy_s.read() || lane1_busy_s.read());
    done.write(lane0_done_s.read() && lane1_done_s.read());
}
