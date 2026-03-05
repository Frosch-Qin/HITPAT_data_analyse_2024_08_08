// PosAlign.h

#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class PosAlign : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "PosAlign";
    }

protected:
    void on_begin_run(const RunContext &ctx) override;

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        // Position alignment logic goes here
        for (int i = 0; i < nrBoards; ++i)
        {
            if (tags.boardTags[i].Cluster_num < 1)
            {
                continue; // Skip processing if no cluster is found
            }
            tags.boardTags[i].Position = tags.boardTags[i].Position * slope_scaled[i] + intercept_scaled[i] - offset_origin;
        }
    }

    void end_run(const RunContext &ctx) override
    {
    }

private:
    double intercept_scaled[6] = {0, 0, 0, 0, 0, 0};
    double slope_scaled[6] = {1, 1, 1, 1, 1, 1};
    const double offset_origin = 128.4; // 128.4 mm set the origin of mm coordinate in the middle of the 320 channels
    void readIn_align(const RunContext &ctx);
};

void PosAlign::on_begin_run(const RunContext &ctx)
{

    readIn_align(ctx);
}

void PosAlign::readIn_align(const RunContext &ctx)
{
    TFile *file = TFile::Open(Form("output/%s_Pos1D.root", ctx.ALIGN_runname), "READ");
    if (!file)
    {
        std::cout << Form("output/%s_Pos1D.root does not exist ", ctx.ALIGN_runname) << std::endl;
    }

    TF1 *H_Fit[3]{};
    TF1 *V_Fit[3]{};
    for (int i = 0; i < nrBoards / 2; i++)
    {
        H_Fit[i] = (TF1 *)file->Get(Form("Pos1D/H%d_Fit", i));
        V_Fit[i] = (TF1 *)file->Get(Form("Pos1D/V%d_Fit", i));

        if (!H_Fit[i])
        {
            std::cout << " can not find the TF1 fit " << std::endl;
            return;
        }

        if (i != 1) // H1 is the reference
        {
            intercept_scaled[ctx.H_boardID[i]] = H_Fit[i]->GetParameter(0);
            slope_scaled[ctx.H_boardID[i]] = H_Fit[i]->GetParameter(1);
        }
        if (i != 1) // H1 is the reference
        {
            intercept_scaled[ctx.V_boardID[i]] = V_Fit[i]->GetParameter(0);
            slope_scaled[ctx.V_boardID[i]] = V_Fit[i]->GetParameter(1);
        }
    }

    for (int i = 0; i < 6; i++)
    {
        std::cout << "alignment factor: " << intercept_scaled[i] << " " << slope_scaled[i] << std::endl;
    }
    file->Close();
}