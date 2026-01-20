/******************************************************************************
* Copyright                                                                   *
* Scintillating Fibre Beam Profile Monitor Software by Michal Dziewiecki,     *
* Blake Leverington and Liqing Qin is licensed under CC BY 4.0                *
* https://creativecommons.org/licenses/by/4.0/                                *
* funded by the Deutsche Forschungsgemeinschaft                               *
* (DFG, German Research Foundation) Projektnummer 419255448                   *
* Project Leader: B.Leverington                                               *
*******************************************************************************
* Create Date - 2019                                                          *
* Author: M.Dziewiecki                                                        *
* Module - hitreader.h                                                        *
* Edited by L.Qin on 01. Feb. 2024                                            *
******************************************************************************/

// This is an object interface for reading HIT data files for testbeam 2024Feb/Aug with the new FPGA firmware.
// See HIT documentation for details and examples.
/*
  .L hitreader.c

  Hitdata data;
  data.readFile(“my_file.da2”);                                            //to load whole file at once – forget it! See below.
  data.readFile(“my_file.da2”,1000,100,10)                    //to read 100 frames starting from frame 1000 and incrementing by 10 (i.e. frame 1000, 1010, 1020, ... 1990 will be read)
  //Reading 10 000 frames is reasonable. Reading 100 000 frames made my VM beg for memory.

  data.nrFrames                                                                  //to see how many frames you have
  data.frames[0].nrBoards                                              //to see how many boards you had in the system
  data.frames[0].boards[0].nrChannels                    //to see how many channels you have in board 0
  data.frames[10].boards[0].data[100]                      //get signal value for frame 10, board 0, channel 100
  data.frames[10].boards[0].syncframe.local_ctr //get the local synchro counter for frame 10, board 0
  //same for .global_ctr, .sma_state, .dummy, .device_nr, .data_ok

 */

//*********************** Helper *************************

#include <fstream>
#include <iostream>
#define DATASOURCE string("2024_08_08")


using namespace std;

// #define debug(str)	std::cout << "HIT DEBUG: " << str << endl;
#define debug(str)

//*********************** Fpgasframe *************************
	/* reg results has 32*4 bits
	WORDS 163 [31:16]MeanXleftshift [15:0] Sigma0;
			164 [31:16]MaxY; [15:0] STATUS;
			165 for debug;
			166 for debut;
      
      results[1][0] <= bkg_sub_on;
			results[1][1] <= has_cluster;
			results[1][2] <= no_cluster;
			results[2][31:16] <= {7'b0,Windowleft};
			results[2][15:0] <= {7'b0,Windowright};
		for status register:
		STATUS_BKG_SUB_ON  0x0001
		STATUS_HAS_CLUSTER 0x0002
		STATUS_NO_CLUSTER  0x0004	
		*/
		
class Fpgasframe
{
public:
  Fpgasframe()
  {
    debug("Fpgasframe()");

    Position = Sigma = Peak = Status = 0;
    Dummy0 = Dummy1 = 0;
    BKG_SUB_ON = Status & 0x0001;
    HAS_CLUSTER = Status & 0x0002;
    NO_CLUSTER = Status & 0x0004;
    Windowleft = (Dummy0 & 0xFFFF0000)>>16;
    Windowright = (Dummy0 & 0x0000FFFF);
  };

  ~Fpgasframe()
  {
    debug("~Fpgasframe()");
  };

  int sizeInFile()
  {
    return 16;
  };

  int read(std::ifstream *file)
  {
    char buffer[16];
    file->read(buffer, 16);
    if (file->fail())
      return 1;
    Position = *(unsigned short *)(buffer + 0)*0.05;
    Sigma = *(unsigned short *)(buffer + 2)*0.05;
    Peak = *(unsigned short *)(buffer + 4);
    Status = *(unsigned short *)(buffer + 6);
    Dummy0 = *(int *)(buffer + 8);
    Dummy1 = *(int *)(buffer + 12);
    BKG_SUB_ON = (Status & 0x0001)>>0;
    HAS_CLUSTER = (Status & 0x0002)>>1;
    NO_CLUSTER = (Status & 0x0004)>>2;
    Windowright = (Dummy0 & 0xFFFF0000)>>16;
    Windowleft = (Dummy0 & 0x0000FFFF);
    //std::cout << "Fpgasframe:" << Position << " " << Sigma << " " << Peak << " " << Status << " " << Dummy0 << " " << Dummy1 << std::endl;
    return 0;
  };

