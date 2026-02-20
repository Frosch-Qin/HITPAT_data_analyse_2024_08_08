#include <iostream>
#include <memory>
#include <vector>

#include "src/io/HitStream.h"
#include "src/analysis/RunContext.h"
#include "src/analysis/IAnalyzer.h"
#include "src/analysis/FrameTags.h"
#include "src/analysis/AnalysisPipeline.h"
#include "src/analysis/modules/FrameTagger.h"
#include "src/analysis/modules/NoiseAnalyzer.h"
#include "src/analysis/modules/SpillIDAssigner.h"
#include "src/analysis/modules/UnCalData.h"
#include "src/analysis/modules/Clustering.h"
#include "src/analysis/modules/SpillIDInput.h"
#include "src/analysis/modules/BkgSub.h"
#include "src/analysis/modules/Sum1D.h"
#include "src/analysis/modules/CorrMap.h"
#include "src/analysis/modules/CalData.h"
#include "src/analysis/modules/Algo.h"
#include "src/analysis/modules/CommonModeSub.h"
#include "src/analysis/modules/PosAlign.h"
#include "src/analysis/modules/Pos2DMap.h"
#include "src/analysis/modules/ScanXY.h"


void spillID_analyser(HitStream &stream, RunContext &ctx)
{
  AnalysisPipeline<Fullframe> pipe0;
  pipe0.add(std::make_unique<BkgSub>());
  pipe0.add(std::make_unique<UnCalData>());
  pipe0.add(std::make_unique<Clustering>());
  pipe0.add(std::make_unique<SpillIDAssigner>());
  pipe0.name();
  pipe0.begin_run(ctx);

  Fullframe frame;
  long i = 0;

  // ---- Frame loop ----
  stream.reset();
  while (stream.next(frame))
  {

    // std::cout << "board number in the data" << frame.nrBoards << std::endl;
    FrameTags tags;
    tags.frame_index = i;
    pipe0.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for SpillID " << i << "\n";
    }
    ++i;
    // std::cout<< "frame ID " << i << " processed." <<std::endl;
  }
  pipe0.end_run(ctx);

  // ctx.SpillIDfile->cd();
  // ctx.SpillIDfile->Close();
  stream.close();
  return;
}

// noise

void noise_analyser(HitStream &stream, RunContext &ctx)
{

  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<UnCalData>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<NoiseAnalyzer>());
  pipe.name();
  pipe.begin_run(ctx);

  // ---- Frame loop ----
  Fullframe frame;
  long i = 0;
  stream.reset();
  while (stream.next(frame))
  {
    FrameTags tags;
    tags.frame_index = i;
    pipe.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for Noise " << i << "\n";
    }
    ++i;
  }
  pipe.end_run(ctx);

  stream.close();
  return;
}

void noise_correlation_analyser(HitStream &stream, RunContext &ctx)
{
  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<UnCalData>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<CorrMap>()); // correlation map is very time-consuming; limits the number of frames to be analyzed

  pipe.name();
  pipe.begin_run(ctx);

  // ---- Frame loop ----
  Fullframe frame;
  long i = 0;
  stream.reset();
  while (stream.next(frame))
  {
    FrameTags tags;
    tags.frame_index = i;
    if (i < 10000)
      pipe.process(frame, i, tags);
    else
      break; // only analyze at most 20000 frames for correlation map, to save time and RAM
    if (i % 1000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for Corr Noise " << i << "\n";
    }
    ++i;
  }
  pipe.end_run(ctx);

  stream.close();
  return;
}

void sum1D_analyser(HitStream &stream, RunContext &ctx)
{
  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<UnCalData>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<Sum1D>());

  pipe.name();
  pipe.begin_run(ctx);

  // ---- Frame loop ----
  Fullframe frame;
  long i = 0;
  stream.reset();
  while (stream.next(frame))
  {
    FrameTags tags;
    tags.frame_index = i;
    pipe.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for Sum1D " << i << "\n";
    }
    ++i;
  }
  pipe.end_run(ctx);

  stream.close();
  return;
}

// compare FPGA with CPU

void compare_fpga_cpu()
{
}

// resolution analysis

void resolution_2DMap0(HitStream &stream, RunContext &ctx)
{

  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<UnCalData>());
  pipe.add(std::make_unique<CommonModeSub>());
  pipe.add(std::make_unique<Clustering>());
  pipe.add(std::make_unique<CalData>());
  pipe.add(std::make_unique<Algo>("grarms"));
  pipe.add(std::make_unique<PosAlign>());
  pipe.add(std::make_unique<ScanXY>());

  
  // pipe.add(std::make_unique<Pos2DMap>());

  pipe.name();
  pipe.begin_run(ctx);

  // ---- Frame loop ----
  Fullframe frame;
  long i = 0;
  stream.reset();
  while (stream.next(frame))
  {
    FrameTags tags;
    tags.frame_index = i;
    pipe.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for 2DMap " << i << "\n";
    }
    ++i;
  }
  pipe.end_run(ctx);

  stream.close();
  return;
}

void resolution_2DMap1(HitStream &stream, RunContext &ctx)
{

  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<UnCalData>());
  pipe.add(std::make_unique<CommonModeSub>());
  pipe.add(std::make_unique<Clustering>());
  pipe.add(std::make_unique<CalData>());
  pipe.add(std::make_unique<Algo>("grarms"));
  pipe.add(std::make_unique<PosAlign>());
  // pipe.add(std::make_unique<ScanXY>());

  pipe.add(std::make_unique<Pos2DMap>());

  pipe.name();
  pipe.begin_run(ctx);

  // ---- Frame loop ----
  Fullframe frame;
  long i = 0;
  stream.reset();
  while (stream.next(frame))
  {
    FrameTags tags;
    tags.frame_index = i;
    pipe.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for 2DMap " << i << "\n";
    }
    ++i;
  }
  pipe.end_run(ctx);

  stream.close();
  return;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file.da2> [int run_number] [max_frames]\n";
    return 2;
  }

  const std::string filename = argv[1];
  const long max_frames = (argc >= 4) ? std::stol(argv[3]) : -1;

  HitStream stream(HitStream::Config{
      .filename = filename,
      .first_frame = 0,
      .max_frames = max_frames,
      .increment = 1});

  if (!stream.open())
  {
    return 1;
  }

  std::cout << "File frames total: " << stream.total_frames_in_file() << "\n";
  std::cout << "Frames to read:    " << stream.frames_to_read() << "\n";

  // ---- RunContext ----
  RunContext ctx;
  ctx.run_number = std::stoi(argv[2]);
  ctx.input_file = filename;
  ctx.nrBoards = 6;
  ctx.readout_rate = 10000; // 10 kHz
  ctx.max_frames = (max_frames == -1) ? stream.total_frames_in_file() : max_frames;
  ctx.noise_run = false; // for noise run, noise analyser will analyse all the frames
  ctx.FPGA_calibrated = true;

  spillID_analyser(stream, ctx);
  // noise_analyser(stream, ctx);
  // noise_correlation_analyser(stream, ctx);
  // sum1D_analyser(stream, ctx);

  resolution_2DMap0(stream, ctx);
  resolution_2DMap1(stream, ctx);
  stream.close();
  return 0;
}
