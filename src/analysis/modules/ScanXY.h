// output TH2D histogram for positions

// ScanXY.h

#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class ScanXY : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "ScanXY";
    }

    void begin_run(const RunContext &ctx) override
    {

        file_ = new TFile(Form("output/run%d_ScanXY.root", ctx.run_number), "RECREATE");
        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            H_boardID[i] = ctx.H_boardID[i];
            V_boardID[i] = ctx.V_boardID[i];
        }
        createHistograms(ctx);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            // Fill the 2D histogram with the hit positions
            if (tags.SpillID >= 0) // if it is during a spill
            {

                Pos2D[i]->Fill(tags.boardTags[H_boardID[i]].Position, tags.boardTags[V_boardID[i]].Position);
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        if (file_)
        {
            dir_->cd();
            for (int i = 0; i < nrBoards / 2; ++i)
            {
                // go through the content, if content is 1, set it to 0
                for (int ix = 1; ix <= Pos2D[i]->GetNbinsX(); ++ix)
                {
                    for (int iy = 1; iy <= Pos2D[i]->GetNbinsY(); ++iy)
                    {
                        if (Pos2D[i]->GetBinContent(ix, iy) <= 2)
                        {
                            Pos2D[i]->SetBinContent(ix, iy, 0);
                        }
                    }
                }
                Pos2D[i]->Write();
            }
        }
        file_->Close();
    }

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    int nrBoards = 6;
    int V_boardID[3];
    int H_boardID[3];

    TH2D *Pos2D[3]; // prepare for 3 stations

    void createHistograms(const RunContext &ctx)
    {

        if (!file_)
            return;
        dir_ = file_->GetDirectory("ScanXY");
        if (!dir_)
            dir_ = file_->mkdir("ScanXY");
        dir_->cd();

        for (int i = 0; i < nrBoards / 2; ++i)
        {
            Pos2D[i] = new TH2D(Form("Pos2D_H%dV%d", i, i), Form("Pos2D_H%dV%d", i, i), 2560, -128, 128, 2560, -128, 128);

            Pos2D[i]->GetXaxis()->SetTitle(Form("H%d position [mm]", i));
            Pos2D[i]->GetYaxis()->SetTitle(Form("V%d position [mm]", i));
        }
    }

    // return the scan boundaries
    void findpeaks(TH1D *hist, int peakNum)
    {
        // find the first peak and last peak
        int firstPeak = hist->FindFirstBinAbove(10);
        int lastPeak = hist->FindLastBinAbove(10);
        int estimate_x_pitch = (lastPeak - firstPeak) / (peakNum - 1);

        //go through the peak one by one to get their positions

        


        // Implement peak finding algorithm here
    }
};