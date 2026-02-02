// NoiseAnalyzer.h
#pragma once

#include "../AnalysisPipeline.h"

class NoiseAnalyzer final : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "NoiseAnalyzer";
    }

    void begin_run(const RunContext &ctx) override
    {
        if (!ctx.rootfile)
            return;
        dir_ = ctx.rootfile->GetDirectory("NoiseAnalysis");
        if (!dir_)
            dir_ = ctx.rootfile->mkdir("NoiseAnalysis");
        dir_->cd();

        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }
        // TH1D for nrBoards
        for (int i = 0; i < nrBoards; ++i)
        {
            noise_total_hist[i] = new TH1D(Form("h_noise_board_%d", i), Form("Noise Histogram Board %zu;ADC Value;Counts", i), 200, -50, 50);
        }

        is_noise_run = ctx.noise_run;
        if (is_noise_run)
        {
          std::cout<< "this is a noise run " << std::endl;
        }
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {

        // if is noise only, fill the hist
        if (tags.BKG_SUB_ON)
        {
            for (int i = 0; i < nrBoards; ++i)
            {
                if (is_noise_run)
                {
                    for (int j = 0; j < frame.boards[i].nrChannels; ++j)
                    {
                        noise_total_hist[i]->Fill(frame.boards[i].data[j]);
                    }
                }
                else if (tags.SpillID == -2)
                {
                    for (int j = 0; j < frame.boards[i].nrChannels; ++j)
                    {

                        // if (j == 0)
                        //     std::cout << "Board " << i << " Channel " << j << " Value " << frame.boards[i].data[j] << std::endl;
                        noise_total_hist[i]->Fill(frame.boards[i].data[j]);
                    }

                    // std::cout<< "filling noise from group A frame " << std::endl;
                }
            }
        }
    }
    // ~NoiseAnalyzer()
    // {
    //     for (auto hist : noise_total_hist)
    //     {
    //         delete hist;
    //     }
    // } //should not deconstruct the hist, because rootfile needs to save them later

    void end_run(const RunContext &ctx) override
    {
        ctx.rootfile->cd();
        if (!dir_)
            return;
        dir_->cd();
        for (int i = 0; i < nrBoards; ++i)
            if (noise_total_hist[i])
                noise_total_hist[i]->Write();
    }

private:
    int nrBoards = 6;
    TH1D *noise_total_hist[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}; // prepare 6 histograms
    TDirectory *dir_ = nullptr;
    bool is_noise_run = false;
};
