// Aggregate CWB input for two independent x32 DDR5 subchannels.
// Behavioral memory models and testbench are intentionally excluded.
#include "ddr5_controller_x32.cpp"
#include "ddr5_write_phy_pos.cpp"
#include "ddr5_write_phy_neg.cpp"
#include "ddr5_write_phy_output_mux.cpp"
#include "ddr5_read_phy_pos.cpp"
#include "ddr5_read_phy_neg.cpp"
#include "ddr5_x32_write_phy.cpp"
#include "ddr5_x32_read_phy.cpp"
#include "ddr5_x32_integrated.h"
#include "ddr5_dual_x32_router.h"
#include "ddr5_dual_x32_integrated.h"