  int write(std::ofstream *file)
  {
    char buffer[16];
    *(unsigned short *)(buffer + 0) = Position/0.05;
    *(unsigned short *)(buffer + 2) = Sigma/0.05;
    *(unsigned short *)(buffer + 4) = Peak;
    *(unsigned short *)(buffer + 6) = Status;
    *(int *)(buffer + 8) = Dummy0;
    *(int *)(buffer + 12) = Dummy1;

    file->write(buffer, 16);

    if (file->fail())
      return 1;

    return 0;
  }

  double Position;
  double Sigma;
  unsigned short Peak;
  unsigned short Status;
  unsigned short Windowleft;
  unsigned short Windowright;
  unsigned short BKG_SUB_ON;
  unsigned short HAS_CLUSTER;
  unsigned short NO_CLUSTER;
  int Dummy0;
  int Dummy1;
};

//*********************** Syncframe *************************
class Syncframe
{
public:
  Syncframe()
  {
    debug("Syncframe()");

    local_ctr = global_ctr = 0;
    sma_state = dummy = 0;
    device_nr = -1;
    data_ok = 0;
  };

  ~Syncframe()
  {
    debug("~Syncframe()");
  };

  int sizeInFile()
  {
    return 16;
  };

  int read(std::ifstream *file)
  {
    char buffer[16];
    file->read(buffer, 16);
    if (file->fail())
      return 1;
    local_ctr = *(unsigned short *)(buffer + 0);
    global_ctr = *(unsigned short *)(buffer + 2);
    sma_state = *(unsigned short *)(buffer + 4);
    dummy = *(unsigned short *)(buffer + 6);
    device_nr = *(int *)(buffer + 8);
    data_ok = *(int *)(buffer + 12);
    //    std::cout << "Syncframe:" << local_ctr << " " << global_ctr << " " << sma_state << " " << dummy << " " << device_nr << " " << data_ok << std::endl;

    return 0;
  };

  int write(std::ofstream *file)
  {
    char buffer[16];
    *(unsigned short *)(buffer + 0) = local_ctr;
    *(unsigned short *)(buffer + 2) = global_ctr;
    *(unsigned short *)(buffer + 4) = sma_state;
    *(unsigned short *)(buffer + 6) = dummy;
    *(int *)(buffer + 8) = device_nr;
    *(int *)(buffer + 12) = data_ok;

    file->write(buffer, 16);

    if (file->fail())
      return 1;

    return 0;
  }

  unsigned short local_ctr;
  unsigned short global_ctr;
  unsigned short sma_state;
  unsigned short dummy;
  int device_nr;
  unsigned int data_ok;
};

//*********************** Sensorframe *************************
class Boardframe
{
public:
  Boardframe(int nr_channels = 0)
  {
    debug("Boardframe()");

    data = NULL;
    resize(nr_channels);
  };

  Boardframe(const Boardframe &in)
  {
    debug("Boardframe(Boardframe&)");

    data = NULL;
    resize(in.nrChannels);
    for (int i = 0; i < nrChannels; i++)
      data[i] = in.data[i];
    syncframe = in.syncframe;
    fpgas = in.fpgas;
  };

  Boardframe &operator=(const Boardframe &in)
  {
    debug("Boardframe::operator==");

    resize(in.nrChannels); // creates an array called data of length nrChannels
    for (int i = 0; i < nrChannels; i++)
      data[i] = in.data[i];
    syncframe = in.syncframe;
    fpgas = in.fpgas;
    return *this;
  };

  ~Boardframe()
  {
    debug("~Boardframe()");

    if (data)
      delete[] data;
  };

  void resize(int nr_channels)
  {
    if (data)
      delete[] data;
    nrChannels = nr_channels;
    if (nrChannels)
      data = new signed short[nrChannels];
    else
      data = NULL;
  };

