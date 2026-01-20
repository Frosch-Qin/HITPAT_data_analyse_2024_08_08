
// get the position histogram for rectanglar scan
#include "../hitreader/analyser.h"
#include "histManager_beam_on_status.c"
/*
lb-conda default/2021-04-03 bash
make clean;
make;

 */

int reconstucting(const char *run_file_path, const char *root_file_path, const char *algo, int cl_size = 4)
{
    // define the algorithm map for the reconstruction
    define_algorithmMap();

    int cl_threshold = 15; //15
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
    double toteltime = max_frames * 1e-4;
    HistogramManager histManager(rootFile);
    histManager.createHistograms(toteltime);

    // create space for nrBoards number of boards
    Fpgasframe FPGArecon[nrBoards];
    beamRecon CPUrecon[nrBoards];

    long long int eventID;

    /********open and get Calibration factor for fpga run and to uncalibrated the data**********/

    TFile *uncalFile = TFile::Open(Form("../cal_fac_testbeam/cal.root"), "READ");
    // init uncalFac
    Fullframe_float uncalFac;
    uncalFac = sampleframe;
    get_calfac(uncalFile, &uncalFac, nrBoards);

    /********open and get Calibration factor and prepare calibrated data**********/

    TFile *calFile = TFile::Open(Form("../cal_pre/calibration_factor_run19/cal_run19.root"), "READ");
    Fullframe_float calfac;
    calfac = sampleframe;
    get_calfac(calFile, &calfac, nrBoards);

    /*************** prepare the fullframe container******************************************/
    Fullframe_float sampleframe_uncal;
    sampleframe_uncal = sampleframe;
    // pedestal subtracted data
    Fullframe_float sampleframe_uncal_pedestal;
    sampleframe_uncal_pedestal = sampleframe;
    // common mode subtracted data
    Fullframe_float sampleframe_uncal_pedestal_base;
    sampleframe_uncal_pedestal_base = sampleframe;
    // calibrated data
    Fullframe_float sampleframe_cal;
    sampleframe_cal = sampleframe;

    double pedestal[nrBoards][320]{};
    get_pedestal_temp(&file, 1e4, &uncalFac, pedestal, nrBoards);

    /*********************************** Loop over the frames and do the reconstruction ************************/
    double eventTime;

    // std::queue<int> has_cluster_box10[6];
    // std::queue<int> has_cluster_box3[6];
    // const int box3_size = 3; // 3 frames in a row; when it is 1, it shows not found beam rate
    // const int box10_size = 10;
    // int beam_on_10state[6]{};
    // int beam_on_3state[6]{};

    for (long long int i = 0; i < max_frames; i++)
    {

        if (i % 10000 == 0)
        {
            cout << i << " frames analyse done " << endl;
        }
        eventID = i;
        eventTime = i * 1e-4;
        // read data to sampleframe
        file.seekg(i * sampleframe.sizeInFile(), std::ios::beg);
        sampleframe.read(&file);

        // unclibrate the data
        uncali_frame(&sampleframe, &uncalFac, &sampleframe_uncal, nrBoards);

        // pedestal subtraction
        sub_pedestal(&sampleframe_uncal, &sampleframe_uncal_pedestal, pedestal, nrBoards);

        // common mode subtraction
        common_mode_subtraction(&sampleframe_uncal_pedestal, &sampleframe_uncal_pedestal_base, nrBoards);

        // calibrate the data
        cali_frame(&sampleframe_uncal_pedestal_base, &calfac, &sampleframe_cal, nrBoards);


        // get the FPGArecon from the sampleframe
        for (int j = 0; j < nrBoards; j++)
        {
            FPGArecon[j] = sampleframe.boards[j].fpgas;
        }

        // get the beam noise; snr; channel_avg4,avg8,avg16 info
        for (int j = 0; j < nrBoards; j++)
        {
            // initialize the beamRecon structure
            CPUrecon[j] = beamRecon_beamoff;

            if (FPGArecon[j].BKG_SUB_ON)
            {
                // get_beaminfo(&sampleframe_uncal_pedestal_base, j, &CPUrecon[j]);
                // if (cluster_locate(&sampleframe, j, &CPUrecon[j], cl_threshold[j], 4))
                if (cluster_locate(&sampleframe_uncal_pedestal_base, j, &CPUrecon[j], cl_threshold, cl_size))
                {
                    //   algorithmMap_float[(std::string)algo](&sampleframe_cal, j, &CPUrecon[j]);
                    //   sum_var_rsqr(&sampleframe_cal, j, &CPUrecon[j]);
                }
            }

        //     // for judge the beam on state
        //     if (has_cluster_box10[j].size() < box10_size - 1)
        //     {
        //         has_cluster_box10[j].push(CPUrecon[j].Cluster_num);
        //     }
        //     else
        //     {
        //         has_cluster_box10[j].push(CPUrecon[j].Cluster_num);
        //         double beam_on_judge = 0;
        //         std::queue<int> temp = has_cluster_box10[j];
        //         for (int i = 0; i < box10_size; i++)
        //         {
        //             beam_on_judge += temp.front();
        //             temp.pop();
        //         }
        //         if (beam_on_judge == 10)
        //         {
        //             beam_on_10state[j] = 1;
        //         }
        //         if (beam_on_judge == 0)
        //         {
        //             beam_on_10state[j] = 0;
        //         }
        //         CPUrecon[j].Beam_on = beam_on_10state[j];
        //         has_cluster_box10[j].pop();
        //     }
        //     // for judge the beam on state for 3 in a row
        //     if (has_cluster_box3[j].size() < box3_size - 1)
        //     {
        //         has_cluster_box3[j].push(CPUrecon[j].Cluster_num);
        //     }
        //     else
        //     {
        //         has_cluster_box3[j].push(CPUrecon[j].Cluster_num);
        //         double beam_on_judge = 0;
        //         std::queue<int> temp = has_cluster_box3[j];
        //         for (int i = 0; i < box3_size; i++)
        //         {
        //             beam_on_judge += temp.front();
        //             temp.pop();
        //         }
        //         if (beam_on_judge == 3)
        //         {
        //             beam_on_3state[j] = 1;
        //         }
        //         if (beam_on_judge == 0)
        //         {
        //             beam_on_3state[j] = 0;
        //         }
        //         CPUrecon[j].Beam_on_judge1 = beam_on_3state[j];
        //         has_cluster_box3[j].pop();
        //     }
        }

        // alignment(CPUrecon);
        // signal_scale(CPUrecon);
        histManager.fillHistograms(CPUrecon, eventTime);
    }

    // close input file
    file.close();
    rootFile->cd();
    histManager.writeHistograms();

    rootFile->Close();

    return 0;
}

int main(Int_t argc, const char *argv[])

{

    auto run_name = argv[1];
    auto cl_size = atoi(argv[2]);
    //   auto algo = argv[2];
    string run_file_path = Form("/auto/data/qinliqing/hitdata/HIT_%s/%s.da2", DATASOURCE.c_str(), run_name);
    string algo2 = "grarms";
    // string algo2 = "gaussianfit";
    string root_file_path = Form("rootfile/%s_%d_beam_on_status.root", run_name, cl_size);

    reconstucting(run_file_path.c_str(), root_file_path.c_str(), algo2.c_str(), cl_size);

    return 0;
}
