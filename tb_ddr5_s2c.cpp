#include "tb_ddr5_s2c.h"

void tb_ddr5_dual_x32_integrated::trace_clock_gate() {
        // Exclude the ~6.7 million initialization cycles from the VCD clock trace.
        trace_clk.write(init_done.read() ? clk.read() : false);
    }

sc_biguint<512> tb_ddr5_dual_x32_integrated::make_pattern(unsigned seed) {
        sc_biguint<512> result = 0;
        for (unsigned i=0; i<16; ++i) {
            sc_uint<32> word = (sc_uint<32>)((seed << 24) ^ (i * 0x01010101u) ^ 0x5A3210E7u);
            result.range(i*32+31,i*32) = word;
        }
        return result;
    }

bool tb_ddr5_dual_x32_integrated::enqueue(bool subch, bool wr, sc_uint<26> local_addr,
                 sc_biguint<512> data, sc_uint<64> mask) {
        // Drive request data at falling edges and hold all fields while stalled.
        wait(clk.negedge_event());
        sc_uint<27> a=local_addr;
        a[26]=subch;
        req_addr.write(a);
        req_write.write(wr);
        req_wdata.write(data);
        req_wmask.write(mask);
        req_valid.write(true);
        for (unsigned n=0;n<8;++n) wait(SC_ZERO_TIME);
        for (;;) {
            bool ready=req_ready.read();
            wait(clk.posedge_event());
            for (unsigned n=0;n<8;++n) wait(SC_ZERO_TIME);
            if (ready) break;
            wait(clk.negedge_event());
            for (unsigned n=0;n<8;++n) wait(SC_ZERO_TIME);
        }
        wait(clk.negedge_event());
        req_valid.write(false);
        return true;
    }

void tb_ddr5_dual_x32_integrated::overlap_monitor() {
        overlap_seen = false;
        while (true) {
            wait(clk.posedge_event() | clk.negedge_event());
            for (unsigned n=0;n<8;++n) wait(SC_ZERO_TIME);
            if (sc0_dq_oe.read() && sc1_dq_oe.read()) overlap_seen = true;
        }
    }

void tb_ddr5_dual_x32_integrated::recv() {
        FILE *out = fopen("ddr_output.txt", "wt");
        if (!out) { std::cerr << "Cannot create ddr_output.txt" << std::endl; ++errors; return; }
        while (true) {
            wait(clk.posedge_event());
            for (unsigned n=0;n<8;++n) wait(SC_ZERO_TIME);
            if (rsp_valid.read()) {
                unsigned ch=rsp_subch.read()?1u:0u;
                const std::string hex=rsp_rdata.read().to_string(SC_HEX);
                fprintf(out,"%u %s\n",ch,hex.c_str());
                fflush(out);
                ++received_reads;
            }
        }
    }

std::string tb_ddr5_dual_x32_integrated::canonical_hex(const char *s) {
        std::string r(s);
        if (r.size() >= 2 && r[0]=='0' && (r[1]=='x' || r[1]=='X')) r.erase(0,2);
        for (unsigned i=0;i<r.size();++i)
            if (r[i]>='A' && r[i]<='F') r[i]=(char)(r[i]-'A'+'a');
        std::string::size_type nonzero=r.find_first_not_of('0');
        return nonzero==std::string::npos ? "0" : r.substr(nonzero);
    }

bool tb_ddr5_dual_x32_integrated::compare_results() {
        FILE *got=fopen("ddr_output.txt","rt");
        FILE *gold=fopen("ddr_output_golden.txt","rt");
        FILE *diff=fopen("ddrdiff.txt","wt");
        if (!got || !gold || !diff) {
            std::cerr << "Could not open actual/golden/difference files" << std::endl;
            if(got)fclose(got);if(gold)fclose(gold);if(diff)fclose(diff);
            return false;
        }
        std::string actual[2],expected[2];
        unsigned counts_actual[2]={0,0},counts_expected[2]={0,0};
        unsigned ch=0;
        char data[256];
        bool ok=true;
        while(fscanf(got,"%u %255s",&ch,data)==2) {
            if(ch>1) {ok=false;continue;}
            ++counts_actual[ch];actual[ch]=canonical_hex(data);
        }
        while(fscanf(gold,"%u %255s",&ch,data)==2) {
            if(ch>1) {ok=false;continue;}
            ++counts_expected[ch];expected[ch]=canonical_hex(data);
        }
        for(unsigned i=0;i<2;++i) {
            if(counts_actual[i]!=counts_expected[i] || actual[i]!=expected[i]) {
                fprintf(diff,"Subchannel %u mismatch: count(actual)=%u count(golden)=%u\n  actual: %s\n  golden: %s\n",
                    i,counts_actual[i],counts_expected[i],actual[i].c_str(),expected[i].c_str());
                ok=false;
            }
        }
        fclose(got);fclose(gold);fclose(diff);
        return ok;
    }

void tb_ddr5_dual_x32_integrated::send() {
        rst.write(true);req_valid.write(false);req_write.write(false);
        req_addr.write(0);req_wdata.write(0);req_wmask.write(0);
        done=false;pass=false;expected_reads=0;received_reads=0;errors=0;
        wait(5,SC_NS);
        rst.write(false);
        while (!init_done.read()) wait(clk.posedge_event());
        std::cout << "DDR5 FCFS both subchannels initialized @ " << sc_time_stamp() << std::endl;

        FILE *input=fopen("ddr_in_data.txt","rt");
        if (!input) {
            std::cerr << "Cannot open ddr_in_data.txt" << std::endl;
            ++errors;done=true;sc_stop();return;
        }
        // S2CBench-style input: op(1=WRITE/0=READ) subch local_address seed mask_hex
        unsigned op,ch,addr,seed;
        unsigned long long mask;
        while (fscanf(input,"%u %u %u %u %llx",&op,&ch,&addr,&seed,&mask)==5) {
            if (op>1 || ch>1 || addr >= (1u<<26)) {
                std::cerr << "Invalid input request" << std::endl;
                ++errors;break;
            }
            if(!op)++expected_reads;
            enqueue(ch!=0,op!=0,(sc_uint<26>)addr,make_pattern(seed),(sc_uint<64>)mask);
        }
        fclose(input);

        // Response stream uses one cycle per captured response, with a guard timeout.
        unsigned wait_cycles=0;
        while(received_reads<expected_reads && wait_cycles<200000) {
            wait(clk.posedge_event());
            ++wait_cycles;
        }
        for (unsigned n=0;n<12;++n) wait(clk.posedge_event());
        if (received_reads!=expected_reads) {
            std::cerr << "Read timeout: expected " << expected_reads
                      << " received " << received_reads << std::endl;
            ++errors;
        }
        if (!overlap_seen) {
            std::cerr << "Subchannel overlap not observed" << std::endl;
            ++errors;
        }
        pass=(errors==0 && compare_results());
        done=true;
        std::cout << (pass?"Finished simulation SUCCESSFULLY":"DDR5 FCFS S2C BENCH FAIL") << std::endl;
        sc_stop();
    }