  int sizeInFile()
  {
    return syncframe.sizeInFile() + nrChannels * 2 + fpgas.sizeInFile();
  };

  int read(std::ifstream *file)
  {
    if (syncframe.read(file) == 1) // get the syncframe before the board data
      return 1;
    // I must be already resized at this point!
    file->read((char *)data, 2 * nrChannels);
    if (file->fail())
      return 1;
    if (fpgas.read(file) == 1) // get the fpgasframe after the board data
      return 1;
    // for print data out
    /*    std::cout<< "data[" << nrChannels << "]: ";
    for (int i = 0;i<nrChannels;i++) std::cout << data[i] << " ";
    std::cout << std::endl;*/

    return 0;
  };

  int write(std::ofstream *file)
  {
    if (!syncframe.write(file))
      return 1;

    file->write(reinterpret_cast<char *>(data), 2 * nrChannels);

    if (file->fail())
      return 1;
    
    if (!fpgas.write(file))
      return 1;

    return 0;
  }

  signed short &operator[](int index)
  {
    return data[index];
  };

  Syncframe syncframe;
  int nrChannels;
  signed short *data;
  Fpgasframe fpgas;
};

//*********************** Fullframe *************************
class Fullframe
{
public:
  Fullframe(int nr_boards = 0)
  {
    debug("Fullframe()");
    boards = NULL;
    resize(nr_boards);
  };

  Fullframe(const Fullframe &in)
  {
    debug("Fullframe(Fullframe&)");
    boards = NULL;
    resize(in.nrBoards);
    for (int i = 0; i < nrBoards; i++)
      boards[i] = in.boards[i];
  };

  Fullframe &operator=(const Fullframe &in)
  {
    debug("Fullframe::operator==");
    resize(in.nrBoards);
    for (int i = 0; i < nrBoards; i++)
      boards[i] = in.boards[i];

    return *this;
  };

  ~Fullframe()
  {
    debug("~Fullframe()");
    if (boards)
      delete[] boards;
  };

  void resize(int nr_boards)
  {
    if (boards)
      delete[] boards;
    nrBoards = nr_boards;
    if (nrBoards)
      boards = new Boardframe[nrBoards];
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
      return 1; // no boards, makes no sense...
  };

  int read(std::ifstream *file)
  {
    // Read number of boards
    unsigned short nr_boards;
    file->read((char *)&nr_boards, 2);
    if (file->fail() || nr_boards > 6)
    {
      std::cerr << "Unrealistic number of board to be read:" << nr_boards << std::endl;
      return 1;
    }
    //  std::cout << " nr_boards: " << nr_boards << std::endl;
    // Read channel counts
    unsigned short *channel_counts = new unsigned short[nr_boards];
    file->read((char *)channel_counts, nr_boards * 2);
    if (file->fail())
    {
      delete[] channel_counts;
      return 1;
    }

    // Read board frames
    resize(nr_boards);
    for (int board_nr = 0; board_nr < nr_boards; board_nr++)
    {
      //	std::cout << " channel_counts[" << board_nr << "]: "<< channel_counts[board_nr] << std::endl;

      boards[board_nr].resize(channel_counts[board_nr]);
      if (boards[board_nr].read(file) == 1) // read the board
      {
        delete[] channel_counts;
        return 1;
      }
    }

    delete[] channel_counts;
    return 0;
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
      if (boards[board_nr].write(file) == 1) // write the board
      {
        delete[] channel_counts;
        return 1;
      }
    }
    delete[] channel_counts;
    return 0;
  };

  TGraph *plot(int board_nr)
  {

    auto g = new TGraph();
    for (int i = 0; i < boards[board_nr].nrChannels; ++i)
    {
      g->SetPoint(i, i, boards[board_nr].data[i]);
    }
    g->Draw();

    return g;
  }

  int nrChannels()
  {
    int result = 0;
    for (int board_nr = 0; board_nr < nrBoards; board_nr++)
      result += boards[board_nr].nrChannels;
    return result;
  };

  signed short &operator[](int index)
  {
    for (int board_nr = 0; board_nr < nrBoards; board_nr++)
    {
      if (index >= boards[board_nr].nrChannels)
        index -= boards[board_nr].nrChannels;
      else
        return boards[board_nr][index];
    }

    std::cerr << " ### Fullframe::operator[]: index out of range!" << std::endl;
    //   return (*NULL); //this will cause crash (intended).
    return boards[nrBoards][index];
  };

  int nrBoards;
  Boardframe *boards;
};

