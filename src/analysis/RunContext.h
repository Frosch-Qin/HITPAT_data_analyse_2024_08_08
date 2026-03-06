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

  const char *CAL_runname = "run3";   // used for generating calibration factors
  const char *ALIGN_runname = "run4"; // used for align between boards

  // Detector configuration
  int nrBoards = 4;
  int nrChannels = 320;
  int BoardIP[4] = {22, 21, 20, 19};
  const char* BoardName[4] = {"V2","H2", "V1", "H1"};
  int V_boardID[2] = {0, 2}; // measuring vertical profile
  int H_boardID[2] = {1, 3}; // measuring horizontal profile

  double readout_rate = 10000; // in Hz; normal setting is 10 kHz

  // Clustering setting for each board
  // int clustering_threshold[6] = {16, 16, 16, 16, 16, 16}; // up for 6 boards 2024 08 08 with common mode subtraction
  // int clustering_threshold[6] = {30, 15, 15, 30, 30, 30}; // up for 6 boards 2024 08 08

  int clustering_threshold[4] = {16, 16, 16, 16};  // up for 6 boards 2025 05 03
  bool common_mode[4] = {true, true, true, false}; // common mode subtraction for 4 boards 2025

  int clustering_size[6] = {4, 4, 4, 4, 4, 4}; // up for 6 boards

  // bool common_mode[6] = {true, false, false, true, true, true}; // common mode subtraction for 4 boards

  // FPGA Settings
  int FPGA_clustering_threshold[6] = {27, 13, 13, 29, 32, 29}; // up for 6 boards
  int FPGA_clustering_size[6] = {4, 4, 4, 4, 4, 4};            // up for 6 boards
  bool FPGA_calibrated = true;                                 // if it is FPGA calibrated

  // Analysis flags // for different analysis types
  bool noise_run = false; // if it is a noise run, noise analyser will analyse all the frames
};
