#include "ddr5_x32_read_phy.h"
void ddr5_x32_read_phy::split_and_merge() {
    sc_uint<32> q = dq_in.read();
    lane0_dq_s.write(q.range(15,0));
    lane1_dq_s.write(q.range(31,16));

    sc_biguint<256> l0=lane0_data_s.read(), l1=lane1_data_s.read();
    sc_biguint<512> d=0;
#define MERGE_BEAT(i) \
    d.range((i)*32+15,(i)*32)=l0.range((i)*16+15,(i)*16); \
    d.range((i)*32+31,(i)*32+16)=l1.range((i)*16+15,(i)*16)
    MERGE_BEAT(0);  MERGE_BEAT(1);  MERGE_BEAT(2);  MERGE_BEAT(3);
    MERGE_BEAT(4);  MERGE_BEAT(5);  MERGE_BEAT(6);  MERGE_BEAT(7);
    MERGE_BEAT(8);  MERGE_BEAT(9);  MERGE_BEAT(10); MERGE_BEAT(11);
    MERGE_BEAT(12); MERGE_BEAT(13); MERGE_BEAT(14); MERGE_BEAT(15);
#undef MERGE_BEAT
    rd_data.write(d);
    rd_valid.write(lane0_valid_s.read() && lane1_valid_s.read());
    busy.write(lane0_busy_s.read() || lane1_busy_s.read());
    capture_en.write(lane0_cap_s.read() && lane1_cap_s.read());
}
