CXX=g++
CXXFLAGS=-std=gnu++98 -D_GLIBCXX_USE_CXX11_ABI=0 -O1 -I/proj/cad/cwb-6.1/osci/include
LDFLAGS=-L/proj/cad/cwb-6.1/osci/lib-linux64 -lsystemc -lm
RUNENV=LD_LIBRARY_PATH=/proj/cad/cwb-6.1/osci/lib-linux64:$$LD_LIBRARY_PATH
CWB=/proj/cad/cwb-6.1/packages

X16_PHY_SRCS=ddr5_write_phy_pos.cpp ddr5_write_phy_neg.cpp ddr5_write_phy_output_mux.cpp ddr5_read_phy_pos.cpp ddr5_read_phy_neg.cpp
X32_PHY_SRCS=ddr5_x32_write_phy.cpp ddr5_x32_read_phy.cpp

BDL_OPTS=-EE -c625 -cu1ps -sN \
	-Zflatten_nmux=FSM -Zsche_logic_delay=YES -Zreg_share=NO \
	-Zresource_fcnt=GENERATE -Zresource_mcnt=GENERATE \
	-Zdup_reset=YES -Zfolding_sharing=inter_stage -tcio \
	-lb $(CWB)/asic_45.BLIB -lfl $(CWB)/asic_45.FLIB

all: s2c_test

# S2CBench-style entry points (simulation is separate from CWB synthesis).
S2C_SRCS=ddr5_controller_x32.cpp $(X16_PHY_SRCS) $(X32_PHY_SRCS) ddr5_x32_memory_model.cpp tb_ddr5_s2c.cpp main.cpp

build/ddr5_fcfs_s2c: $(S2C_SRCS) tb_ddr5_s2c.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $(S2C_SRCS) $(LDFLAGS)

s2c_test: build/ddr5_fcfs_s2c
	$(RUNENV) ./build/ddr5_fcfs_s2c

build/ddr5_fcfs_s2c_wave: $(S2C_SRCS) tb_ddr5_s2c.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -DWAVE_DUMP -o $@ $(S2C_SRCS) $(LDFLAGS)

wave: build/ddr5_fcfs_s2c_wave
	$(RUNENV) ./build/ddr5_fcfs_s2c_wave



.PHONY: s2c_test wave synth



# Parse the dual-subchannel design for CWB synthesis.
parse:
	mkdir -p build/dual
	cd build/dual && scpars -EE -incremental=NO -Zboundary_wait=YES -info_base_name scpars ../../ddr5_dual_x32_integrated_cwb.cpp

# Rebuild the already-validated one-subchannel hierarchy bottom-up, then use
# that LMSPEC twice in the dual top.
x16_children: parse
	cd build/dual && bdltran $(BDL_OPTS) ddr5_write_phy_pos.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_write_phy_neg.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_write_phy_output_mux.IFF
	cd build/dual && bdltran $(BDL_OPTS) -ll ddr5_write_phy_pos.LMSPEC -ll ddr5_write_phy_neg.LMSPEC -ll ddr5_write_phy_output_mux.LMSPEC ddr5_write_phy.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_read_phy_pos.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_read_phy_neg.IFF
	cd build/dual && bdltran $(BDL_OPTS) -ll ddr5_read_phy_pos.LMSPEC -ll ddr5_read_phy_neg.LMSPEC ddr5_read_phy.IFF

x32_children: x16_children
	cd build/dual && bdltran $(BDL_OPTS) ddr5_x32_write_phy_split_and_combine.IFF
	cd build/dual && bdltran $(BDL_OPTS) -ll ddr5_write_phy.LMSPEC -ll ddr5_x32_write_phy_split_and_combine.LMSPEC ddr5_x32_write_phy.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_x32_read_phy_split_and_merge.IFF
	cd build/dual && bdltran $(BDL_OPTS) -ll ddr5_read_phy.LMSPEC -ll ddr5_x32_read_phy_split_and_merge.LMSPEC ddr5_x32_read_phy.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_controller_x32.IFF
	cd build/dual && bdltran $(BDL_OPTS) -ll ddr5_controller_x32.LMSPEC -ll ddr5_x32_write_phy.LMSPEC -ll ddr5_x32_read_phy.LMSPEC ddr5_x32_integrated.IFF

# The dual router contains two CWB-visible child processes (SC_METHOD route_req
# and SC_CTHREAD collect_rsp). Synthesize those first, then provide their
# LMSPECs when synthesizing the router parent.
router_children: x32_children
	cd build/dual && bdltran $(BDL_OPTS) ddr5_dual_x32_router_route_req.IFF
	cd build/dual && bdltran $(BDL_OPTS) ddr5_dual_x32_router_collect_rsp.IFF

router_synth: router_children
	cd build/dual && bdltran $(BDL_OPTS) \
		-ll ddr5_dual_x32_router_route_req.LMSPEC \
		-ll ddr5_dual_x32_router_collect_rsp.LMSPEC \
		ddr5_dual_x32_router.IFF

# The dual integrated top also has one local SC_METHOD (combine_init), so it
# must be synthesized before the structural parent for the same reason.
dual_top_children: router_synth
	cd build/dual && bdltran $(BDL_OPTS) ddr5_dual_x32_integrated_combine_init.IFF

# The same validated ddr5_x32_integrated LMSPEC is instantiated twice by the
# top (SC0 and SC1). CWB only needs the module LMSPEC supplied once.
synth: dual_top_children
	cd build/dual && bdltran $(BDL_OPTS) \
		-ll ddr5_x32_integrated.LMSPEC \
		-ll ddr5_dual_x32_router.LMSPEC \
		-ll ddr5_dual_x32_integrated_combine_init.LMSPEC \
		ddr5_dual_x32_integrated.IFF

clean:
	rm -rf build trace_behav.vcd ddr_output.txt ddrdiff.txt

.PHONY: all s2c_test wave parse synth x16_children x32_children router_children router_synth dual_top_children clean
