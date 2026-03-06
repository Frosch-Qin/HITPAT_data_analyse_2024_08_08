// Pos2DMap.h  (patched)

#pragma once

#include <cmath>
#include <iostream>
#include <vector>

#include "../IAnalyzer.h"
#include "../FrameTags.h"
#include "ScanBin.C" // PeakScan + convertPeaksToBinEdges

class Pos2DMap : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override { return "Pos2DMap"; }

protected:
    void on_begin_run(const RunContext &ctx) override
    {
        file_ = new TFile(Form("output2025/run%d_Pos2DMap.root", ctx.run_number), "RECREATE");

        histReady_ = false;
        if (!getBinEdges(ctx))
        {
            std::cerr << "Pos2DMap: getBinEdges failed -> skip creating histograms for this run.\n";
            return;
        }

        createHistograms(ctx);
        histReady_ = true;
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override;
    void end_run(const RunContext &ctx) override;

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    bool histReady_ = false;

    int nXbins = 0;
    int nYbins = 0;
    const int nZbins = 1000;
    const double Zbin_leftedge = -10;
    const double Zbin_rightedge = 10;
    std::vector<double> HbinEdges;
    std::vector<double> VbinEdges;
    std::vector<double> ZbinEdges;

    const double offset_origin = 128.4; // 128.4 mm set the origin of mm coordinate in the middle of the 320 channels

    TH2D *V_PosBias[2] = {nullptr, nullptr};
    TH2D *H_PosBias[2] = {nullptr, nullptr};

    TH2D *V_PosRes[2] = {nullptr, nullptr};
    TH2D *H_PosRes[2] = {nullptr, nullptr};

    TH3D *V_3D[2] = {nullptr, nullptr};
    TH3D *H_3D[2] = {nullptr, nullptr};

    static bool edgesStrictlyIncreasing(const std::vector<double> &e)
    {
        if (e.size() < 2)
            return false;
        for (size_t i = 0; i + 1 < e.size(); ++i)
            if (!(e[i] < e[i + 1]))
                return false;
        return true;
    }

    void createHistograms(const RunContext &ctx);
    bool getBinEdges(const RunContext &ctx);

    void convertTH3DtoMeanSigma(const TH3D *h3, TH2D *hMean, TH2D *hSigma)
    {
        int nx = h3->GetNbinsX();
        int ny = h3->GetNbinsY();

        for (int ix = 1; ix <= nx; ++ix)
        {
            for (int iy = 1; iy <= ny; ++iy)
            {
                TH1D *hz = h3->ProjectionZ("hz_tmp", ix, ix, iy, iy);

                if (hz->GetEntries() > 10)
                {
                    hz->Fit("gaus", "Q");
                    TF1 *f = hz->GetFunction("gaus");

                    if (f)
                    {
                        hMean->SetBinContent(ix, iy, f->GetParameter(1));
                        hSigma->SetBinContent(ix, iy, f->GetParameter(2));
                    }
                }

                delete hz;
            }
        }
    }
};

inline void Pos2DMap::process(Fullframe &frame, long frame_index, FrameTags &tags)
{

    if (!histReady_)
        return;

    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        if (tags.SpillID < 0)
            continue;

        if (!H_3D[i] || !V_3D[i])
            continue;

        double x = tags.boardTags[H_boardID[1]].Position;
        double y = tags.boardTags[V_boardID[1]].Position;
        double Hdiff = tags.boardTags[H_boardID[i + 1]].Position - tags.boardTags[H_boardID[i]].Position;
        double Vdiff = tags.boardTags[V_boardID[i + 1]].Position - tags.boardTags[V_boardID[i]].Position;

        H_3D[i]->Fill(x, y, Hdiff);
        V_3D[i]->Fill(x, y, Vdiff);
    }
}

inline void Pos2DMap::createHistograms(const RunContext &ctx)
{
    if (!file_)
        return;

    dir_ = file_->GetDirectory("Pos2DMap");
    if (!dir_)
        dir_ = file_->mkdir("Pos2DMap");
    dir_->cd();

    ZbinEdges.reserve(nZbins + 1);
    for (int i = 0; i <= nZbins; i++)
    {
        ZbinEdges.push_back(Zbin_leftedge + (Zbin_rightedge - Zbin_leftedge) / nZbins * i);
    }
    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        V_3D[i] = new TH3D(Form("V_3D_V%dV%d", i, i + 1),
                           Form("V_3D_V%dV%d", i, i + 1),
                           nXbins, HbinEdges.data(),
                           nYbins, VbinEdges.data(), nZbins, ZbinEdges.data());
        H_3D[i] = new TH3D(Form("H_3D_H%dH%d", i, i + 1),
                           Form("H_3D_H%dH%d", i, i + 1),
                           nXbins, HbinEdges.data(),
                           nYbins, VbinEdges.data(), nZbins, ZbinEdges.data());

        V_PosBias[i] = new TH2D(Form("V_PosBias_V%dV%d", i, i + 1),
                                Form("V_PosBias_V%dV%d", i, i + 1),
                                nXbins, HbinEdges.data(),
                                nYbins, VbinEdges.data());

        H_PosBias[i] = new TH2D(Form("H_PosBias_H%dH%d", i, i + 1),
                                Form("H_PosBias_H%dH%d", i, i + 1),
                                nXbins, HbinEdges.data(),
                                nYbins, VbinEdges.data());

        V_PosRes[i] = new TH2D(Form("V_PosRes_V%dV%d", i, i + 1),
                               Form("V_PosRes_V%dV%d", i, i + 1),
                               nXbins, HbinEdges.data(),
                               nYbins, VbinEdges.data());

        H_PosRes[i] = new TH2D(Form("H_PosRes_H%dH%d", i, i + 1),
                               Form("H_PosRes_H%dH%d", i, i + 1),
                               nXbins, HbinEdges.data(),
                               nYbins, VbinEdges.data());

        H_PosBias[i]->GetXaxis()->SetTitle("H1 position [mm]");
        H_PosBias[i]->GetYaxis()->SetTitle("V1 position [mm]");
        V_PosBias[i]->GetXaxis()->SetTitle("H1 position [mm]");
        V_PosBias[i]->GetYaxis()->SetTitle("V1 position [mm]");

        H_PosRes[i]->GetXaxis()->SetTitle("H1 position [mm]");
        H_PosRes[i]->GetYaxis()->SetTitle("V1 position [mm]");
        V_PosRes[i]->GetXaxis()->SetTitle("H1 position [mm]");
        V_PosRes[i]->GetYaxis()->SetTitle("V1 position [mm]");
    }
}

