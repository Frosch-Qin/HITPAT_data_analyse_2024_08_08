// Pos2DHH.h
#pragma once

#include "../AnalysisPipeline.h"

class Pos2DHH final : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "Pos2DHH";
    }
    protected:
    void on_begin_run(const RunContext &ctx) override
    {
        file_ = new TFile(Form("output2025/run%d_Pos2DHH.root", ctx.run_number), "RECREATE");

        createHistograms(ctx);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        for (int i = 0; i < nrBoards / 2 - 1; ++i)
        {
            // Fill the 2D histogram with the hit positions
            if (tags.SpillID >= 0) // if it is during a spill
            {

                PosHH[i]->Fill(tags.boardTags[H_boardID[i + 1]].Position, tags.boardTags[H_boardID[0]].Position);
                PosVV[i]->Fill(tags.boardTags[V_boardID[i + 1]].Position, tags.boardTags[V_boardID[0]].Position);
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        if (file_)
        {
            dir_->cd();
            for (int i = 0; i < nrBoards / 2 - 1; ++i)
            {
                PosHH[i]->Write();
                PosVV[i]->Write();
            }
        }
        file_->Close();
    }

private:

    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    TH2D *PosHH[2];
    TH2D *PosVV[2];

    // Linear Fit
    TF1 *alignHH[2];
    TF1 *alignVV[2];

    

    void LinearFit(TH2D *h, TH1 *f)
    {
        
        TH2D *hc = (TH2D *)h->Clone("h_thr");
        const int nx = hc->GetNbinsX();
        const int ny = hc->GetNbinsY();

        double thr = 5; //threshold for cutting away some bins with fewer counts
        for (int ix = 1; ix <= nx; ++ix)
        {
            for (int iy = 1; iy <= ny; ++iy)
            {
                if (hc->GetBinContent(ix, iy) < thr)
                {
                    hc->SetBinContent(ix, iy, 0.0);
                }
            }
        }
    }

    void createHistograms(const RunContext &ctx)
    {

        if (!file_)
            return;
        dir_ = file_->GetDirectory("Pos2DHH");
        if (!dir_)
            dir_ = file_->mkdir("Pos2DHH");
        dir_->cd();

        for (int i = 0; i < nrBoards / 2 - 1; ++i)
        {
            PosHH[i] = new TH2D(Form("PosH%dH%d", i + 1, 0), Form("PosH%dH%d", i + 1, 0), 2560, 0, 256, 2560, 0, 256);

            PosHH[i]->GetXaxis()->SetTitle(Form("H%d position [mm]", 0));
            PosHH[i]->GetYaxis()->SetTitle(Form("H%d position [mm]", i + 1));

            PosVV[i] = new TH2D(Form("PosV%dV%d", i + 1, 0), Form("PosV%dV%d", i + 1, 0), 2560, 0, 256, 2560, 0, 256);

            PosVV[i]->GetXaxis()->SetTitle(Form("V%d position [mm]", 0));
            PosVV[i]->GetYaxis()->SetTitle(Form("V%d position [mm]", i + 1));
        }
    }
};