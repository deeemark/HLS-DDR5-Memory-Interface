#include "ddr5_x32_memory_model.h"
unsigned ddr5_x32_memory_model::bank_index(sc_uint<2> bg,sc_uint<2> ba) { return ((unsigned)bg.to_uint()<<2)|(unsigned)ba.to_uint(); }
unsigned long ddr5_x32_memory_model::key_for(sc_uint<16> row,sc_uint<4> bank,sc_uint<6> col) {
    return ((((unsigned long)row.to_uint())<<10)|(((unsigned long)bank.to_uint())<<6)|((unsigned long)col.to_uint())) & 0x3FFFFFFul;
}
void ddr5_x32_memory_model::apply_write(unsigned beat,sc_uint<32> dq,sc_uint<4> dm_n) {
    unsigned lo=beat*32;
    if (dm_n[0]) write_word.range(lo+7,lo)=dq.range(7,0);
    if (dm_n[1]) write_word.range(lo+15,lo+8)=dq.range(15,8);
    if (dm_n[2]) write_word.range(lo+23,lo+16)=dq.range(23,16);
    if (dm_n[3]) write_word.range(lo+31,lo+24)=dq.range(31,24);
}
void ddr5_x32_memory_model::run() {
    for(int i=0;i<16;++i){open_row[i]=0;row_open[i]=false;}
    pend_act=pend_wr=pend_rd=false; pend_bank=0; pend_row_low=0; pend_col_bank=0;
    write_active=false;write_beat=0;write_key=0;write_word=0;
    read_active=false;edge_count=0;read_first_edge=0;read_word=0;dq_in.write(0);
    while(1){
        wait(clk.posedge_event()|clk.negedge_event()); ++edge_count; bool pos=clk.read();

        // The x32 PHY adds one extra combinational wrapper level around the
        // already-hierarchical x16 PHYs.  Allow all SystemC delta-cycle signal
        // propagation (edge CTHREAD -> x16 output mux -> x32 lane combiner) to
        // settle before the behavioral memory samples DQ/DM/DQ_OE.  This is a
        // simulation-only settling delay and does not change synthesized PHY
        // timing or DDR WL/RL.
        for (int settle=0; settle<5; ++settle) wait(SC_ZERO_TIME);
        if(rst.read()||!ddr_reset_n.read()){
            pend_act=pend_wr=pend_rd=false;write_active=false;read_active=false;dq_in.write(0);
            for(int i=0;i<16;++i)row_open[i]=false;
            continue;
        }
        if(pos){
            sc_uint<14> ca=ddr_ca.read(); bool cs=ddr_cs_n.read();
            if(pend_act && cs){
                sc_uint<16> row=0; row.range(3,0)=pend_row_low; row.range(15,4)=ca.range(11,0);
                open_row[pend_bank.to_uint()]=row; row_open[pend_bank.to_uint()]=true; pend_act=false;
            } else if(pend_wr && cs){
                sc_uint<6> col=ca.range(7,2); unsigned b=pend_col_bank.to_uint();
                if(row_open[b]){
                    write_key=key_for(open_row[b],pend_col_bank,col);
                    std::map<unsigned long,sc_biguint<512> >::iterator it=mem.find(write_key);
                    write_word=(it==mem.end())?sc_biguint<512>(0):it->second; write_active=true; write_beat=0;
                    row_open[b]=false;
                    std::cout<<"["<<name()<<"] X32 MEM WRITE CMD key=0x"<<std::hex<<write_key<<std::dec<<" @ "<<sc_time_stamp()<<std::endl;
                }
                pend_wr=false;
            } else if(pend_rd && cs){
                sc_uint<6> col=ca.range(7,2); unsigned b=pend_col_bank.to_uint();
                if(row_open[b]){
                    unsigned long key=key_for(open_row[b],pend_col_bank,col);
                    std::map<unsigned long,sc_biguint<512> >::iterator it=mem.find(key);
                    read_word=(it==mem.end())?sc_biguint<512>(0):it->second;
                    read_first_edge=edge_count+52; read_active=true; row_open[b]=false;
                    std::cout<<"["<<name()<<"] X32 MEM READ CMD  key=0x"<<std::hex<<key<<std::dec<<" @ "<<sc_time_stamp()<<std::endl;
                }
                pend_rd=false;
            }
            if(!cs){
                if(ca[0]==0 && ca[1]==0){ sc_uint<2> ba=ca.range(7,6), bg=ca.range(9,8); pend_bank=bank_index(bg,ba); pend_row_low=ca.range(5,2); pend_act=true; }
                else if(ca[0]==1&&ca[1]==0&&ca[2]==1&&ca[3]==1&&ca[4]==0){ sc_uint<2> ba=ca.range(7,6), bg=ca.range(9,8); pend_col_bank=bank_index(bg,ba); pend_wr=true; }
                else if(ca[0]==1&&ca[1]==0&&ca[2]==1&&ca[3]==1&&ca[4]==1){ sc_uint<2> ba=ca.range(7,6), bg=ca.range(9,8); pend_col_bank=bank_index(bg,ba); pend_rd=true; }
            }
        }
        if(write_active && dq_oe.read()){
            apply_write(write_beat,dq_out.read(),dm_n_out.read());
            std::cout<<"["<<name()<<"] X32 MEM WRITE beat"<<write_beat<<" DQ="<<dq_out.read().to_string(SC_HEX)<<" DM_n="<<dm_n_out.read().to_uint()<<" @ "<<sc_time_stamp()<<std::endl;
            ++write_beat;
            if(write_beat==16){mem[write_key]=write_word;write_active=false;std::cout<<"["<<name()<<"] X32 MEM WRITE COMMIT data="<<write_word.to_string(SC_HEX)<<" @ "<<sc_time_stamp()<<std::endl;}
        }
        if(read_active){
            if(edge_count>=read_first_edge-1 && edge_count<=read_first_edge+14){
                unsigned beat=(unsigned)(edge_count-(read_first_edge-1));
                dq_in.write((sc_uint<32>)read_word.range(beat*32+31,beat*32));
                std::cout<<"["<<name()<<"] X32 MEM READ drive beat"<<beat<<" DQ="<<read_word.range(beat*32+31,beat*32).to_string(SC_HEX)<<" @ "<<sc_time_stamp()<<std::endl;
            } else if(edge_count>=read_first_edge+15){dq_in.write(0);read_active=false;}
        }
    }
}
