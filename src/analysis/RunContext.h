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

  const char *CAL_runname = "run19";   // used for generating calibration factors
  const char *ALIGN_runname = "run17"; // used for align between boards

  // Detector configuration
  int nrBoards = 6;
  int nrChannels = 320;
  int BoardIP[6] = {17, 18, 19, 20, 21, 22};
  const char *BoardName[6] = {"H0", "V0", "H1", "V1", "H2", "V2"};
  int V_boardID[3] = {1, 3, 5}; // measuring vertical profile
  int H_boardID[3] = {0, 2, 4}; // measuring horizontal profile

  double readout_rate = 10000; // in Hz; normal setting is 10 kHz

  // Clustering setting for each board
  // int clustering_threshold[6] = {30, 15, 15, 30, 30, 30}; // up for 6 boards 2024 08 08

  int clustering_threshold[6] = {16, 16, 16, 16, 16, 16}; 
  bool common_mode[6] = {true, false, false, true, true, true};

  int clustering_size[6] = {4, 4, 4, 4, 4, 4}; // up for 6 boards

  // FPGA Settings
  int FPGA_clustering_threshold[6] = {27, 13, 13, 29, 32, 29}; // up for 6 boards
  int FPGA_clustering_size[6] = {4, 4, 4, 4, 4, 4};            // up for 6 boards
  bool FPGA_calibrated = true;                                 // if it is FPGA calibrated

  // Analysis flags // for different analysis types
  bool noise_run = false; // if it is a noise run, noise analyser will analyse all the frames
};
