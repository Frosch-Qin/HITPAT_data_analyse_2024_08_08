#include "main.h"
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
  //check RunContext.h to specify the run character
  RunContext ctx;
  ctx.run_number = std::stoi(argv[2]);
  ctx.input_file = filename;
  ctx.max_frames = (max_frames == -1) ? stream.total_frames_in_file() : max_frames;

  // spillID_analyser(stream, ctx);
  // noise_analyser(stream, ctx);
  // noise_correlation_analyser(stream, ctx);
  sum1D_analyser(stream, ctx);

  // alignment_analyser(stream, ctx);
  resolution_2DMap(stream, ctx);


  stream.close();
  return 0;
}
