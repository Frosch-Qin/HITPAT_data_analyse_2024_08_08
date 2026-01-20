#include <bitset>
#include <iomanip>
#include <map>
#include <functional>
#include <string>
#include <stdio.h>
#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm> // std::count
#include <utility>
#include <TLine.h>
#include <TH2.h>
#include <TF1.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystemDirectory.h>
#include <gsl/gsl_statistics.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <gsl/gsl_sort.h>
#include <TVector.h>
#include "TStopwatch.h"
#include "Math/MinimizerOptions.h"
#include "TVirtualFitter.h"
#include "TMatrixD.h"
#include "TApplication.h"
#include "TRint.h"
#include "TSystem.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include <TLine.h>
#include <type_traits>
#include <functional>
#include <TGraph2D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <queue>
#include "../plot/lhcbStyle.h" // custom LHCb style for plots

#include <gmp.h>   // for big number
#include <gmpxx.h> //for c++ class interface
#include <stdarg.h>
#include <obstack.h>
#include "../algo/for_register/MyHdReg.h" // a class for register bit manipulation


// include the data interface
#include "hitreader_float.h"

#include "../algo/HITNamespace.h"
// include the algorithm
#include "../algo/beamrecon.h"
#include "../algo/cluster_find.c"
#include "../algo/sum_peak.h"
#include "../algo/gaussian_fit.h"
#include "../algo/fas.h"
#include "../algo/fas_cluster.h"
#include "../algo/fas_3sigma.h"
#include "../algo/recon_grav_rms.c"
#include "../algo/recon_grav_rms_hd.c"
#include "../algo/simple_2by2.h"
#include "../algo/caruana.h"
#include "../algo/guo.h"
#include "../algo/rms_fitting.h"
#include "../algo/caruana_fitting.h"
#include "../algo/for_register/recon_grav_rms_hd_reg.c"

// algorithm and their map
// one should run define_algorithmMap() before use the map

