// Pos1D.h

#pragma once
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

#include "../IAnalyzer.h"
#include "../FrameTags.h"
#include "ScanBin.C" // PeakScan + convertPeaksToBinEdges

class Pos1D : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override { return "Pos1D"; }

protected:
    void on_begin_run(const RunContext &ctx) override;
    void process(Fullframe &frame, long frame_index, FrameTags &tags) override;
    void end_run(const RunContext &ctx) override;

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    TH1D *h1_Pos[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    // store the alignment graph
    TGraph *align_graph[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    // store fit results
    TF1 *f1_fit[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    // for storing peak positions
    std::vector<Peak> best_peaks[6];

    void createHistograms(const RunContext &ctx);
    void fillTree(const RunContext &ctx); // save the peak positions in TTree

    void alignBoards(TGraph *&g, TF1 *&f,
                     const std::vector<Peak> &y,
                     const std::vector<Peak> &x,
                     TH1D *href,
                     const char *boardName);

    void findpeaks(std::vector<Peak> &best_peaks,
                   TH1D *h1,
                   const char *canvas_name,
                   int expected_peak_count = 80);
};

inline void Pos1D::on_begin_run(const RunContext &ctx)
{
    file_ = new TFile(Form("output2025/run%d_Pos1D.root", ctx.run_number), "RECREATE");
    createHistograms(ctx);
}

inline void Pos1D::createHistograms(const RunContext &ctx)
{
    if (!file_)
        return;

    dir_ = file_->GetDirectory("Pos1D");
    if (!dir_)
        dir_ = file_->mkdir("Pos1D");

    dir_->cd();

    for (int i = 0; i < nrBoards; ++i)
    {
        h1_Pos[i] = new TH1D(Form("h1_Pos%s", ctx.BoardName[i]),
                             Form("h1_Pos%s", ctx.BoardName[i]),
                             2560, 0, 256);
        h1_Pos[i]->GetXaxis()->SetTitle(Form("%s position [mm]", ctx.BoardName[i]));
    }
}

inline void Pos1D::process(Fullframe &frame, long frame_index, FrameTags &tags)
{
    (void)frame;
    (void)frame_index;

    for (int i = 0; i < nrBoards; ++i)
    {
        if (tags.SpillID >= 0)
        {
            if (tags.boardTags[i].Cluster_num > 0 && h1_Pos[i])
                h1_Pos[i]->Fill(tags.boardTags[i].Position);
        }
    }
}

inline void Pos1D::end_run(const RunContext &ctx)
{
    if (!file_ || !dir_)
        return;

    dir_->cd();

    for (int i = 0; i < nrBoards; ++i)
    {
        if (!h1_Pos[i])
            continue;

        h1_Pos[i]->Write();
        findpeaks(best_peaks[i], h1_Pos[i], Form("h1_Pos_canvas%s", ctx.BoardName[i]));
    }

    fillTree(ctx);

    for (int i = 0; i < nrBoards; ++i)
    {
        int refboardID = (ctx.BoardName[i][0] == 'V') ? ctx.V_boardID[1] : ctx.H_boardID[1];

        alignBoards(align_graph[i],
                    f1_fit[i],
                    best_peaks[i],
                    best_peaks[refboardID],
                    h1_Pos[i],
                    ctx.BoardName[i]);
    }

    file_->Close();
    delete file_;
    file_ = nullptr;
    dir_ = nullptr;
}

inline void Pos1D::findpeaks(std::vector<Peak> &best_peaks,
                             TH1D *h1,
                             const char *canvas_name,
                             int expected_peak_count)
{
    if (!h1)
        return;

    PeakScan(h1, best_peaks, expected_peak_count);

    TCanvas *c = new TCanvas(canvas_name, canvas_name, 800, 600);
    h1->Draw();

    for (const auto &p : best_peaks)
    {
        double x = h1->GetXaxis()->GetBinCenter((int)std::round(p.x));
        TLine *line = new TLine(x, 0, x, p.y);
        line->SetLineColor(kRed);
        line->Draw("same");
    }

    c->Write();
    delete c;
}

inline void Pos1D::fillTree(const RunContext &ctx)
{
    if (!file_ || !dir_)
        return;

    dir_->cd();

    // ---------------- V tree ----------------
    TTree *t_V = new TTree("t_Vpos", "Peak Positions");
    int order_V = 0;
    double V_peakPos[3] = {0.0, 0.0, 0.0};

    for (int i = 0; i < nrBoards / 2; ++i)
    {
        t_V->Branch(ctx.BoardName[ctx.V_boardID[i]],
                    &V_peakPos[i],
                    Form("%s/D", ctx.BoardName[ctx.V_boardID[i]]));
    }
    t_V->Branch("order", &order_V, "order/I");

    size_t minV = best_peaks[ctx.V_boardID[0]].size();
    for (int i = 1; i < nrBoards / 2; ++i)
        minV = std::min(minV, best_peaks[ctx.V_boardID[i]].size());

    for (size_t j = 0; j < minV; ++j)
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            const int boardID = ctx.V_boardID[i];
            const int binV = (int)std::lround(best_peaks[boardID][j].x);
            V_peakPos[i] = h1_Pos[boardID]->GetXaxis()->GetBinCenter(binV);
        }
        order_V = (int)j;
        t_V->Fill();
    }
    t_V->Write();

    // ---------------- H tree ----------------
    TTree *t_H = new TTree("t_Hpos", "Peak Positions");
    int order_H = 0;
    double H_peakPos[3] = {0.0, 0.0, 0.0};

    for (int i = 0; i < nrBoards / 2; ++i)
    {
        t_H->Branch(ctx.BoardName[ctx.H_boardID[i]],
                    &H_peakPos[i],
                    Form("%s/D", ctx.BoardName[ctx.H_boardID[i]]));
    }
    t_H->Branch("order", &order_H, "order/I");

    size_t minH = best_peaks[ctx.H_boardID[0]].size();
    for (int i = 1; i < nrBoards / 2; ++i)
        minH = std::min(minH, best_peaks[ctx.H_boardID[i]].size());

    for (size_t j = 0; j < minH; ++j)
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            const int boardID = ctx.H_boardID[i];
            const int binH = (int)std::lround(best_peaks[boardID][j].x);
            H_peakPos[i] = h1_Pos[boardID]->GetXaxis()->GetBinCenter(binH);
        }
        order_H = (int)j;
        t_H->Fill();
    }
    t_H->Write();
}

