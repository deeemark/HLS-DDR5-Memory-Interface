#include "ddr5_read_phy_neg.h"
void ddr5_read_phy_neg::run() {
    odd1.write(0); odd3.write(0); odd5.write(0); odd7.write(0); odd9.write(0); odd11.write(0); odd13.write(0); odd15.write(0); wait();
    while (1) {
        if (capture_en.read()) {
            switch (odd_sel.read().to_uint()) {
                case 0: odd1.write(dq_in.read()); break;
                case 1: odd3.write(dq_in.read()); break;
                case 2: odd5.write(dq_in.read()); break;
                case 3: odd7.write(dq_in.read()); break;
                case 4: odd9.write(dq_in.read()); break;
                case 5: odd11.write(dq_in.read()); break;
                case 6: odd13.write(dq_in.read()); break;
                default: odd15.write(dq_in.read()); break;
            }
        }
        wait();
    }
}
