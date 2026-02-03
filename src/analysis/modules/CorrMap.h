// CorrMap.h
// generate correlation maps between channels for each boards
// focus on channel-to-channel correlations
// for noise? is it possible also for signal?

#pragma once

#include "../AnalysisPipeline.h"

class CorrMap final : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "CorrMap";
    }

    void begin_run(const RunContext &ctx) override
    {
        file_ = ctx.rootfile;
        if (!file_)
            return;
        dir_ = file_->GetDirectory("CorrMap");
        if (!dir_)
            dir_ = file_->mkdir("CorrMap");
        dir_->cd();

        is_noise_run = ctx.noise_run;

        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }
        for (int i = 0; i < nrBoards; i++)
        {
            create2dHistograms(ctx, i);
        }
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        if (tags.BKG_SUB_ON && (tags.SpillID == -2 || is_noise_run) && frame_index < 20000)  //analyse at most 20000 frames
        {
            for (int i = 0; i < nrBoards; i++)
            {
                fillHistograms(frame.boards[i].data, false, i);
            }
        }
        else if (frame_index == 20000) // save the plot and free RAM
        {
            for (int i = 0; i < nrBoards; i++)
            {
                get_corr_map(false, i);  //get correlation map and write to .root
            }
            file_saved = true;
        }
    }

    void end_run(const RunContext &ctx) override
    {
        if (!file_saved)
        {
            for (int i = 0; i < nrBoards; i++)
            {
                get_corr_map(false, i);
            }
        }
    }

private:
    int nrBoards = 6;
    TDirectory *dir_ = nullptr;
    TFile *file_ = nullptr;

    bool is_noise_run = false;

    TH2D *channel_corr[6][320][320];
    TH2D *channel_corr_map[6];
    TH1D *channel_corr_1d[6];

    bool file_saved = false;

    void fillHistograms(double *data, bool after_cc, int board_id)
    {
        if (after_cc)
        {
            double cc = 0;
            for (int i = 0; i < 4; i++)
            {
                cc += data[i];
            }
            cc /= 4;
            for (int i = 0; i < 320; i++)
            {
                for (int j = 0; j < 320; j++)
                {
                    if (i >= j)
                        channel_corr[board_id][i][j]->Fill(data[i] - cc, data[j] - cc);
                }
            }
        }
        else
        {
            for (int i = 0; i < 320; i++)
            {
                for (int j = 0; j < 320; j++)
                {
                    if (i >= j)
                        channel_corr[board_id][i][j]->Fill(data[i], data[j]);
                }
            }
        }
    }

    void get_corr_map(bool after_cc, int board_id)
    {
        file_->cd();
        int startchannel = 0;
        if (after_cc)
        {
            startchannel = 4;
        }
        else
        {
            startchannel = 0;
        }
        for (int i = startchannel; i < 320; i++)
        {
            for (int j = startchannel; j < 320; j++)
            {
                if (i >= j)
                {
                    channel_corr_1d[board_id]->Fill(channel_corr[board_id][i][j]->GetCorrelationFactor());
                    channel_corr_map[board_id]->SetBinContent(i + 1, j + 1, channel_corr[board_id][i][j]->GetCorrelationFactor());
                }
                else
                {
                    channel_corr_map[board_id]->SetBinContent(i + 1, j + 1, channel_corr[board_id][j][i]->GetCorrelationFactor());
                }
            }
        }
        dir_->cd();
        channel_corr_map[board_id]->Write();
        // normalize the histogram
        channel_corr_map[board_id]->Scale(1.0 / channel_corr_map[board_id]->Integral());
        channel_corr_1d[board_id]->Write();

        // clear channel
        for (int i = 0; i < 320; i++)
        {
            for (int j = 0; j < 320; j++)
            {
                if (i >= j)
                    channel_corr[board_id][i][j]->Reset();
            }
        }

        //set 2D map Z-axis range -1 to 1
        channel_corr_map[board_id]->GetZaxis()->SetRangeUser(-1, 1);
    };

    void create2dHistograms(const RunContext &ctx, int board_id)
    {

        for (int i = 0; i < 320; i++)
        {
            for (int j = 0; j < 320; j++)
            {
                channel_corr[board_id][i][j] = new TH2D(Form("channel_corr_board%d_%d_%d", board_id, i, j), Form("channel_corr_board%d_%d_%d", board_id, i, j), 201, -100.5, 100.5, 201, -100.5, 100.5); // for noise 
            }
        }
        channel_corr_map[board_id] = new TH2D(Form("channel_corr_map_board%d", board_id), Form("channel_corr_map_%d", board_id), 320, -0.5, 319.5, 320, -0.5, 319.5);
        channel_corr_map[board_id]->GetXaxis()->SetTitle("Channel ID");
        channel_corr_map[board_id]->GetYaxis()->SetTitle("Channel ID");
        channel_corr_map[board_id]->GetZaxis()->SetTitle("Correlation Coefficient");
        channel_corr_1d[board_id] = new TH1D(Form("channel_corr_1d_board%d", board_id), Form("channel_corr_1d_%d", board_id), 1000, -1, 1);
        channel_corr_1d[board_id]->GetXaxis()->SetTitle("Correlation Coefficient");
        channel_corr_1d[board_id]->GetYaxis()->SetTitle("Fraction");
    }
};