inline void Pos2DMap::end_run(const RunContext &ctx)
{
    (void)ctx;

    if (!file_ || !dir_ || !histReady_)
        return;

    dir_->cd();

    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {

        convertTH3DtoMeanSigma(V_3D[i], V_PosBias[i], V_PosRes[i]);
        convertTH3DtoMeanSigma(H_3D[i], H_PosBias[i], H_PosRes[i]);
        H_PosBias[i]->Write();
        H_PosRes[i]->Write();
        V_PosBias[i]->Write();
        V_PosRes[i]->Write();

        // ----- Draw canvases -----

        // H bias
        TCanvas *c_Hbias = new TCanvas(Form("c_Hbias_%d", i), "", 800, 600);
        H_PosBias[i]->GetZaxis()->SetRangeUser(-1, 1);
        H_PosBias[i]->Draw("colz");
        c_Hbias->Write();

        // H resolution
        TCanvas *c_Hres = new TCanvas(Form("c_Hres_%d", i), "", 800, 600);
        H_PosRes[i]->GetZaxis()->SetRangeUser(0, 0.2);
        H_PosRes[i]->Draw("colz");
        c_Hres->Write();

        // V bias
        TCanvas *c_Vbias = new TCanvas(Form("c_Vbias_%d", i), "", 800, 600);
        V_PosBias[i]->GetZaxis()->SetRangeUser(-1, 1);
        V_PosBias[i]->Draw("colz");
        c_Vbias->Write();

        // V resolution
        TCanvas *c_Vres = new TCanvas(Form("c_Vres_%d", i), "", 800, 600);
        V_PosRes[i]->GetZaxis()->SetRangeUser(0, 1);
        V_PosRes[i]->Draw("colz");
        c_Vres->Write();

        delete c_Hbias;
        delete c_Hres;
        delete c_Vbias;
        delete c_Vres;
    }

    file_->Close();
}

inline bool Pos2DMap::getBinEdges(const RunContext &ctx) // from the TTree in Pos1D.root
{
    TFile *binFile = TFile::Open(Form("output2025/run%d_Pos1D.root", ctx.run_number), "READ");
    if (!binFile || binFile->IsZombie())
    {
        std::cerr << "Error: cannot open file to get bin edges.\n";
        return false;
    }

    TTree *t_H = (TTree *)binFile->Get("Pos1D/t_Hpos");
    TTree *t_V = (TTree *)binFile->Get("Pos1D/t_Vpos");

    std::vector<double> peaksH;
    std::vector<double> peaksV;
    peaksH.reserve(t_H->GetEntries());
    peaksV.reserve(t_V->GetEntries());

    double peak_H1;
    double peak_V1;

    t_H->SetBranchAddress("H1", &peak_H1);
    t_V->SetBranchAddress("V1", &peak_V1);

    for (int i = 0; i < t_H->GetEntries(); i++)
    {
        t_H->GetEntry(i);
        peaksH.push_back(peak_H1 - offset_origin);
    }
    for (int i = 0; i < t_V->GetEntries(); i++)
    {
        t_V->GetEntry(i);
        peaksV.push_back(peak_V1 - offset_origin);
    }

    convertPeaksToBinEdges(peaksH, HbinEdges);
    convertPeaksToBinEdges(peaksV, VbinEdges);

    nXbins = (int)HbinEdges.size() - 1;
    nYbins = (int)VbinEdges.size() - 1;

    if (nXbins <= 0 || nYbins <= 0)
    {
        std::cerr << "FATAL: invalid nbins (nXbins=" << nXbins << ", nYbins=" << nYbins << ")\n";
        binFile->Close();
        return false;
    }

    if (!edgesStrictlyIncreasing(HbinEdges))
    {
        std::cerr << "FATAL: HbinEdges not strictly increasing\n";
        binFile->Close();
        return false;
    }
    if (!edgesStrictlyIncreasing(VbinEdges))
    {
        std::cerr << "FATAL: VbinEdges not strictly increasing\n";
        binFile->Close();
        return false;
    }

    binFile->Close();
    return true;
}
