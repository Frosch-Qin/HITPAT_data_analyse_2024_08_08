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

  // Output ROOT file
  std::string output_file;
  TFile *rootfile = nullptr;
  TFile *SpillIDfile = nullptr;

  // Detector configuration
  int nrBoards = 4; // default is 4
  int nrChannels = 320;
  double readout_rate; // in Hz; normal setting is 10 kHz

  // Clustering setting for each board
  // int clustering_threshold[6] = {30, 15, 15, 30, 30, 30}; // up for 6 boards 2024 08 08
  int clustering_threshold[6] = {30, 30, 30, 15, 30, 30}; // up for 6 boards 2025 05 03

  int clustering_size[6] = {4, 4, 4, 4, 4, 4};            // up for 6 boards

  // FPGA Settings
  int FPGA_clustering_threshold[6] = {15, 15, 15, 15, 15, 15}; // up for 6 boards
  int FPGA_clustering_size[6] = {4, 4, 4, 4, 4, 4};            // up for 6 boards
  bool FPGA_calibrated = false;                                // if it is FPGA calibrated

  // Analysis flags // for different analysis types
  bool noise_run; // if it is a noise run
};