inline void Pos1D::alignBoards(TGraph *&g, TF1 *&fitL,
                               const std::vector<Peak> &y,
                               const std::vector<Peak> &x,
                               TH1D *href,
                               const char *boardName)
{
    if (!file_ || !dir_)
        return;

    const size_t n = std::min(y.size(), x.size());
    if (n < 2)
    {
        std::cout << "alignBoards: not enough points for " << boardName
                  << " (y=" << y.size() << ", x=" << x.size() << ")\n";
        return;
    }

    if (y.size() != x.size())
    {
        std::cout << "alignBoards WARNING: size mismatch for " << boardName
                  << " (y=" << y.size() << ", x=" << x.size()
                  << "), using n=" << n << "\n";
    }

    std::vector<double> posy;
    std::vector<double> posx;
    posy.reserve(n);
    posx.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
        const int biny = (int)std::lround(y[i].x);
        const int binx = (int)std::lround(x[i].x);

        const double yy = href ? href->GetXaxis()->GetBinCenter(biny) : y[i].x;
        const double xx = href ? href->GetXaxis()->GetBinCenter(binx) : x[i].x;

        posy.push_back(yy);
        posx.push_back(xx);
    }

    dir_->cd();

    g = new TGraph((int)n, posy.data(), posx.data());
    g->SetName(Form("%s_graph", boardName));
    g->SetTitle(Form("%s alignment;%s(ref);%s", boardName, boardName, boardName));

    fitL = new TF1(Form("%s_Fit", boardName), "pol1");
    g->Fit(fitL, "Q");

    TCanvas *c = new TCanvas(Form("%s_align_c", boardName), boardName, 700, 500);
    g->Draw("AP");
    fitL->Draw("SAME");
    c->Write();

    g->Write("", TObject::kOverwrite);
    fitL->Write("", TObject::kOverwrite);

    delete c;
}