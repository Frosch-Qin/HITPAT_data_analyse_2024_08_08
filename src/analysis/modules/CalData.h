//CalData.h
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"




class CalData : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "CalData";
    }

    void begin_run(const RunContext &ctx) override
    {

        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }

        get_calfac();

        
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        for (int i = 0; i < nrBoards; ++i)
        {
            for (int j = 0; j < frame.boards[i].nrChannels; ++j)
            {
                frame.boards[i].data[j] *= calFac[i][j];
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        // Finalization code here
    }

private:
    bool FPGA_calibrated = false;

    double calFac[6][320] = {0}; // 6 boards, 320 channels

    int nrBoards = 6;

    void get_calfac()
    {
        TFile *calFile = TFile::Open(Form("cal_pre/calibration_factor_run19/cal_run19.root"), "READ");

        if (!calFile || calFile->IsZombie())
        {
            std::cout << "Error: Could not open calibration file." << std::endl;
            std::cout << "Setting default calibration factors." << std::endl;
            //set default calibration factors
            for (int i = 0; i < 6; ++i)
            {
                for (int j = 0; j < 320; ++j)
                {
                    calFac[i][j] = 1.0; // Default calibration factor
                }
            }
            return;
        }

        TGraph *cal[nrBoards];  
        for (int i = 0; i < nrBoards; i++)
        {
            cal[i] = (TGraph *)calFile->Get(Form("cal%d", i));
        }

        for (int j = 0; j < nrBoards; j++)
        {
            for (int i = 0; i < 320; i++)
            {
                calFac[j][i] = cal[j]->GetPointY(i) / 8192;
                // std::cout << "Board " << j << " Channel " << i << " Uncalibration Factor: " << calFac[j][i] << std::endl;
                // calFac[j][i] = 1; //for run1 to run5
            }
        }
        calFile->Close();
    }
};
