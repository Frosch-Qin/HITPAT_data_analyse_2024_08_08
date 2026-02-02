#pragma once

#include <bitset>
#include <iomanip>
#include <map>
#include <functional>
#include <string>
#include <stdio.h>
#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm> // std::count
#include <utility>
#include <TLine.h>
#include <TH2.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystemDirectory.h>
#include <gsl/gsl_statistics.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <gsl/gsl_sort.h>
#include <TVector.h>
#include "TStopwatch.h"
#include "Math/MinimizerOptions.h"
#include "TVirtualFitter.h"
#include "TMatrixD.h"
#include "TApplication.h"
#include "TRint.h"
#include "TSystem.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include <TLine.h>
#include <type_traits>
#include <functional>
#include <TGraph2D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <queue>
#include "hitreader.h" // provides Fullframe

class HitStream {
public:
  struct Config {
    std::string filename;
    std::int64_t first_frame = 0; // starting frame index in file
    std::int64_t max_frames  = -1; // -1 = read to end
    std::int64_t increment   = 1;  // read every Nth frame
  };

  explicit HitStream(Config cfg)
    : cfg_(std::move(cfg))
  {
    if (cfg_.increment <= 0) throw std::invalid_argument("HitStream: increment must be > 0");
    if (cfg_.first_frame < 0) throw std::invalid_argument("HitStream: first_frame must be >= 0");
  }

  bool open() {
    close();

    file_.open(cfg_.filename, std::ios::binary);
    if (!file_.is_open()) {
      std::cerr << "HitStream: cannot open file: " << cfg_.filename << "\n";
      return false;
    }

    // Read a sample frame to determine frame byte size
    {
      Fullframe sample;
      file_.seekg(0, std::ios::beg);
      if (sample.read(&file_) == 1) {
        std::cerr << "HitStream: failed to read first frame (file format?)\n";
        close();
        return false;
      }
      frame_size_bytes_ = sample.sizeInFile();
    }

    // Determine file size
    file_.clear();
    file_.seekg(0, std::ios::end);
    const std::streamoff end_pos = file_.tellg();
    if (end_pos <= 0 || frame_size_bytes_ <= 0) {
      std::cerr << "HitStream: invalid file size or frame size\n";
      close();
      return false;
    }
    file_size_bytes_ = static_cast<std::int64_t>(end_pos);

    // total frames in file (floor)
    total_frames_in_file_ = file_size_bytes_ / frame_size_bytes_;

    // compute how many frames we will output
    const std::int64_t available_from_first =
      (cfg_.first_frame < total_frames_in_file_) ? (total_frames_in_file_ - cfg_.first_frame) : 0;

    const std::int64_t possible_outputs =
      (available_from_first <= 0) ? 0 : ((available_from_first + cfg_.increment - 1) / cfg_.increment);

    if (cfg_.max_frames < 0) {
      frames_to_read_ = possible_outputs;
    } else {
      frames_to_read_ = std::min<std::int64_t>(possible_outputs, cfg_.max_frames);
    }

    // initialize iteration state
    produced_ = 0;
    current_file_frame_index_ = cfg_.first_frame;

    // position stream ready for first read
    file_.clear();
    return true;
  }

  void reset() {
    close();
    open();
  }

  void close() {
    if (file_.is_open()) file_.close();
    frame_size_bytes_ = 0;
    file_size_bytes_ = 0;
    total_frames_in_file_ = 0;
    frames_to_read_ = 0;
    produced_ = 0;
    current_file_frame_index_ = 0;
  }

  bool is_open() const { return file_.is_open(); }

  // how many frames exist physically in the file
  std::int64_t total_frames_in_file() const { return total_frames_in_file_; }

  // how many frames this stream will yield given first_frame/increment/max_frames
  std::int64_t frames_to_read() const { return frames_to_read_; }

  std::int64_t produced() const { return produced_; }

  // the underlying "real" frame index in the file for the next read
  std::int64_t next_file_frame_index() const { return current_file_frame_index_; }

  // Read next frame into 'out'. Returns false at end (or if not open).
  bool next(Fullframe& out) {
    if (!file_.is_open()) return false;
    if (produced_ >= frames_to_read_) return false;
    if (current_file_frame_index_ < 0 || current_file_frame_index_ >= total_frames_in_file_) return false;

    const std::int64_t offset = current_file_frame_index_ * frame_size_bytes_;
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!file_) return false;

    if (out.read(&file_) == 1) {
      std::cerr << "HitStream: read error at file frame " << current_file_frame_index_ << "\n";
      return false;
    }

    // advance state
    produced_ += 1;
    current_file_frame_index_ += cfg_.increment;
    return true;
  }

private:
  Config cfg_;
  std::ifstream file_;

  std::int64_t frame_size_bytes_ = 0;
  std::int64_t file_size_bytes_ = 0;
  std::int64_t total_frames_in_file_ = 0;

  std::int64_t frames_to_read_ = 0;
  std::int64_t produced_ = 0;
  std::int64_t current_file_frame_index_ = 0;
};
