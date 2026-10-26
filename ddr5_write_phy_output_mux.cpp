#include "ddr5_write_phy_output_mux.h"
void ddr5_write_phy_output_mux::mux_proc() {
    if (clk.read()) { dq_out.write(rise_dq.read()); dm_n_out.write(rise_dm_n.read()); dqs_out.write(rise_dqs.read()); }
    else { dq_out.write(fall_dq.read()); dm_n_out.write(fall_dm_n.read()); dqs_out.write(fall_dqs.read()); }
}
