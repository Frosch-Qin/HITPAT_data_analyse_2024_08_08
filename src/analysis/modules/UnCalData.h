// UnCalData.h
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class UnCalData : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "UnCalData";
    }

    protected:
    void on_begin_run(const RunContext &ctx) override
    {

        FPGA_calibrated = ctx.FPGA_calibrated;
        if (FPGA_calibrated)
        {
            // READ FPGA calibration factor
            get_uncalfac();
            
        }
        else
        {

            for (int i = 0; i < 6; ++i)
            {
                for (int j = 0; j < 320; ++j)
                {
                    uncalFac[i][j] = 1.0; // Default uncalibrated factor
                }
            }
        }
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        for (int i = 0; i < nrBoards; ++i)
        {
            for (int j = 0; j < frame.boards[i].nrChannels; ++j)
            {
                frame.boards[i].data[j] /= uncalFac[i][j];
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        // Finalization code here
    }

private:
    bool FPGA_calibrated = false;

    double uncalFac[6][320] = {0}; // 6 boards, 320 channels


    void get_uncalfac()
    {
        TFile *calFile = TFile::Open(Form("cal_fac_testbeam/cal.root"), "READ");
        if (!calFile || calFile->IsZombie())
        {
            std::cerr << "Error: Could not open calibration file." << std::endl;
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
                uncalFac[j][i] = cal[j]->GetPointY(i) / 8192;
                // std::cout << "Board " << j << " Channel " << i << " Uncalibration Factor: " << uncalFac[j][i] << std::endl;
                // uncalFac[j][i] = 1; //for run1 to run5
            }
        }
        calFile->Close();
    }
};
