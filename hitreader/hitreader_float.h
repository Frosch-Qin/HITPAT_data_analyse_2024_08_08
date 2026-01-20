// This is an object interface for reading and writing the background signal and
// calibration signal for HITDATA
// The difference between Fullframe and Fullframe_float is that the datatype in Fullframe_float
// is float

/*
  .L fullframe_float.h
  Fullframe sampleframe;
  sampleframe.read(file);
  Fullframe_float bkg;
  bkg=sampleframe;
 */

//*********************** Helper *************************

// #define debug(str)	std::cout << "HIT DEBUG: " << str << endl;
#define debug(str)

#include "hitreader.h"

//*********************** Sensorframe *************************
class Boardframe_float
{
public:
  Boardframe_float(int nr_channels = 0)
  {
    debug("Boardframe_float()");

    data = NULL;
    resize(nr_channels);
  };

  Boardframe_float(const Boardframe_float &in)
  {
    debug("Boardframe_float(Boardframe_float&)");

    data = NULL;
    resize(in.nrChannels);
    for (int i = 0; i < nrChannels; i++)
      data[i] = in.data[i];
    syncframe = in.syncframe;
  };

  Boardframe_float &operator=(const Boardframe_float &in)
  {
    debug("Boardframe_float::operator==");

    resize(in.nrChannels); // creates an array called data of length nrChannels
    for (int i = 0; i < nrChannels; i++)
      data[i] = in.data[i];
    syncframe = in.syncframe;
    return *this;
  };

  ~Boardframe_float()
  {
    debug("~Boardframe_float()");

    if (data)
      delete[] data;
  };

  void resize(int nr_channels)
  {
    if (data)
      delete[] data;
    nrChannels = nr_channels;
    if (nrChannels)
      data = new double[nrChannels];
    else
      data = NULL;
  };

  int sizeInFile()
  {
    return syncframe.sizeInFile() + nrChannels * 8;
  };

  int read(std::ifstream *file)
  {
    if (syncframe.read(file) == 0) // get the syncframe before the board data
      return 0;
    // I must be already resized at this point!
    file->read((char *)data, 8 * nrChannels);
    if (file->fail())
      return 0;
    /*   std::cout<< "data[" << nrChannels << "]: ";
    for (int i = 0;i<nrChannels;i++) std::cout << data[i] << " ";
    std::cout << std::endl;*/

    return 1;
  };

  int write(std::ofstream *file)
  {
    if (!syncframe.write(file))
      return 0;

    file->write(reinterpret_cast<char *>(data), 8 * nrChannels);

    if (file->fail())
      return 0;

    return 1;
  }

  double &operator[](int index)
  {
    return data[index];
  };

  Syncframe syncframe;
  int nrChannels;
  double *data;
};

//*********************** Fullframe_float *************************
class Fullframe_float
{
public:
  Fullframe_float(int nr_boards = 0)
  {
    debug("Fullframe_float()");
    boards = NULL;
    resize(nr_boards);
  };

  Fullframe_float(const Fullframe_float &in)
  {
    debug("Fullframe_float(Fullframe_float&)");
    boards = NULL;
    resize(in.nrBoards);
    for (int i = 0; i < nrBoards; i++)
      boards[i] = in.boards[i];
  };

  // define Fullframe_foat = Fullframe
  Fullframe_float &operator=(const Fullframe &in)
  {
    nrBoards = in.nrBoards;
    resize(in.nrBoards);
    for (int i = 0; i < nrBoards; i++)
    {
      boards[i].syncframe = in.boards[i].syncframe;
      boards[i].nrChannels = in.boards[i].nrChannels;
      boards[i].resize(in.boards[i].nrChannels);
      for (int j = 0; j < in.boards[i].nrChannels; j++)
      {
        boards[i].data[j] = static_cast<float>(in.boards[i].data[j]);
      }
    }
    return *this;
  };
  Fullframe_float &operator=(const Fullframe_float &in)
  {
    debug("Fullframe_float::operator==");
    resize(in.nrBoards);
    for (int i = 0; i < nrBoards; i++)
      boards[i] = in.boards[i];

    return *this;
  };

