// Pos1D.h

#pragma once
#include <cmath>
#include <iostream>
#include <vector>

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

    TH1D *H_Pos[3] = {nullptr, nullptr, nullptr};
    TH1D *V_Pos[3] = {nullptr, nullptr, nullptr};

    //store the alignment graph
    TGraph *H_align_graph[3]{};
    TGraph *V_align_graph[3]{};

    //store fit results
    TF1 *H_fit[3]{};
    TF1 *V_fit[3]{};

    // for storing peak positions
    std::vector<Peak> H_best_peaks[3];
    std::vector<Peak> V_best_peaks[3];

    void createHistograms(const RunContext &ctx);

    void fillTree(); // save the peak positions in TTree

    void alignBoards(TGraph *g, TF1 *f, const std::vector<Peak> &y, const std::vector<Peak> &x, TH1D *href, const char *boardName); // calculate for example V0:V1, get the the fit function

    void findpeaks(std::vector<Peak> &best_peaks, TH1D *h1, const char *canvas_name, int expected_peak_count = 80);
};

inline void Pos1D::on_begin_run(const RunContext &ctx)
{
    file_ = new TFile(Form("output/run%d_Pos1D.root", ctx.run_number), "RECREATE");
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

    for (int i = 0; i < nrBoards / 2; ++i)
    {
        H_Pos[i] = new TH1D(Form("H_Pos%d", i), Form("H_Pos%d", i), 2560, 0, 256);
        H_Pos[i]->GetXaxis()->SetTitle(Form("H%d position [mm]", i));
        V_Pos[i] = new TH1D(Form("V_Pos%d", i), Form("V_Pos%d", i), 2560, 0, 256);
        V_Pos[i]->GetXaxis()->SetTitle(Form("H%d position [mm]", i));
    }
}

inline void Pos1D::process(Fullframe &frame, long frame_index, FrameTags &tags)
{
    for (int i = 0; i < nrBoards / 2; ++i)
    {
        if (tags.SpillID >= 0) // if it is during a spill
        {
            if (tags.boardTags[H_boardID[i]].Cluster_num > 0)
                H_Pos[i]->Fill(tags.boardTags[H_boardID[i]].Position);
            if (tags.boardTags[V_boardID[i]].Cluster_num > 0)
                V_Pos[i]->Fill(tags.boardTags[V_boardID[i]].Position);
        }
    }
}

inline void Pos1D::end_run(const RunContext &ctx)
{

    if (file_)
    {
        dir_->cd();
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            V_Pos[i]->Write();
            H_Pos[i]->Write();
            findpeaks(V_best_peaks[i], V_Pos[i], Form("V_Pos_canvas%d", i));
            findpeaks(H_best_peaks[i], H_Pos[i], Form("H_Pos_canvas%d", i));
        }
        fillTree(); // fill the peak positions in TTree and save it
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            alignBoards(V_align_graph[i], V_fit[i], V_best_peaks[i], V_best_peaks[1], V_Pos[i], Form("V%d", i));
            alignBoards(H_align_graph[i], H_fit[i], H_best_peaks[i], H_best_peaks[1], H_Pos[i], Form("H%d", i)); // the middle boards as reference
        }
        file_->Close();
    }
}

inline void Pos1D::findpeaks(std::vector<Peak> &best_peaks, TH1D *h1, const char *canvas_name, int expected_peak_count)
{

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
}

inline void Pos1D::fillTree()
{
    if (!file_ || !dir_)
        return;

    TTree *t_V = new TTree("t_Vpos", "Peak Positions");
    int order_V = 0;
    double V_peakPos[3]{};

    // check if the all V boards have the same peak number
    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        if (V_best_peaks[i].size() != V_best_peaks[i + 1].size())
        {
            std::cout << "V board " << i << " and " << i + 1 << " have different number of peaks" << std::endl;
            return;
        }
    }

    t_V->Branch("V0", &V_peakPos[0], "V0/D");
    t_V->Branch("V1", &V_peakPos[1], "V1/D");
    t_V->Branch("V2", &V_peakPos[2], "V2/D");
    t_V->Branch("order", &order_V, "order/I");

    for (int j = 0; j < V_best_peaks[0].size(); j++)
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            const int binV = (int)std::lround(V_best_peaks[i][j].x);
            V_peakPos[i] = H_Pos[0]->GetXaxis()->GetBinCenter(binV);
        }
        order_V = j;
        t_V->Fill();
    }
    t_V->Write();
    // For horizontal planes
    TTree *t_H = new TTree("t_Hpos", "Peak Positions");
    int order_H = 0;
    double H_peakPos[3]{};

    // check if the all H boards have the same peak number
    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        if (H_best_peaks[i].size() != H_best_peaks[i + 1].size())
        {
            std::cout << "H board " << i << " and " << i + 1 << " have different number of peaks" << std::endl;
            return;
        }
    }

    t_H->Branch("H0", &H_peakPos[0], "H0/D");
    t_H->Branch("H1", &H_peakPos[1], "H1/D");
    t_H->Branch("H2", &H_peakPos[2], "H2/D");
    t_H->Branch("order", &order_H, "order/I");

    for (int j = 0; j < H_best_peaks[0].size(); j++)
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            const int binH = (int)std::lround(H_best_peaks[i][j].x);
            H_peakPos[i] = H_Pos[0]->GetXaxis()->GetBinCenter(binH);
        }
        order_H = j;
        t_H->Fill();
    }
    t_H->Write();
}

inline void Pos1D::alignBoards(TGraph *g, TF1 *fitL, const std::vector<Peak> &y,
                               const std::vector<Peak> &x,
                               TH1D *href, // reference histogram for axis conversion
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
    posy.reserve(n);
    std::vector<double> posx;
    posx.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
        // If Peak.x is bin index, convert to axis coordinate:
        const int biny = (int)std::lround(y[i].x);
        const int binx = (int)std::lround(x[i].x);

        const double yy = href ? href->GetXaxis()->GetBinCenter(biny) : y[i].x;
        const double xx = href ? href->GetXaxis()->GetBinCenter(binx) : x[i].x;

        posy.push_back(yy);
        posx.push_back(xx);
    }

    // Ensure we write into a known directory
    dir_->cd();

    g = new TGraph((int)n, posy.data(), posx.data()); // later p1*y + p0  align to the reference board
    g->SetName(Form("%s_graph", boardName));
    g->SetTitle(Form("%s alignment;%s(ref);%s", boardName, boardName, boardName));

    fitL = new TF1(Form("%s_Fit", boardName), "pol1");
    g->Fit(fitL, "Q"); // pol1: p0 + p1*x

    // Optional: save a canvas too
    TCanvas *c = new TCanvas(Form("%s_align_c", boardName), boardName, 700, 500);
    g->Draw("AP");
    fitL->Draw("SAME");
    c->Write();

    g->Write("", TObject::kOverwrite);
    fitL->Write("", TObject::kOverwrite);

    // Optional cleanup
    // delete c; delete g; delete fitL;
}