// FrameTagger.h
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"
#include "../RunContext.h"

#include <string>

class FrameTagger final : public IAnalyzer<Fullframe>
{
public:
  explicit FrameTagger(bool Raw_data_reserved)
    : Raw_data_reserved(Raw_data_reserved) {}

  std::string name() const override
  {
    return "FrameTagger";
  }

  void on_begin_run(RunContext &ctx) override
  {
    readout_rate = ctx.readout_rate;
  }

  void process(Fullframe &frame,
               long frame_index,
               FrameTags &tags) override
  {
    // tags = FrameTags{}; // reset tags
    tags.frame_index = frame_index;
    tags.time = frame_index / readout_rate;
    tags.Has_signal = false;

    if (Raw_data_reserved) //if 1, reserve raw data in tags.raw_frame
      tags.raw_frame = frame;

    // --------------------------------------------
    // calculate SNR
    // for (int i = 0; i < nrBoards; i++)
    // {
    //   if (tags.boardTags[i].Cluster_num > 0)
    //   {
    //     tags.boardTags[i].Snr = tags.boardTags[i].Peak / uncommon_noise[i];
    //     std::cout << tags.boardTags[i].Snr << " ";
    //   }
    // }
  }

  void end_run(const RunContext &ctx) override {};

private:
  double readout_rate;
  bool Raw_data_reserved;

};