  ~Fullframe_float()
  {
    debug("~Fullframe_float()");
    if (boards)
      delete[] boards;
  };

  void resize(int nr_boards)
  {
    if (boards)
      delete[] boards;
    nrBoards = nr_boards;
    if (nrBoards)
      boards = new Boardframe_float[nrBoards];
    else
      boards = NULL;
  }

  int sizeInFile()
  {
    int sum_size = 0;
    if (boards)
    {
      for (int i = 0; i < nrBoards; i++)
        sum_size += boards[i].sizeInFile();
      return 2 + nrBoards * 2 + sum_size;
    }
    // return 2 + 4*2 + (16 +320*2)+(16+128*2)*3;
    // return 2 + nrBoards*2 + nrBoards * boards[0].sizeInFile();
    else
      return 0; // no boards, makes no sense...
  };

  int read(std::ifstream *file)
  {
    // Read number of boards
    unsigned short nr_boards;
    file->read((char *)&nr_boards, 2);
    if (file->fail() || nr_boards > 6)
    {
      std::cerr << "Unrealistic number of board to be read:" << nr_boards << std::endl;
      return 0;
    }
    //  std::cout << " nr_boards: " << nr_boards << std::endl;
    // Read channel counts
    unsigned short *channel_counts = new unsigned short[nr_boards];
    file->read((char *)channel_counts, nr_boards * 2);
    if (file->fail())
    {
      delete[] channel_counts;
      return 0;
    }

    // Read board frames
    resize(nr_boards);
    for (int board_nr = 0; board_nr < nr_boards; board_nr++)
    {
      //	std::cout << " channel_counts[" << board_nr << "]: "<< channel_counts[board_nr] << std::endl;

      boards[board_nr].resize(channel_counts[board_nr]);
      if (boards[board_nr].read(file) == 0) // read the board
      {
        delete[] channel_counts;
        return 0;
      }
    }

    delete[] channel_counts;
    return 1;
  };

  int write(std::ofstream *file)
  {
    // write nrboards
    file->write((char *)&nrBoards, 2);
    unsigned short *channel_counts = new unsigned short[nrBoards];
    for (int board_nr = 0; board_nr < nrBoards; board_nr++)
    {
      channel_counts[board_nr] = boards[board_nr].nrChannels;
    }
    file->write((char *)channel_counts, nrBoards * 2);
    for (int board_nr = 0; board_nr < nrBoards; board_nr++)
    {
      if (boards[board_nr].write(file) == 0) // write the board
      {
        delete[] channel_counts;
        return 0;
      }
    }
    delete[] channel_counts;
    return 1;
  };

  TGraph *plot(int board_nr)
  {

    auto g = new TGraph();
    for (int i = 0; i < boards[board_nr].nrChannels; ++i)
    {
      g->SetPoint(i, i, boards[board_nr].data[i]);
    }
    g->GetXaxis()->SetTitle("Channel");
    g->GetYaxis()->SetTitle("Counts");
    g->GetXaxis()->SetRangeUser(0, boards[board_nr].nrChannels);
    g->SetMarkerStyle(20);
    g->SetMarkerSize(0.5);
    g->Draw("APL");

    return g;
  }

  int nrChannels()
  {
    int result = 0;
    for (int board_nr = 0; board_nr < nrBoards; board_nr++)
      result += boards[board_nr].nrChannels;
    return result;
  };

  double &operator[](int index)
  {
    for (int board_nr = 0; board_nr < nrBoards; board_nr++)
    {
      if (index >= boards[board_nr].nrChannels)
        index -= boards[board_nr].nrChannels;
      else
        return boards[board_nr][index];
    }

    std::cerr << " ### Fullframe_float::operator[]: index out of range!" << std::endl;
    //   return (*NULL); //this will cause crash (intended).
    return boards[nrBoards][index];
  };

  int nrBoards;
  Boardframe_float *boards;
};
