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

protected:
    void on_begin_run(const RunContext &ctx) override
    {

        file_ = new TFile(Form("output/run%d_ScanXY.root", ctx.run_number), "RECREATE");

        createHistograms(ctx);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            // Fill the 2D histogram with the hit positions
            if (tags.SpillID >= 0) // if it is during a spill
            {

                Pos2D[i]->Fill(tags.boardTags[H_boardID[i]].Position - offset_origin, tags.boardTags[V_boardID[i]].Position - offset_origin);
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
            file_->Close();
        }
    }

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;
    const double offset_origin = 128.4;

    TH2D *Pos2D[3] = {nullptr, nullptr, nullptr}; // prepare for 3 stations

    void createHistograms(const RunContext &ctx);

};
// inline: later it may show up in multiple translation units.
inline void ScanXY::createHistograms(const RunContext &ctx)
{
    if (!file_)
        return;
   
    dir_ = file_->GetDirectory("ScanXY");
    if (!dir_)
        dir_ = file_->mkdir("ScanXY");

    dir_->cd();

    for (int i = 0; i < nrBoards / 2; ++i)
    {
        const char* HboardName = ctx.BoardName[ctx.H_boardID[i]];
        const char* VboardName = ctx.BoardName[ctx.V_boardID[i]];

        Pos2D[i] = new TH2D(Form("Pos2D_%s%s",HboardName, VboardName), Form("Pos2D_%s%s",HboardName, VboardName), 2560, -128, 128, 2560, -128, 128);

        Pos2D[i]->GetXaxis()->SetTitle(Form("%s position [mm]", HboardName));
        Pos2D[i]->GetYaxis()->SetTitle(Form("%s position [mm]", VboardName));
    }
}