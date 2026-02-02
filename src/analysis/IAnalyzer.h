// IAnalyzer.h
#pragma once

#include <string>

// Forward declarations (you can replace these with #include "RunContext.h" etc. if you have them)
#include "RunContext.h"
#include "FrameTags.h"
#include "../io/hitreader.h"


// Interface for “analyzer modules” that process frames of type FrameT
template <class FrameT>
class IAnalyzer {
public:
  virtual ~IAnalyzer() = default;

  // A short identifier for logging / output folders / plots, etc.
  virtual std::string name() const = 0;

  // Called once at the start of a run (optional to override)
  virtual void begin_run(const RunContext& ctx) {};
  // Called for every frame (must override)
  virtual void process(FrameT& frame, long frame_index, FrameTags& tags) = 0;

  // Called once at the end of a run (optional to override)
  virtual void end_run(const RunContext& ctx) {};
};
