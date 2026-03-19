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
        (void)frame;
        (void)frame_index;

        for (int i = 0; i < nrBoards; ++i)
        {
            if (tags.boardTags[i].Cluster_num < 1)
                continue;

            tags.boardTags[i].Position =
                tags.boardTags[i].Position * slope_scaled[i] + intercept_scaled[i] - offset_origin;
        }
    }

    void end_run(const RunContext &ctx) override
    {
        (void)ctx;
    }

private:
    double intercept_scaled[6] = {0, 0, 0, 0, 0, 0};
    double slope_scaled[6] = {1, 1, 1, 1, 1, 1};

    const double offset_origin = 128.4; // 128.4 mm set the origin of mm coordinate in the middle of the 320 channels

    void readIn_align(const RunContext &ctx);
};

inline void PosAlign::on_begin_run(const RunContext &ctx)
{
    readIn_align(ctx);
}

inline void PosAlign::readIn_align(const RunContext &ctx)
{
    TFile *file = TFile::Open(Form("output2025/%s_Pos1D.root", ctx.ALIGN_runname), "READ");
    if (!file || file->IsZombie())
    {
        std::cout << Form("output2025/%s_Pos1D.root does not exist", ctx.ALIGN_runname) << std::endl;
        return;
    }

    // reset defaults
    for (int i = 0; i < nrBoards; ++i)
    {
        intercept_scaled[i] = 0.0;
        slope_scaled[i] = 1.0;
    }

    for (int i = 0; i < nrBoards; ++i)
    {
        const char *boardName = ctx.BoardName[i];

        TF1 *fit = (TF1 *)file->Get(Form("Pos1D/%s_Fit", boardName));
        if (!fit)
        {
            std::cout << "Cannot find TF1 fit for " << boardName << std::endl;
            continue;
        }

        intercept_scaled[i] = fit->GetParameter(0);
        slope_scaled[i] = fit->GetParameter(1);
    }

    for (int i = 0; i < nrBoards; ++i)
    {
        std::cout << ctx.BoardName[i]
                  << " alignment factor: "
                  << intercept_scaled[i] << " "
                  << slope_scaled[i] << std::endl;
    }

    file->Close();
    delete file;
}