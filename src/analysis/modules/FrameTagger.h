// FrameTagger.h
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"
#include "../RunContext.h"

#include <string>

class FrameTagger final : public IAnalyzer<Fullframe> {
public:
  explicit FrameTagger(double threshold = 10.0)
    : threshold_(threshold) {}

  std::string name() const override {
    return "FrameTagger";
  }

  void begin_run(const RunContext& /*ctx*/) override {
    // nothing needed here for simple threshold tagging
  }

  void process(Fullframe& frame,
               long frame_index,
               FrameTags& tags) override
  {
    tags = FrameTags{};          // reset tags
    tags.frame_index = frame_index;


    bool has_signal = false;
    int  above_cnt  = 0;

    // --------------------------------------------

  }

  void end_run(const RunContext& ctx) override {};

private:
  double threshold_;
};
