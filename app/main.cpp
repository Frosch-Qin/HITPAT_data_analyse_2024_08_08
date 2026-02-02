#include <iostream>
#include <memory>
#include <vector>

#include "../src/io/HitStream.h"
#include "../src/analysis/RunContext.h"
#include "../src/analysis/IAnalyzer.h"
#include "../src/analysis/FrameTags.h"
#include "../src/analysis/AnalysisPipeline.h"
#include "../src/analysis/modules/FrameTagger.h"
#include "../src/analysis/modules/NoiseAnalyzer.h"
#include "../src/analysis/modules/SpillIDAssigner.h"
#include "../src/analysis/modules/UnCalData.h"
#include "../src/analysis/modules/Clustering.h"
#include "../src/analysis/modules/SpillIDInput.h"
#include "../src/analysis/modules/BkgSub.h"
#include "../src/analysis/modules/Sum1D.h"

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file.da2> [max_frames]\n";
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
  ctx.max_frames = stream.total_frames_in_file();
  ctx.noise_run = false; // for noise run
  ctx.FPGA_calibrated = false;

  Fullframe frame;
  long i = 0;

  //************************************************************//
  //************************************************************//
  //**********************  Analyzers 0  ***********************//
  //************************************************************//
  ctx.SpillIDfile = new TFile(Form("../output/SpillID/run%d_SpillID.root", ctx.run_number), "RECREATE");

  AnalysisPipeline<Fullframe> pipe0;
  pipe0.add(std::make_unique<BkgSub>()); 
  pipe0.add(std::make_unique<UnCalData>());
  pipe0.add(std::make_unique<Clustering>());
  pipe0.add(std::make_unique<SpillIDAssigner>());
  pipe0.name();
  pipe0.begin_run(ctx);

  // ---- Frame loop ----


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

  ctx.SpillIDfile->cd();
  ctx.SpillIDfile->Close();

  //************************************************************//
  //************************************************************//
  //**********************  Analyzers 1  ***********************//
  //************************************************************//

  ctx.rootfile = new TFile(Form("../output/run%d_output.root", ctx.run_number), "RECREATE");
  ctx.rootfile->cd();
  AnalysisPipeline<Fullframe> pipe1;
  pipe1.add(std::make_unique<BkgSub>());
  pipe1.add(std::make_unique<UnCalData>());
  pipe1.add(std::make_unique<Clustering>());
  pipe1.add(std::make_unique<SpillIDInput>());
  // pipe1.add(std::make_unique<FrameTagger>(10.0));
  pipe1.add(std::make_unique<Sum1D>());
  pipe1.add(std::make_unique<NoiseAnalyzer>());

  pipe1.name();
  pipe1.begin_run(ctx);

  // ---- Frame loop ----

  i = 0;
  stream.reset();
  while (stream.next(frame))
  {

    // std::cout << "board number in the data" << frame.nrBoards << std::endl;
    FrameTags tags;
    tags.frame_index = i;

    pipe1.process(frame, i, tags);

    if (i % 100000 == 0)
    {
      // print tag for each frame
      std::cout << tags.to_string() << "\n";
      std::cout << "Processing frame..." << i << "\n";
    }
    ++i;
    // std::cout<< "frame ID " << i << " processed." <<std::endl;
  }
  pipe1.end_run(ctx);


  ctx.rootfile->cd();
  ctx.rootfile->Close();

  stream.close();
  return 0;
}
