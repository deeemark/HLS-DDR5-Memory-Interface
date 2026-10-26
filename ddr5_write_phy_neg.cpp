#include "ddr5_write_phy_neg.h"
void ddr5_write_phy_neg::run() {
    fall_dq.write(0); fall_dm_n.write(3); fall_dqs.write(0); wait();
    while (1) {
        fall_dq.write(fall_dq_next.read());
        fall_dm_n.write(fall_dm_n_next.read());
        fall_dqs.write(fall_dqs_next.read());
        wait();
    }
}
