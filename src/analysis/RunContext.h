// RunContext.h
// define the common feature for all the frames in one run

#pragma once

#include <string>
#include <cstddef>

struct RunContext
{
  int run_number;
  std::string input_file;
  long max_frames;

  // Detector configuration
  int nrBoards = 6; 
  int nrChannels = 320;
  int V_boardID[3] = {1,3,5}; //measuring vertical profile
  int H_boardID[3] = {0,2,4}; //measuring horizontal profile
  double readout_rate; // in Hz; normal setting is 10 kHz

  // Clustering setting for each board
  int clustering_threshold[6] = {16, 16, 16, 16, 16, 16}; // up for 6 boards 2024 08 08 with common mode subtraction
  // int clustering_threshold[6] = {30, 15, 15, 30, 30, 30}; // up for 6 boards 2024 08 08

  // int clustering_threshold[6] = {30, 30, 30, 15, 30, 30}; // up for 6 boards 2025 05 03

  int clustering_size[6] = {4, 4, 4, 4, 4, 4}; // up for 6 boards

  bool common_mode[6] = {true, false, false, true, true, true}; // common mode subtraction for 4 boards

  // FPGA Settings
  int FPGA_clustering_threshold[6] = {27, 13, 13, 29, 32, 29}; // up for 6 boards
  int FPGA_clustering_size[6] = {4, 4, 4, 4, 4, 4};            // up for 6 boards
  bool FPGA_calibrated = false;                                // if it is FPGA calibrated

  // Analysis flags // for different analysis types
  bool noise_run; // if it is a noise run
};
