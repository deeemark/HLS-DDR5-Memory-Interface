#include "ddr5_read_phy_pos.h"
// Logical DDR5-3200 x16 BL16 read PHY. RL=26 CK.
// rd_start is observed one CK after the controller's READ second command cycle;
// waiting RL-1=25 more CK places beat0 capture at READ + 26 CK.
#define R_IDLE 0
#define R_WAIT 1
#define R_CAP2 2
#define R_CAP4 3
#define R_CAP6 4
#define R_CAP8 5
#define R_CAP10 6
#define R_CAP12 7
#define R_CAP14 8
#define R_FINISH 9
#define R_VALID 10

void ddr5_read_phy_pos::run() {
    state=R_IDLE; wait_count=0;
    even0=even2=even4=even6=even8=even10=even12=even14=0;
    neg_capture_en.write(false); odd_sel.write(0); capture_en.write(false); rd_data.write(0); rd_valid.write(false); busy.write(false); wait();
    while (1) {
        rd_valid.write(false);
        if (state==R_IDLE) {
            neg_capture_en.write(false); odd_sel.write(0); capture_en.write(false); busy.write(false);
            if (rd_start.read()) { wait_count=25; busy.write(true); state=R_WAIT; }
        } else if (state==R_WAIT) {
            busy.write(true); neg_capture_en.write(false); capture_en.write(false);
            if (wait_count<=1) { even0=dq_in.read(); odd_sel.write(0); neg_capture_en.write(true); capture_en.write(true); wait_count=0; state=R_CAP2; }
            else wait_count=wait_count-1;
        } else if (state==R_CAP2) {
            even2=dq_in.read(); odd_sel.write(1); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_CAP4;
        } else if (state==R_CAP4) {
            even4=dq_in.read(); odd_sel.write(2); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_CAP6;
        } else if (state==R_CAP6) {
            even6=dq_in.read(); odd_sel.write(3); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_CAP8;
        } else if (state==R_CAP8) {
            even8=dq_in.read(); odd_sel.write(4); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_CAP10;
        } else if (state==R_CAP10) {
            even10=dq_in.read(); odd_sel.write(5); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_CAP12;
        } else if (state==R_CAP12) {
            even12=dq_in.read(); odd_sel.write(6); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_CAP14;
        } else if (state==R_CAP14) {
            even14=dq_in.read(); odd_sel.write(7); neg_capture_en.write(true); capture_en.write(true); busy.write(true); state=R_FINISH;
        } else if (state==R_FINISH) {
            sc_biguint<256> a=0;
            neg_capture_en.write(false); capture_en.write(false); busy.write(true);
            a.range(15,0)=even0; a.range(31,16)=odd1.read(); a.range(47,32)=even2; a.range(63,48)=odd3.read();
            a.range(79,64)=even4; a.range(95,80)=odd5.read(); a.range(111,96)=even6; a.range(127,112)=odd7.read();
            a.range(143,128)=even8; a.range(159,144)=odd9.read(); a.range(175,160)=even10; a.range(191,176)=odd11.read();
            a.range(207,192)=even12; a.range(223,208)=odd13.read(); a.range(239,224)=even14; a.range(255,240)=odd15.read();
            rd_data.write(a); rd_valid.write(true); state=R_VALID;
        } else {
            neg_capture_en.write(false); capture_en.write(false); busy.write(false); state=R_IDLE;
        }
        wait();
    }
}
