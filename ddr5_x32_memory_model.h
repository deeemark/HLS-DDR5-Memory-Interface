#ifndef DDR5_X32_MEMORY_MODEL_H
#define DDR5_X32_MEMORY_MODEL_H
#include <systemc.h>
#include <map>

// Behavioral one-subchannel DDR5 model: two x16 devices in lockstep, represented
// as a 32-bit DQ path. Stores 512-bit BL16 transactions (64 bytes).
SC_MODULE(ddr5_x32_memory_model) {
    sc_in<bool> clk,rst,ddr_reset_n,ddr_cs_n;
    sc_in<sc_uint<14> > ddr_ca;
    sc_in<sc_uint<32> > dq_out;
    sc_in<sc_uint<4> > dm_n_out;
    sc_in<bool> dq_oe;
    sc_out<sc_uint<32> > dq_in;

    sc_uint<16> open_row[16]; bool row_open[16];
    std::map<unsigned long, sc_biguint<512> > mem;

    bool pend_act,pend_wr,pend_rd;
    sc_uint<4> pend_bank,pend_row_low,pend_col_bank;
    bool write_active; unsigned write_beat; unsigned long write_key; sc_biguint<512> write_word;
    bool read_active; unsigned long long edge_count,read_first_edge; sc_biguint<512> read_word;

    unsigned bank_index(sc_uint<2> bg,sc_uint<2> ba);
    unsigned long key_for(sc_uint<16> row,sc_uint<4> bank,sc_uint<6> col);
    void apply_write(unsigned beat,sc_uint<32> dq,sc_uint<4> dm_n);
    void run();
    SC_CTOR(ddr5_x32_memory_model) { SC_THREAD(run); }
};
#endif
