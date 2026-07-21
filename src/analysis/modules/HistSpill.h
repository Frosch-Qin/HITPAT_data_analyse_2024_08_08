// HistSpill.h

#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class HistSpill : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override { return "HistSpill"; }

protected:
    void on_begin_run(RunContext &ctx) override;
    void process(Fullframe &frame, long frame_index, FrameTags &tags) override;
    void end_run(const RunContext &ctx) override;

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    std::vector<double> total_noise;
    std::vector<double> uncommon_noise;
    std::vector<double> common_noise;

    std::vector<TH2D *> PosDiffHH;
    std::vector<TH2D *> PosDiffVV;
    std::vector<TH2D *> PosDiffHH_time;
    std::vector<TH2D *> PosDiffVV_time;

    std::vector<TH2D *> Pos_time_cut;
    std::vector<TH2D *> Pos_time;

    std::vector<TH2D *> Snr_SpillID;
    std::vector<TH2D *> Snr_time;

    void createHistograms(const RunContext &ctx);
    void writeHistograms(const RunContext &ctx);

    void readinNoise(const RunContext &ctx);

    void draw_colz_with_minz(TH2D *h, double minz)
    {
        if (!h)
            return;

        TCanvas *c = new TCanvas(Form("%s_c", h->GetName()),
                                 Form("%s_c", h->GetName()),
                                 800, 600);

        h->SetMinimum(minz);
        h->SetMaximum(h->GetMaximum());
        h->Draw("colz");
        file_->cd();
        dir_->cd();
        c->Write();
    }
};
inline void HistSpill::on_begin_run(RunContext &ctx)
{
    file_ = new TFile(Form("output2025/run%d_%s.root", ctx.run_number, name().c_str()), "RECREATE");
    createHistograms(ctx);
    readinNoise(ctx);
}

inline void HistSpill::createHistograms(const RunContext &ctx)
{
    if (!file_)
        return;

    dir_ = file_->GetDirectory(name().c_str());

    if (!dir_)
        dir_ = file_->mkdir(name().c_str());
    dir_->cd();

    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        const char *hA = ctx.BoardName[H_boardID[i]];
        const char *hB = ctx.BoardName[H_boardID[i + 1]];
        const char *vA = ctx.BoardName[V_boardID[i]];
        const char *vB = ctx.BoardName[V_boardID[i + 1]];

        PosDiffHH.push_back(new TH2D(Form("PosDiffHH_%s_%s", hA, hB), Form("pos_diff hist %s_%s", hA, hB), ctx.spillNum + 1, -1.5, ctx.spillNum - 0.5, 1000, -4, 4));
        PosDiffHH[i]->GetXaxis()->SetTitle("Spill ID");
        PosDiffHH[i]->GetYaxis()->SetTitle(Form("pos difference %s - %s [mm] / #sqrt{2}", hB, hA));

        PosDiffVV.push_back(new TH2D(Form("PosDiffVV_%s_%s", vA, vB), Form("pos_diff hist %s_%s", vA, vB), ctx.spillNum + 1, -1.5, ctx.spillNum - 0.5, 1000, -4, 4));
        PosDiffVV[i]->GetXaxis()->SetTitle("Spill ID");
        PosDiffVV[i]->GetYaxis()->SetTitle(Form("pos difference %s - %s [mm] / #sqrt{2}", vB, vA));

        double time_binsize = 0.1; // unit second
        int time_binnumber = int(ctx.max_frames / ctx.readout_rate / time_binsize);

        PosDiffHH_time.push_back(new TH2D(Form("PosDiffHH_time_%s_%s", hA, hB), Form("pos_diff hist %s_%s", hA, hB), time_binnumber, 0 - time_binsize / 2, time_binnumber * time_binsize - time_binsize / 2, 1000, -4, 4));
        PosDiffHH_time[i]->GetXaxis()->SetTitle("Time [s]");
        PosDiffHH_time[i]->GetYaxis()->SetTitle(Form("pos difference %s - %s [mm] / #sqrt{2}", hB, hA));

        PosDiffVV_time.push_back(new TH2D(Form("PosDiffVV_time_%s_%s", vA, vB), Form("pos_diff hist %s_%s", vA, vB), time_binnumber, 0 - time_binsize / 2, time_binnumber * time_binsize - time_binsize / 2, 1000, -4, 4));
        PosDiffVV_time[i]->GetXaxis()->SetTitle("Time [s]");
        PosDiffVV_time[i]->GetYaxis()->SetTitle(Form("pos difference %s - %s [mm] / #sqrt{2}", vB, vA));
    }

    // for each board
    for (int i = 0; i < nrBoards; ++i)
    {
        const char *boardName = ctx.BoardName[i];
        double time_binsize = 0.1; // unit second
        int time_binnumber = int(ctx.max_frames / ctx.readout_rate / time_binsize);
        Pos_time_cut.push_back(new TH2D(Form("Pos_time_cut_%s", boardName), Form("pos_time_cut hist %s", boardName), time_binnumber, 0 - time_binsize / 2, time_binnumber * time_binsize - time_binsize / 2, 150, -150, 150)); // unit mm
        Pos_time.push_back(new TH2D(Form("Pos_time_%s", boardName), Form("pos_time hist %s", boardName), time_binnumber, 0 - time_binsize / 2, time_binnumber * time_binsize - time_binsize / 2, 30000, -150, 150));           // unit mm

        Snr_SpillID.push_back(new TH2D(Form("Snr_SpillID_%s", boardName), Form("Snr_SpillID hist %s", boardName), ctx.spillNum + 1, -1.5, ctx.spillNum - 0.5, 1000, 0, 1000));
        Snr_time.push_back(new TH2D(Form("Snr_time_%s", boardName), Form("Snr_time hist %s", boardName), time_binnumber, 0 - time_binsize / 2, time_binnumber * time_binsize - time_binsize / 2, 1000, 0, 1000));
    }
}

