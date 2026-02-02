#pragma once
#include <memory>
#include <vector>

#include "IAnalyzer.h"

template <class FrameT>
class AnalysisPipeline {
public:
  void add(std::unique_ptr<IAnalyzer<FrameT>> a) {
    modules_.push_back(std::move(a));
  }

  void name() const {
    for (const auto& m : modules_) {
      std::cout << m->name() << "\n";
    }
  }

  void begin_run(const RunContext& ctx) {
    for (auto& m : modules_) m->begin_run(ctx);
  }

  void process(FrameT& f, long i, FrameTags& tags) {
    for (auto& m : modules_) m->process(f, i, tags);
  }

  void end_run(const RunContext& ctx) {
    for (auto& m : modules_) m->end_run(ctx);
  }

private:
  std::vector<std::unique_ptr<IAnalyzer<FrameT>>> modules_;
};
