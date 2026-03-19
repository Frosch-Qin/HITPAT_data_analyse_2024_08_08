#include <iostream>
#include <memory>
#include <vector>

#include "src/io/HitStream.h"

#include "src/algo/HITNamespace.h"
#include "src/algo/beamrecon.h"
#include "src/algo/cluster_find.c"
#include "src/algo/sum_peak.h"
#include "src/algo/gaussian_fit.h"
#include "src/algo/fas.h"
#include "src/algo/fas_cluster.h"
#include "src/algo/fas_3sigma.h"
#include "src/algo/recon_grav_rms.c"
#include "src/algo/recon_grav_rms_hd.c"
#include "src/algo/simple_2by2.h"
#include "src/algo/caruana.h"
#include "src/algo/guo.h"
#include "src/algo/rms_fitting.h"
#include "src/algo/caruana_fitting.h"
// #include "src/algo/for_register/recon_grav_rms_hd_reg.c"

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
#include "src/analysis/modules/Pos2DHH.h"
#include "src/analysis/modules/Pos1D.h"
#include "src/analysis/modules/Sum1DPos.h"




void alignment_analyser(HitStream &stream, RunContext &ctx)
{
  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<CommonModeSub>());
  pipe.add(std::make_unique<Clustering>());
  pipe.add(std::make_unique<CalData>());
  pipe.add(std::make_unique<Algo>("grarms"));
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<Pos1D>());

  pipe.name();
  pipe.begin_run(ctx);

  Fullframe frame;
  long i = 0;

  // ---- Frame loop ----
  stream.reset();
  while (stream.next(frame))
  {

    // std::cout << "board number in the data" << frame.nrBoards << std::endl;
    FrameTags tags;
    tags.frame_index = i;
    pipe.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for alignment " << i << "\n";
    }
    ++i;
    // std::cout<< "frame ID " << i << " processed." <<std::endl;
  }
  pipe.end_run(ctx);

  // ctx.SpillIDfile->cd();
  // ctx.SpillIDfile->Close();
  stream.close();
  return;
}

void spillID_analyser(HitStream &stream, RunContext &ctx)
{
  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<CommonModeSub>());
  pipe.add(std::make_unique<Clustering>());
  pipe.add(std::make_unique<SpillIDAssigner>());
  pipe.name();
  pipe.begin_run(ctx);

  Fullframe frame;
  long i = 0;

  // ---- Frame loop ----
  stream.reset();
  while (stream.next(frame))
  {

    // std::cout << "board number in the data" << frame.nrBoards << std::endl;
    FrameTags tags;
    tags.frame_index = i;
    pipe.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame for SpillID " << i << "\n";
    }
    ++i;
    // std::cout<< "frame ID " << i << " processed." <<std::endl;
  }
  pipe.end_run(ctx);

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



void resolution_2DMap(HitStream &stream, RunContext &ctx)
{

  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<CommonModeSub>());
  pipe.add(std::make_unique<Clustering>());
  pipe.add(std::make_unique<CalData>());
  pipe.add(std::make_unique<Algo>("grarms"));
  pipe.add(std::make_unique<PosAlign>());
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


void ScanXY_analyser(HitStream &stream, RunContext &ctx)
{

  AnalysisPipeline<Fullframe> pipe;
  pipe.add(std::make_unique<BkgSub>());
  pipe.add(std::make_unique<SpillIDInput>());
  pipe.add(std::make_unique<CommonModeSub>());
  pipe.add(std::make_unique<Clustering>());
  pipe.add(std::make_unique<CalData>());
  pipe.add(std::make_unique<Algo>("grarms"));
  pipe.add(std::make_unique<ScanXY>());

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
      std::cout << "Processing frame for ScanXY " << i << "\n";
    }
    ++i;
  }
  pipe.end_run(ctx);

  stream.close();
  return;
}


void convert_Sum1DPos(HitStream &stream, RunContext &ctx)
{

  (void) stream; 
  AnalysisPipeline<Fullframe> pipe;
  
  pipe.add(std::make_unique<Sum1DPos>());

  pipe.name();
  pipe.begin_run(ctx);
  pipe.end_run(ctx);

  stream.close();
  return;
}