
// get the position histogram for rectanglar scan
#ifndef ANALYSER_H
#define ANALYSER_H
#include "../hitreader/analyser.h"
#endif

/*
lb-conda default/2021-04-03 bash
make clean;
make;

 */

int reconstucting(const char *run_name, const char *run_file_path, const char *root_file_path, const char *algo, int clsize, const char *beamplan_type)
{

  /*******************Open the .da2 file collected from HIT*************************************/
  ifstream file;
  file.open(run_file_path, ifstream::in | ifstream::binary);
  if (!file.is_open())
  {
    std::cout << Form("%s  file failed to open", run_file_path) << std::endl;
    return 1;
  }
  else
  {
    std::cout << Form("%s  file open successfully", run_file_path) << std::endl;
  }

  Fullframe sampleframe;
  sampleframe.read(&file);
  file.seekg(0, std::ios::beg);
  int nrBoards = sampleframe.nrBoards;
  int nrChannels[nrBoards];

  std::cout << "boards number: " << nrBoards << std::endl;
  for (int i = 0; i < nrBoards; i++)
  {
    nrChannels[i] = sampleframe.boards[i].nrChannels;
    std::cout << "board " << i << " channel number: " << sampleframe.boards[i].nrChannels << std::endl;
  }

  // get the max frames
  Long64_t max_frames = get_max_frames(&file);
  std::cout << "     Hitdata: Nr frames to be read: " << max_frames << std::endl;

  double toteltime = max_frames * 1e-4;

  // create space for nrBoards number of boards
  Fpgasframe FPGArecon[nrBoards];

  /********open and get Calibration factor for fpga run and to uncalibrated the data**********/

  TFile *uncalFile = TFile::Open(Form("../cal_fac_testbeam/cal.root"), "READ");
  // init uncalFac
  Fullframe_float uncalFac;
  uncalFac = sampleframe;
  get_calfac(uncalFile, &uncalFac, nrBoards);

  /********open and get Calibration factor and prepare calibrated data**********/

  TFile *calFile = TFile::Open(Form("../cal_pre/calibration_factor_run19/cal_run20.root"), "READ");
  // TFile *calFile = TFile::Open(Form("../cal_pre/calibration_factor_%s/cal_%s.root", (strcmp(beamplan_type, "Lscan") == 0 ? "linescanH" : "run19"), (strcmp(beamplan_type, "Lscan") == 0 ? "linescanH" : "run19")), "READ");

  Fullframe_float calfac;
  calfac = sampleframe;
  // If the calFile does not exist, set the calfac to 1
  if (!calFile || !calFile->IsOpen())
  {
    std::cout << "Calibration file not found, using default calibration factor of 1." << std::endl;
    set_framedata_one(&calfac);
  }
  else
  {
    get_calfac(calFile, &calfac, nrBoards);
  }

  double pedestal0[nrBoards][320]{};                  // uncal
  double pedestal1[nrBoards][320]{};                  // uncal
  double sum_all[nrBoards][320]{};                    // uncal
  double sum_all_pedestal0_sub[nrBoards][320]{};      // uncal
  double sum_all_pedestal0_sub_cali[nrBoards][320]{}; // cal

  long int frameNum_pedestal0 = 0;
  long int frameNum_pedestal1 = 0;
  long int frameNum_sum_all = 0;

  // get spill info

  int spillNum = spill_start_time.size();

  double sum_all_pedestal0_sub_spill[spillNum][nrBoards][320]{};
  double sum_all_pedestal0_sub_cali_spill[spillNum][nrBoards][320]{};
  double frameNum_sum_all_spill[spillNum]{};

  int inter_spill_ctr = 0;
  int inter_spill = 0; //  1 inside spill; 0 even no spill; -1 odd no spill

  int spillID_ctr = 0;
  int spillID = -1;

  /*********************************** Loop over the frames and do the reconstruction ************************/
  double eventTime;
  for (long long int i = 0; i < max_frames; i++)
  {
    if (i % 10000 == 0)
    {
      cout << i << " frames analyse done " << endl;
    }
    // read data to sampleframe
    file.seekg(i * sampleframe.sizeInFile(), std::ios::beg);
    sampleframe.read(&file);

    // get the FPGArecon from the sampleframe
    for (int j = 0; j < nrBoards; j++)
    {
      FPGArecon[j] = sampleframe.boards[j].fpgas;
    }

    // judge if the spill is on or not
    eventTime = i * 1e-4;

    if (inter_spill_ctr >= spill_start_time.size())
    {
      // std::cout << "No more spill info available." << std::endl;
      inter_spill = 0;
    }
    else
    {
      if (eventTime < spill_start_time[inter_spill_ctr])
      {
        // before the first spill
        inter_spill = ((inter_spill_ctr % 2 == 0) ? 0 : -1);
      }
      else if (eventTime < spill_end_time[inter_spill_ctr])
      {
        // inside the spill
        inter_spill = 1;
      }
      else
      {
        inter_spill = ((inter_spill_ctr % 2 == 0) ? -1 : 0);
        inter_spill_ctr++;
      }
    }

    // get spillID

    if (spillID_ctr >= spillNum)
    {
      // std::cout << "No more spill info available." << std::endl;
      spillID = -1;
    }
    else
    {
      if (eventTime < spill_start_time[spillID_ctr])
      {
        // before the first spill
        spillID = -1;
      }
      else if (eventTime < spill_end_time[spillID_ctr])
      {
        // inside the spill
        spillID = spillID_ctr;
      }
      else
      {
        spillID = -1;
        spillID_ctr++;
      }
    }

    // for run34 combine spill 1 and spill 2
    if (strcmp(run_name, "run34") == 0)
    {

      if (spillID != -1)
      {
        if (spillID >= 1)
        {
          spillID = spillID - 1;
        }
      }
    }

    if (FPGArecon[0].BKG_SUB_ON)
    {
      if (inter_spill == 1)
      {
        frameNum_sum_all++;
        frameNum_sum_all_spill[spillID]++;
        for (int j = 0; j < nrBoards; j++)
        {
          for (int k = 0; k < sampleframe.boards[j].nrChannels; k++)
          {
            sum_all[j][k] += sampleframe.boards[j].data[k];
            if (spillID > -1)
              sum_all_pedestal0_sub_spill[spillID][j][k] += sampleframe.boards[j].data[k];
          }
        }
      }
      else if (inter_spill == 0)
      {
        // if not in spill, then do the pedestal subtraction
        frameNum_pedestal0++;
        for (int j = 0; j < nrBoards; j++)
        {
          for (int k = 0; k < sampleframe.boards[j].nrChannels; k++)
          {
            pedestal0[j][k] += sampleframe.boards[j].data[k];
          }
        }
      }
      else if (inter_spill == -1)
      {
        // if not in spill, then do the pedestal subtraction
        frameNum_pedestal1++;
        for (int j = 0; j < nrBoards; j++)
        {
          for (int k = 0; k < sampleframe.boards[j].nrChannels; k++)
          {
            pedestal1[j][k] += sampleframe.boards[j].data[k];
          }
        }
      }
    }
  }

  // get average and uncal and cali
  for (int j = 0; j < nrBoards; j++)
  {
    for (int k = 0; k < sampleframe.boards[j].nrChannels; k++)
    {
      pedestal0[j][k] = pedestal0[j][k] / frameNum_pedestal0 / uncalFac.boards[j].data[k];
      pedestal1[j][k] = pedestal1[j][k] / frameNum_pedestal1 / uncalFac.boards[j].data[k] - pedestal0[j][k];

      sum_all[j][k] = sum_all[j][k] / frameNum_sum_all / uncalFac.boards[j].data[k];

      sum_all_pedestal0_sub[j][k] = sum_all[j][k] - pedestal0[j][k];
      sum_all_pedestal0_sub_cali[j][k] = sum_all_pedestal0_sub[j][k] * calfac.boards[j].data[k];
    }
    for (int m = 0; m < spillNum; m++)
    {
      for (int k = 0; k < sampleframe.boards[j].nrChannels; k++)
      {
        sum_all_pedestal0_sub_spill[m][j][k] = sum_all_pedestal0_sub_spill[m][j][k] / frameNum_sum_all_spill[m] / uncalFac.boards[j].data[k] - pedestal0[j][k];
        sum_all_pedestal0_sub_cali_spill[m][j][k] = sum_all_pedestal0_sub_spill[m][j][k] * calfac.boards[j].data[k];
      }
    }
  }

  std::cout << "pedestal0 frames: " << frameNum_pedestal0 << std::endl;
  std::cout << "pedestal1 frames: " << frameNum_pedestal1 << std::endl;
  std::cout << "sum_all frames: " << frameNum_sum_all << std::endl;

  /*****************open and name the roof file************************************************************/
  TFile *rootFile = new TFile(root_file_path, "RECREATE");
  if (rootFile->IsOpen())
  {
    std::cout << Form("%s opened sucessfully", root_file_path) << endl;
  }
  else
  {
    std::cout << Form("%s failed to open", root_file_path) << endl;
    return 1;
  }
  // create 5*nrBoards histograms and store the data

  rootFile->cd();

  TGraph *pedestal1_graph[nrBoards];
  TGraph *pedestal0_graph[nrBoards];
  TGraph *sum_all_graph[nrBoards];
  TGraph *sum_all_pedestal0_sub_graph[nrBoards];
  TGraph *sum_all_pedestal0_sub_cali_graph[nrBoards];
  TGraph *sum_all_pedestal0_sub_cali_spill_graph[spillNum][nrBoards];
  TGraph *sum_all_pedestal0_sub_spill_graph[spillNum][nrBoards];
  for (int i = 0; i < nrBoards; i++)
  {
    pedestal1_graph[i] = new TGraph();
    pedestal0_graph[i] = new TGraph();
    sum_all_graph[i] = new TGraph();
    sum_all_pedestal0_sub_graph[i] = new TGraph();
    sum_all_pedestal0_sub_cali_graph[i] = new TGraph();

    // set name
    pedestal1_graph[i]->SetName(Form("pedestal1_sub_pedestal0_%d", i));
    pedestal0_graph[i]->SetName(Form("pedestal0_%d", i));
    sum_all_graph[i]->SetName(Form("sum_all_%d", i));
    sum_all_pedestal0_sub_graph[i]->SetName(Form("sum_all_pedestal0_sub_%d", i));
    sum_all_pedestal0_sub_cali_graph[i]->SetName(Form("sum_all_pedestal0_sub_cali_%d", i));

    // set data
    for (int j = 0; j < sampleframe.boards[i].nrChannels; j++)
    {
      pedestal1_graph[i]->SetPoint(j, j, pedestal1[i][j]);
      pedestal0_graph[i]->SetPoint(j, j, pedestal0[i][j]);
      sum_all_graph[i]->SetPoint(j, j, sum_all[i][j]);
      sum_all_pedestal0_sub_graph[i]->SetPoint(j, j, sum_all_pedestal0_sub[i][j]);
      sum_all_pedestal0_sub_cali_graph[i]->SetPoint(j, j, sum_all_pedestal0_sub_cali[i][j]);
    }

    for (int m = 0; m < spillNum; m++)
    {
      sum_all_pedestal0_sub_spill_graph[m][i] = new TGraph();
      sum_all_pedestal0_sub_cali_spill_graph[m][i] = new TGraph();

      // set name
      sum_all_pedestal0_sub_spill_graph[m][i]->SetName(Form("sum_all_pedestal0_sub_spill_%d_%d", m, i));
      sum_all_pedestal0_sub_cali_spill_graph[m][i]->SetName(Form("sum_all_pedestal0_sub_cali_spill_%d_%d", m, i));

      // set data
      for (int j = 0; j < sampleframe.boards[i].nrChannels; j++)
      {
        sum_all_pedestal0_sub_spill_graph[m][i]->SetPoint(j, j, sum_all_pedestal0_sub_spill[m][i][j]);
        sum_all_pedestal0_sub_cali_spill_graph[m][i]->SetPoint(j, j, sum_all_pedestal0_sub_cali_spill[m][i][j]);
      }
    }
  }

  // save the graphs
  for (TGraph *graph : pedestal1_graph)
  {
    graph->Write();
  }
  for (TGraph *graph : pedestal0_graph)
  {
    graph->Write();
  }
  for (TGraph *graph : sum_all_graph)
  {
    graph->Write();
  }
  for (TGraph *graph : sum_all_pedestal0_sub_graph)
  {
    graph->Write();
  }
  for (TGraph *graph : sum_all_pedestal0_sub_cali_graph)
  {
    graph->Write();
  }
  TDirectory *spill_sum = rootFile->mkdir("spill_sum");
  spill_sum->cd();
  for (int m = 0; m < spillNum; m++)
  {
    for (int i = 0; i < nrBoards; i++)
    {
      sum_all_pedestal0_sub_spill_graph[m][i]->Write();
      sum_all_pedestal0_sub_cali_spill_graph[m][i]->Write();
    }
  }

  // close input file
  file.close();
  rootFile->cd();

  rootFile->Close();

  return 0;
}

int reconstruct_one_run(const char *run_name, const char *algo, int clsize, const char *beamplan_type)

{

  // const char* is not stable.... the fit function will write this part of the memory and change the value of the const char....
  string run_file_path = Form("/auto/data/qinliqing/hitdata/HIT_%s/%s.da2", DATASOURCE.c_str(), run_name);

  string root_file_path = Form("../sum_signal1d/rootfile/%s_%d_sum1d.root", run_name, clsize);

  // get spill time
  get_spill_time(run_name, clsize);

  reconstucting(run_name, run_file_path.c_str(), root_file_path.c_str(), algo, clsize, beamplan_type);
  return 0;
}

int main(int argc, char *argv[])
{
  auto run_name = argv[1];
  auto clsize = atoi(argv[2]);
  string algo = "none";
  if (argc < 3)
  {
    string beamplan_type = "";
    reconstruct_one_run(run_name, algo.c_str(), clsize, beamplan_type.c_str());
  }
  else
  {
    auto beamplan_type = argv[3];
    // string beamplan_type = "none";
    reconstruct_one_run(run_name, algo.c_str(), clsize, beamplan_type);
  }
  // reconstruct_one_run(run_name, algo2.c_str(), clsize);
}