inline void HistSpill::process(Fullframe &frame, long frame_index, FrameTags &tags)
{
    (void)frame;
    (void)frame_index;

    if (tags.SpillID < 0)
        return;

    if (!tags.Has_signal)
    {
        // std::cout << "No signal for tags: " << tags.SpillID << std::endl;
        return;
    }

    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        if (!PosDiffHH[i] || !PosDiffVV[i])
        {
            std::cout << "Histogram not initialized for board pair: PosDiffHH[" << i << "] || !PosDiffVV[" << i << "] " << i << std::endl;
            continue;
        }

        int hAID = H_boardID[i];
        int hBID = H_boardID[i + 1];
        int vAID = V_boardID[i];
        int vBID = V_boardID[i + 1];

        const double Hdiff = (tags.boardTags[hBID].Position - tags.boardTags[hAID].Position) / sqrt(2);
        const double Vdiff = (tags.boardTags[vBID].Position - tags.boardTags[vAID].Position) / sqrt(2);

        PosDiffHH[i]->Fill(tags.SpillID, Hdiff);
        PosDiffHH_time[i]->Fill(tags.time, Hdiff);

        if (abs(Hdiff) > 2.0)
        {
            Pos_time_cut[hBID]->Fill(tags.time, tags.boardTags[hBID].Position);
            Pos_time_cut[hAID]->Fill(tags.time, tags.boardTags[hAID].Position);
        }

        PosDiffVV[i]->Fill(tags.SpillID, Vdiff);
        PosDiffVV_time[i]->Fill(tags.time, Vdiff);
        if (abs(Vdiff) > 2.0)
        {
            Pos_time_cut[vBID]->Fill(tags.time, tags.boardTags[vBID].Position);
            Pos_time_cut[vAID]->Fill(tags.time, tags.boardTags[vAID].Position);
        }
    }

    // per board
    for (int i = 0; i < nrBoards; ++i)
    {
        if (!Pos_time[i])
            continue;

        Pos_time[i]->Fill(tags.time, tags.boardTags[i].Position);
        Snr_SpillID[i]->Fill(tags.SpillID, tags.boardTags[i].Peak / uncommon_noise[i]);
        Snr_time[i]->Fill(tags.time, tags.boardTags[i].Peak / uncommon_noise[i]);
    }
}

inline void HistSpill::end_run(const RunContext &ctx)
{
    if (!file_ || !dir_)
        return;

    dir_->cd();

    for (auto h : PosDiffHH)
    {
        if (h)
        {
            h->Write();
            draw_colz_with_minz(h, 3.0);
        }
    }

    for (auto h : PosDiffVV)
    {
        if (h)
        {
            h->Write();
            draw_colz_with_minz(h, 3.0);
        }
    }

    for (auto h : PosDiffHH_time)
    {
        if (h)
        {
            h->Write();
            draw_colz_with_minz(h, 1.0);
        }
    }

    for (auto h : PosDiffVV_time)
    {
        if (h)
        {
            h->Write();
            draw_colz_with_minz(h, 1.0);
        }
    }

    for (auto h : Pos_time_cut)
    {
        if (h)
        {
            h->Write();
        }
    }

    for (auto h : Pos_time)
    {
        if (h)
        {
            h->Write();
        }
    }

    for (auto h : Snr_SpillID)
    {
        if (h)
        {
            h->Write();
        }
    }

    for (auto h : Snr_time)
    {
        if (h)
        {
            h->Write();
        }
    }

    file_->Close();
    delete file_;
    file_ = nullptr;
    dir_ = nullptr;
}

inline void HistSpill::readinNoise(const RunContext &ctx)
{
    // Read in noise data from the specified run
    std::string noise_file = Form("output2025/Noise/%s_Noise.root", ctx.Noise_runname);
    TFile *file = TFile::Open(noise_file.c_str());
    if (!file || file->IsZombie())
    {
        std::cerr << "Failed to open noise file: " << noise_file << std::endl;
        return;
    }

    total_noise.clear();
    total_noise.reserve(nrBoards);
    uncommon_noise.clear();
    uncommon_noise.reserve(nrBoards);
    common_noise.clear();
    common_noise.reserve(nrBoards);

    for (int i = 0; i < nrBoards; i++)
    {
        const char *boardName = ctx.BoardName[i];
        TH1D *h_total = dynamic_cast<TH1D *>(file->Get(Form("NoiseAnalysis/noise_total_%s", boardName)));
        TH1D *h_uncommon = dynamic_cast<TH1D *>(file->Get(Form("NoiseAnalysis/noise_uncommon_%s", boardName)));
        TH1D *h_common = dynamic_cast<TH1D *>(file->Get(Form("NoiseAnalysis/noise_common_%s", boardName)));

        if (h_total)
        {
            total_noise.push_back(h_total->GetStdDev());
        }
        if (h_uncommon)
        {
            uncommon_noise.push_back(h_uncommon->GetStdDev());
        }
        if (h_common)
        {
            common_noise.push_back(h_common->GetStdDev());
        }
        std::cout << "Board: " << boardName << ", Total Noise: " << total_noise.back()
                  << ", Uncommon Noise: " << uncommon_noise.back()
                  << ", Common Noise: " << common_noise.back() << std::endl;
    }

    file->Close();
}