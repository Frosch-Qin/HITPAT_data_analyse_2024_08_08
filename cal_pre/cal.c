#include "cal.h"

/*
cal generates board*.txt file for calibration factor for FPGA implementation 
in the board*.txt file 2^13 = 8192 represents 1.0000
*/


int main(int argc, char *argv[])
{
    // auto run_name = argv[1];
    // auto boardID = atoi(argv[2]);
    // double left_nr_sigma = 1.5;  //adjust the left boundary of the plateau region for calibration
    // double right_nr_sigma = 1.5;  //adjust the right boundary of the plateau region for calibration
    // if (argc >= 4)
    // {
    //     left_nr_sigma = atof(argv[3]);
    // }
    // if (argc >= 5)
    // {
    //     right_nr_sigma = atof(argv[4]);
    // }
    // cal_board(run_name, boardID, left_nr_sigma, right_nr_sigma);

   // make_cal_root_file(run_name);

   std::string run_name = "run19";
   double left_sigma[6] = {1.5, 1.5, 1.5, 1.5, 1.5, 1};
   double right_sigma[6] = {1.5, 1.5, 1.5, 1.5, 1, 1.5};
   for (int boardID = 0; boardID < 6; boardID++)
   {
       cal_board(run_name.c_str(), boardID, left_sigma[boardID], right_sigma[boardID]);
   }

//    cal_board("run5", 1, 1.1, 1.1);
//    cal_board("run5", 3, 1.3, 1.6);
//    cal_board("run5", 5, 1.0, 1.35);
//    cal_board("run10", 0, 1.5, 1.5);
//    cal_board("run10", 2, 1.5, 1.5);
//    cal_board("run10", 4, 1.5, 1.5);


    return 0;
}