template <typename T>
void sum_peak_c(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void sum_peak_fpga(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void gaussian_fit(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void fas(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void fas_cluster(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void fas_3sigma(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void recon_gravity_rms(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void recon_gravity_rms_hd(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void simple_2by2(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void caruana(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void guo(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void rms_fitting(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void caruana_fitting(T *p_frame_data, int boardNum, beamRecon *beam);
template <typename T>
void recon_gravity_rms_hd_reg(T *p_frame_data, int boardNum, beamRecon *beam);

// Define your function types

std::map<std::string, std::function<void(Fullframe *, int, beamRecon *)>> algorithmMap;
std::map<std::string, std::function<void(Fullframe_float *, int, beamRecon *)>> algorithmMap_float;

void define_algorithmMap()
{
  algorithmMap["sumpeakc"] = sum_peak_c<Fullframe>;
  algorithmMap["sumpeakfpga"] = sum_peak_fpga<Fullframe>;
  algorithmMap["gaussianfit"] = gaussian_fit<Fullframe>;
  algorithmMap["fas"] = fas<Fullframe>;
  algorithmMap["fascluster"] = fas_cluster<Fullframe>;
  algorithmMap["fas3sigma"] = fas_3sigma<Fullframe>;
  algorithmMap["grarms"] = recon_gravity_rms<Fullframe>;
  algorithmMap["grarmshd"] = recon_gravity_rms_hd<Fullframe>;
  algorithmMap["simple2by2"] = simple_2by2<Fullframe>;
  algorithmMap["caruana"] = caruana<Fullframe>;
  algorithmMap["guo"] = guo<Fullframe>;
  algorithmMap["rmsfitting"] = rms_fitting<Fullframe>;
  algorithmMap["caruanafitting"] = caruana_fitting<Fullframe>;
  algorithmMap["rmsreg"] = recon_gravity_rms_hd_reg<Fullframe>;

  algorithmMap_float["sumpeakc"] = sum_peak_c<Fullframe_float>;
  algorithmMap_float["sumpeakfpga"] = sum_peak_fpga<Fullframe_float>;
  algorithmMap_float["gaussianfit"] = gaussian_fit<Fullframe_float>;
  algorithmMap_float["fas"] = fas<Fullframe_float>;
  algorithmMap_float["fascluster"] = fas_cluster<Fullframe_float>;
  algorithmMap_float["fas3sigma"] = fas_3sigma<Fullframe_float>;
  algorithmMap_float["grarms"] = recon_gravity_rms<Fullframe_float>;
  algorithmMap_float["grarmshd"] = recon_gravity_rms_hd<Fullframe_float>;
  algorithmMap_float["simple2by2"] = simple_2by2<Fullframe_float>;
  algorithmMap_float["caruana"] = caruana<Fullframe_float>;
  algorithmMap_float["guo"] = guo<Fullframe_float>;
  algorithmMap_float["rmsfitting"] = rms_fitting<Fullframe_float>;
  algorithmMap_float["caruanafitting"] = caruana_fitting<Fullframe_float>;
  algorithmMap_float["rmsreg"] = recon_gravity_rms_hd_reg<Fullframe_float>;
}

// Average Alignment parameters:
// Board 0: slope = -1.17948 +- 0.000674353, intercept = 151.104 +- 0.0916188
// Board 1: slope = -1.15266 +- 0.000585865, intercept = 152.108 +- 0.0814702
// Board 2: slope = -1.16352 +- 0.000665237, intercept = 149.537 +- 0.0906004
// Board 3: slope = -1.13745 +- 0.000608628, intercept = 148.412 +- 0.0839875
// Board 4: slope = -1.14899 +- 0.000661942, intercept = 145.259 +- 0.0889251
// Board 5: slope = -1.12519 +- 0.000594419, intercept = 146.414 +- 0.082105

void alignment(beamRecon CPUrecon[])
{
  // values from the alignment run17,18,19 alignment/align4.C
  double intercept_scaled[6] = {129.79469, 133.77857, 128.53562, 130.49730, 124.82089, 128.73066};

  double slope_scaled[6] = {-1.0132711, -1.0135994, -1.0000000, -1.0000000, -0.98736398, -0.98927401};

  for (int boardNum = 0; boardNum < 6; boardNum++)
    CPUrecon[boardNum].Position = CPUrecon[boardNum].Position * slope_scaled[boardNum] + intercept_scaled[boardNum];
}

void signal_scale(beamRecon CPUrecon[])
{
  // double snrfac[6] = {10.2353/10.2353, 10.2353/10.0912, 10.2353/16.3871, 10.2353/2.48502, 10.2353/15.4513, 10.2353/10.8074};
  double signal_scale[6] = {1.0000000, 1.0142798, 0.62459496, 4.1187998, 0.66242323, 0.94706405};
  for (int boardNum = 0; boardNum < 6; boardNum++)
  {
    CPUrecon[boardNum].Peak = CPUrecon[boardNum].Peak * signal_scale[boardNum];
    CPUrecon[boardNum].Sum = CPUrecon[boardNum].Sum * signal_scale[boardNum];
  }
}

// get calfac from file

void set_framedata_one(Fullframe_float *frame)
{
  for (int i = 0; i < frame->nrBoards; i++)
  {
    for (int j = 0; j < frame->boards[i].nrChannels; j++)
    {
      frame->boards[i].data[j] = 1; // set all channels to 0
    }
  }
}

void get_calfac(TFile *calFile, Fullframe_float *calfac, int nrBoards)
{
  TGraph *cal[nrBoards];
  for (int i = 0; i < nrBoards; i++)
  {
    cal[i] = (TGraph *)calFile->Get(Form("cal%d", i));
  }

  for (int j = 0; j < nrBoards; j++)
  {
    for (int i = 0; i < 320; i++)
    {
      calfac->boards[j].data[i] = cal[j]->GetPointY(i) / 8192;
      // calfac->boards[j].data[i] = 1; //for run1 to run5
    }
  }
  calFile->Close();
}

template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void uncali_frame(T *sampleframe, Fullframe_float *uncalFac, Fullframe_float *sampleframe_uncal, int nrBoards)
{
  for (int j = 0; j < nrBoards; j++)
  {
    for (int i = 0; i < sampleframe->boards[j].nrChannels; i++)
    {
      sampleframe_uncal->boards[j].data[i] = sampleframe->boards[j].data[i] / uncalFac->boards[j].data[i];

    }
  }
}

void cali_frame(Fullframe_float *sampleframe_uncal, Fullframe_float *calFac, Fullframe_float *sampleframe, int nrBoards)
{

  for (int j = 0; j < nrBoards; j++)
  {
    for (int i = 0; i < sampleframe_uncal->boards[j].nrChannels; i++)
    {
      sampleframe->boards[j].data[i] = sampleframe_uncal->boards[j].data[i] * calFac->boards[j].data[i];

    }
  }
}

template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void common_mode_subtraction(T *sampleframe_uncal, Fullframe_float *sampleframe_uncal_base, int nrBoards)
{
  double baseline{};
  // get the baesline for common mode noise
  // it's equal to the first and last 4 channels
  for (int j : {0, 3, 4, 5}) // only for the board 0,3,4,5
  {
    baseline = 0;
    for (int k = 0; k < 4; k++)
    {
      baseline += sampleframe_uncal->boards[j].data[k];
      // baseline += sampleframe_uncal->boards[j].data[sampleframe_uncal->boards[j].nrChannels - 1 - k];
    }
    baseline /= 4;

    for (int k = 0; k < sampleframe_uncal->boards[j].nrChannels; k++)
    {
      sampleframe_uncal_base->boards[j].data[k] = sampleframe_uncal->boards[j].data[k] - baseline;
    }
  }
  for (int j : {1, 2}) // only for the board 1,2
  {
    for (int k = 0; k < sampleframe_uncal->boards[j].nrChannels; k++)
    {
      sampleframe_uncal_base->boards[j].data[k] = sampleframe_uncal->boards[j].data[k];
    }
  }
}

// return max_frames for the file
Long64_t get_max_frames(ifstream *file)
{
  Long64_t max_frames = 0;
  Fullframe sampleframe;
  sampleframe.read(file);

  file->seekg(0, std::ios::beg);
  std::streamsize fsize = file->tellg();
  file->seekg(0, std::ios::end);
  fsize = file->tellg() - fsize;
  file->seekg(0, std::ios::beg);
  // Determine real frames to read
  max_frames = fsize / sampleframe.sizeInFile();

  return max_frames;
}

// substract the pedestal
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void sub_pedestal(T *frame_data, Fullframe_float *frame_data_pedestal, double pedestal[6][320], int nrBoards)
{
  for (int j = 0; j < nrBoards; j++)
  {
    for (int k = 0; k < frame_data->boards[j].nrChannels; k++)
    {
      frame_data_pedestal->boards[j].data[k] = frame_data->boards[j].data[k] - pedestal[j][k];
    }
  }
}

// get the pedestal
// fixed nrFrames from beginning of the file
// one has to make sure no beam is on during the first nrFrames frames
Long64_t get_pedestal_temp(ifstream *file, Long64_t nrFrames, Fullframe_float *uncalFac, double pedestal[6][320], int nrBoards)
{

  Long64_t lastframeID = 0;
  // Initialize pedestal array to zero
  for (int j = 0; j < nrBoards; j++)
  {
    for (int k = 0; k < 320; k++)
    { // Assuming nrChannels is always <= 320
      pedestal[j][k] = 0.0;
    }
  }

  Fullframe sampleframe;
  sampleframe.read(file);
  file->seekg(0, std::ios::beg);

  Long64_t max_frames = get_max_frames(file);
  file->seekg(0, std::ios::beg);

  if (nrFrames > max_frames)
  {
    std::cout << "The number of frames in the file is less than the number of frames you want to read" << std::endl;
    return 1;
  }
  else
  {
    std::cout << "The number of frames in the file is " << max_frames << std::endl;
  }

  Fullframe_float sampleframe_uncal;
  Fullframe_float sampleframe_uncal_base;
  sampleframe_uncal = sampleframe;
  sampleframe_uncal_base = sampleframe;

  Long64_t nrFrames_pedestal = 0;

  for (Long64_t i = 0; i < max_frames; i++)
  {
    sampleframe.read(file);
    // uncali the data
    uncali_frame(&sampleframe, uncalFac, &sampleframe_uncal, nrBoards);
    // common mode subtraction
    // common_mode_subtraction(&sampleframe_uncal, &sampleframe_uncal_base, nrBoards);

    if (sampleframe.boards[0].fpgas.BKG_SUB_ON && nrFrames_pedestal < nrFrames)
    {
      for (int j = 0; j < nrBoards; j++)
      {
        for (int k = 0; k < sampleframe_uncal.boards[j].nrChannels; k++)
        {
          pedestal[j][k] += sampleframe_uncal.boards[j].data[k];
        }
        if (j == 0)
        {
          nrFrames_pedestal++;
        }
      }
    }
    if (nrFrames_pedestal == nrFrames)
    {
      std::cout << "The number of frames for pedestal is " << nrFrames_pedestal << std::endl;
      lastframeID = i;
      break;
    }
  }
  for (int j = 0; j < nrBoards; j++)
  {
    for (int k = 0; k < sampleframe_uncal.boards[j].nrChannels; k++)
    {
      pedestal[j][k] = pedestal[j][k] / nrFrames;
    }
  }

  file->seekg(0, std::ios::beg);
  std::cout << "The pedestal is done" << std::endl;
  return lastframeID;
}

// get the pedestal
// half of the nrFrames_beamoff for pedestal
void get_pedestal_all(ifstream *file, Fullframe_float *uncalFac, Long64_t *nrFrames_pedestal, double pedestal[6][320], int nrBoards)
{

  // Initialize pedestal array to zero
  for (int j = 0; j < nrBoards; j++)
  {
    for (int k = 0; k < 320; k++)
    { // Assuming nrChannels is always <= 320
      pedestal[j][k] = 0.0;
    }
  }

  Long64_t nrFrames[nrBoards]{0};

  double pedestal_temp[6][320]{0};
  get_pedestal_temp(file, 1e4, uncalFac, pedestal_temp, nrBoards);

  Fullframe sampleframe;
  sampleframe.read(file);
  file->seekg(0, std::ios::beg);

  Long64_t max_frames = get_max_frames(file);
  file->seekg(0, std::ios::beg);

  Long64_t max_pedestal = max_frames / 3;

  Fullframe_float sampleframe_uncal;
  Fullframe_float sampleframe_uncal_pedestal_base;
  Fullframe_float sampleframe_uncal_pedestal;
  sampleframe_uncal = sampleframe;
  sampleframe_uncal_pedestal_base = sampleframe;
  sampleframe_uncal_pedestal = sampleframe;

  beamRecon CPUrecon;

  for (Long64_t i = 0; i < max_frames; i++)
  {
    sampleframe.read(file);
    // uncali the data
    uncali_frame(&sampleframe, uncalFac, &sampleframe_uncal, nrBoards);
    // common mode subtraction
    // common_mode_subtraction(&sampleframe_uncal, &sampleframe_uncal_pedestal_base, nrBoards);
    // pedestal subtraction
    sub_pedestal(&sampleframe_uncal, &sampleframe_uncal_pedestal, pedestal_temp, nrBoards);

    // common mode subtraction
    common_mode_subtraction(&sampleframe_uncal_pedestal, &sampleframe_uncal_pedestal_base, nrBoards);

    if (sampleframe.boards[0].fpgas.BKG_SUB_ON)
    {
      for (int j = 0; j < nrBoards; j++)
      {
        if (!cluster_locate(&sampleframe_uncal_pedestal_base, j, &CPUrecon, 15, 4))
        {
          if (nrFrames[j] < nrFrames_pedestal[j])
          {
            nrFrames[j]++;
            for (int k = 0; k < sampleframe_uncal.boards[j].nrChannels; k++)
            {
              pedestal[j][k] += sampleframe_uncal.boards[j].data[k];
            }
          }
        }
      }
    }
  }
  for (int j = 0; j < nrBoards; j++)
  {
    for (int k = 0; k < sampleframe_uncal_pedestal_base.boards[j].nrChannels; k++)
    {
      pedestal[j][k] = pedestal[j][k] / nrFrames[j];
    }
  }

  file->seekg(0, std::ios::beg);
  std::cout << "The pedestal is done" << std::endl;
  // output nrFrames
  for (int j = 0; j < nrBoards; j++)
  {
    std::cout << "The number of frames for pedestal for board " << j << " is " << nrFrames[j] << std::endl;
  }
}

vector<double> spill_start_time;
vector<double> spill_end_time;
// run22 clsize = 4
void get_spill_time(const char *run_name, int clsize)
{
  TFile *file = TFile::Open(Form("../beam_on_status/rootfile/%s_%d_beam_on_status.root", run_name, clsize), "READ");
  if (!file || !file->IsOpen())
  {
    std::cerr << "Error opening file: " << Form("../beam_on_status/rootfile/%s_%d_beam_on_status.root", run_name, clsize) << std::endl;
    return;
  }
  TTree *tree = (TTree *)file->Get("spill_info_tree");

  double x;
  double y;
  tree->SetBranchAddress("spill_start", &x);
  tree->SetBranchAddress("spill_end", &y);

  std::cout << tree->GetEntries() << " spills read from beam_on_status.root file." << std::endl;

  for (int i = 0; i < tree->GetEntries(); i++)
  {
    tree->GetEntry(i);
    spill_start_time.push_back(x);
    spill_end_time.push_back(y);
    std::cout << "Spill " << i << ": Start = " << x << ", End = " << y <<", Duration = " << (y - x) << std::endl;
  }
  file->Close();

  return;
}

double NOISE_UNCORRELATED[6];
double NOISE_COMMON_MODE[6];

void get_noise_array(const char *run_name)
{
  TFile *file = TFile::Open(Form("../get_noise/rootfile/%s_noise.root", run_name), "READ");
  if (!file || !file->IsOpen())
  {
    std::cerr << "Error opening file: " << Form("../get_noise/rootfile/%s_noise.root", run_name) << std::endl;
    return;
  }
  for (int i = 0; i < 6; i++){
    NOISE_COMMON_MODE[i] = ((TH1D*)file->Get(Form("noise_common_mode_%d", i)))->GetStdDev();
    NOISE_UNCORRELATED[i] = ((TH1D*)file->Get(Form("noise_uncorrelated_%d", i)))->GetMean();
    std::cout << "Board " << i << ": Common Mode Noise = " << NOISE_COMMON_MODE[i] << ", Uncorrelated Noise = " << NOISE_UNCORRELATED[i] << std::endl;
  }
  file->Close();
  return;
}