// IAnalyzer.h
#pragma once

#include <string>

// Forward declarations (you can replace these with #include "RunContext.h" etc. if you have them)
#include "RunContext.h"
#include "FrameTags.h"
#include "../io/hitreader.h"

// Interface for “analyzer modules” that process frames of type FrameT
template <class FrameT>
class IAnalyzer
{
public:
  virtual ~IAnalyzer() = default;

  // A short identifier for logging / output folders / plots, etc.
  virtual std::string name() const = 0;

  // Called once at the start of a run (optional to override)
  virtual void begin_run(const RunContext &ctx) final
  {
    init_common(ctx);
    on_begin_run(ctx);
  }
  // Called for every frame (must override)
  virtual void process(FrameT &frame, long frame_index, FrameTags &tags) = 0;

  // Called once at the end of a run (optional to override)
  virtual void end_run(const RunContext &ctx) {};

protected:
  static constexpr int kMaxBoards = 6;
  int nrBoards = kMaxBoards;
  int V_boardID[3];
  int H_boardID[3];

  // Derived classes override THESE instead of begin_run/end_run
  virtual void on_begin_run(const RunContext & /*ctx*/) {}
  virtual void on_end_run(const RunContext & /*ctx*/) {}

private:
  void init_common(const RunContext &ctx)
  {
    nrBoards = std::min(ctx.nrBoards, kMaxBoards);

    if (ctx.nrBoards > kMaxBoards)
    {
      std::cerr << "Warning: nrBoards in RunContext is greater than "
                << kMaxBoards << ", limiting to " << kMaxBoards << ".\n";
    }

    for (int i = 0; i < nrBoards / 2; ++i)
    {
      H_boardID[i] = ctx.H_boardID[i];
      V_boardID[i] = ctx.V_boardID[i];
    }
  }
};