//*********************** Hitdata *************************

class Hitdata
{
public:
  Hitdata(int nr_frames = 0)
  {
    frames = NULL;
    resize(nr_frames);
  };

  Hitdata(const Hitdata &in)
  {
    frames = NULL;
    resize(in.nrFrames);
    for (int i = 0; i < nrFrames; i++)
      frames[i] = in.frames[i];
  };

  Hitdata &operator=(const Hitdata &in)
  {
    resize(in.nrFrames);
    for (int i = 0; i < nrFrames; i++)
      frames[i] = in.frames[i];

    return *this;
  };

  ~Hitdata()
  {
    if (nrFrames)
      delete[] frames;
  };

  void resize(int nr_frames)
  {
    if (nrFrames)
      delete[] frames;
    nrFrames = nr_frames;
    if (nrFrames)
      frames = new Fullframe[nrFrames];
    else
      frames = NULL;
  };

  // Read data from a given file.
  // first_frame is the number of first frame to be read
  // nr_frames is the maximum number of frames to be read
  //-1 to read all of them
  // increment allows you reading once every nth sample
  // Return number of frames read or 0 in case of failure
  int readFile(char *filename, int first_frame = 0, int nr_frames = -1, int increment = 1)
  {
    std::ifstream file;
    // Open the file
    file.open(filename, ios_base::in | ios_base::binary);
    if (!file.is_open())
    {
      std::cerr << " ### Hitdata: File could not be open!" << std::endl;
      return 1; // file could not be opened
    }

    // Read first record to find board configuration
    Fullframe sampleframe;
    if (sampleframe.read(&file) == 1)
    {
      std::cerr << " ### Hitdata: First frame could not be read!" << std::endl;
      file.close();
      return 1;
    }

    // Check file size
    file.seekg(0, std::ios::beg);
    std::streamsize fsize = file.tellg();
    file.seekg(0, std::ios::end);
    fsize = file.tellg() - fsize;

    // Determine real frames to read
    unsigned int max_frames = (fsize / sampleframe.sizeInFile() - first_frame) / increment;
    if ((nr_frames == -1) || (max_frames < nr_frames))
      nr_frames = max_frames;

    std::cout << "     Hitdata: Nr frames to be read: " << nr_frames << std::endl;

    // Read!
    resize(nr_frames); // make an array called frames of size nr_frames #####!!!!
    file.seekg(first_frame * sampleframe.sizeInFile(), std::ios::beg);
    for (long int frame_nr = 0; frame_nr < nr_frames; frame_nr++)
    {
      /*	if ((frame_nr%100) == 0)
        std::cout << "        Frame " << frame_nr << std::endl;*/

      file.seekg((first_frame + frame_nr * increment) * sampleframe.sizeInFile(), std::ios::beg);
      if (frames[frame_nr].read(&file) == 1) // read the next frame
      {
        std::cerr << " ### Hitdata: Frame " << frame_nr << " could not be read!" << std::endl;
        file.close(); // read error, finish!
        // frames = frame_nr;	//Kinky! We decrease nr_frames, but the actual array size remains unchanged!
        ///???? I don't know what the above line does.
        return frame_nr;
      }
      //	std::cout << frames[frame_nr].nrBoards << std::endl;
    }

    // Finished
    file.close();
    return nr_frames;
  };

  Fullframe &operator[](int index)
  {
    if (index < nrFrames)
      return frames[index];
    else
    {
      std::cerr << " ### Hitdata::operator[]: index out of range!" << std::endl;
      // return (*NULL);	//this will cause crash (intended).
      return frames[index];
    }
  };

  int nrFrames;
  Fullframe *frames;
};
