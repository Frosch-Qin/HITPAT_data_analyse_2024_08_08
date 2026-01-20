#include <iostream>

int main(){
    ofstream out("calibration_factor/cal_one.txt");
    for (int i = 0; i < 320; i++){
        out << 8192 << endl;
    }
    out.close();

    ofstream out2("calibration_factor/cal_zero.txt");
    for (int i = 0; i < 320; i++){
        out2 << 0 << endl;
    }
    out2.close();
}