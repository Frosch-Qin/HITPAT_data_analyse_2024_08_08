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
        file_ = new TFile(Form("output/run%d_Pos2DMap.root", ctx.run_number), "RECREATE");

        histReady_ = false;
        if (!getBinEdges(ctx))
        {
            std::cerr << "Pos2DMap: getBinEdges failed -> skip creating histograms for this run.\n";
            return;
        }

        createHistograms(ctx);
        histReady_ = true;
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        (void)frame;
        (void)frame_index;

        if (!histReady_)
            return;

        for (int i = 0; i < nrBoards / 2; ++i)
        {
            if (tags.SpillID < 0)
                continue;

            if (!Pos2D[i])
                continue;

            Pos2D[i]->Fill(tags.boardTags[H_boardID[i]].Position,
                           tags.boardTags[V_boardID[i]].Position);

            if (i < nrBoards / 2 - 1)
            {
                if (!H_Pos[i] || !V_Pos[i] || !H_Pos2[i] || !V_Pos2[i])
                    continue;

                double x = tags.boardTags[H_boardID[1]].Position;
                double y = tags.boardTags[V_boardID[1]].Position;
                double Hdiff = tags.boardTags[H_boardID[i + 1]].Position - tags.boardTags[H_boardID[i]].Position;
                double Vdiff = tags.boardTags[V_boardID[i + 1]].Position - tags.boardTags[V_boardID[i]].Position;

                H_Pos[i]->Fill(x, y, Hdiff);
                V_Pos[i]->Fill(x, y, Vdiff);
                H_Pos2[i]->Fill(x, y, Hdiff * Hdiff);
                V_Pos2[i]->Fill(x, y, Vdiff * Vdiff);

                H_3D[i]->Fill(x, y, Hdiff);
                V_3D[i]->Fill(x, y, Vdiff);
            }
        }
    }
    void end_run(const RunContext &ctx) override;

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    bool histReady_ = false;

    int nXbins = 0;
    int nYbins = 0;

    std::vector<double> HbinEdges;
    std::vector<double> VbinEdges;
    std::vector<double> Zbins;

    const double offset_origin = 128.4; // 128.4 mm set the origin of mm coordinate in the middle of the 320 channels

    TH2D *Pos2D[3] = {nullptr, nullptr, nullptr};

    TProfile2D *V_Pos[2] = {nullptr, nullptr};
    TProfile2D *H_Pos[2] = {nullptr, nullptr};
    TProfile2D *V_Pos2[2] = {nullptr, nullptr};
    TProfile2D *H_Pos2[2] = {nullptr, nullptr};

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
};

inline void Pos2DMap::createHistograms(const RunContext &ctx)
{
    (void)ctx;

    if (!file_)
        return;

    dir_ = file_->GetDirectory("Pos2DMap");
    if (!dir_)
        dir_ = file_->mkdir("Pos2DMap");
    dir_->cd();

    for (int i = 0; i < nrBoards / 2; ++i)
    {
        Pos2D[i] = new TH2D(Form("Pos2D_H%dV%d", i, i),
                            Form("Pos2D_H%dV%d", i, i),
                            nXbins, HbinEdges.data(),
                            nYbins, VbinEdges.data());

        Pos2D[i]->GetXaxis()->SetTitle(Form("H%d position [mm]", i));
        Pos2D[i]->GetYaxis()->SetTitle(Form("V%d position [mm]", i));
    }

    for (int i = 0; i < 100; i++)
    {
        Zbins.push_back(-1 + 2 / 100 * i);
    }
    for (int i = 0; i < nrBoards / 2 - 1; ++i)
    {
        V_3D[i] = new TH3D(Form("V_3D_V%dV%d", i, i + 1),
                           Form("V_3D_V%dV%d", i, i + 1),
                           nXbins, HbinEdges.data(),
                           nYbins, VbinEdges.data(), 100, Zbins.data());
        H_3D[i] = new TH3D(Form("H_3D_H%dH%d", i, i + 1),
                           Form("H_3D_H%dH%d", i, i + 1),
                           nXbins, HbinEdges.data(),
                           nYbins, VbinEdges.data(), 100, Zbins.data());
        H_Pos[i] = new TProfile2D(Form("H_Pos_H%dH%d", i, i + 1),
                                  Form("H_Pos_H%dH%d", i, i + 1),
                                  nXbins, HbinEdges.data(),
                                  nYbins, VbinEdges.data());

        V_Pos[i] = new TProfile2D(Form("V_Pos_V%dV%d", i, i + 1),
                                  Form("V_Pos_V%dV%d", i, i + 1),
                                  nXbins, HbinEdges.data(),
                                  nYbins, VbinEdges.data());

        H_Pos2[i] = new TProfile2D(Form("H_Pos2_H%dH%d", i, i + 1),
                                   Form("H_Pos2_H%dH%d", i, i + 1),
                                   nXbins, HbinEdges.data(),
                                   nYbins, VbinEdges.data());

        V_Pos2[i] = new TProfile2D(Form("V_Pos2_V%dV%d", i, i + 1),
                                   Form("V_Pos2_V%dV%d", i, i + 1),
                                   nXbins, HbinEdges.data(),
                                   nYbins, VbinEdges.data());

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

        // Titles
        H_Pos[i]->GetXaxis()->SetTitle("H0 position [mm]");
        H_Pos[i]->GetYaxis()->SetTitle("V0 position [mm]");
        V_Pos[i]->GetXaxis()->SetTitle("H0 position [mm]");
        V_Pos[i]->GetYaxis()->SetTitle("V0 position [mm]");

        H_Pos2[i]->GetXaxis()->SetTitle("H0 position [mm]");
        H_Pos2[i]->GetYaxis()->SetTitle("V0 position [mm]");
        V_Pos2[i]->GetXaxis()->SetTitle("H0 position [mm]");
        V_Pos2[i]->GetYaxis()->SetTitle("V0 position [mm]");

        H_PosBias[i]->GetXaxis()->SetTitle("H0 position [mm]");
        H_PosBias[i]->GetYaxis()->SetTitle("V0 position [mm]");
        V_PosBias[i]->GetXaxis()->SetTitle("H0 position [mm]");
        V_PosBias[i]->GetYaxis()->SetTitle("V0 position [mm]");

        H_PosRes[i]->GetXaxis()->SetTitle("H0 position [mm]");
        H_PosRes[i]->GetYaxis()->SetTitle("V0 position [mm]");
        V_PosRes[i]->GetXaxis()->SetTitle("H0 position [mm]");
        V_PosRes[i]->GetYaxis()->SetTitle("V0 position [mm]");
    }
}

inline bool Pos2DMap::getBinEdges(const RunContext &ctx) // from the TTree in Pos1D.root
{
    TFile *binFile = TFile::Open(Form("output/run%d_Pos1D.root", ctx.run_number), "READ");
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
    std::cout << "it is fine here " << std::endl;

    binFile->Close();
    return true;
}

inline void Pos2DMap::end_run(const RunContext &ctx)
{
    (void)ctx;

    if (!file_ || !dir_ || !histReady_)
        return;

    dir_->cd();

    for (int i = 0; i < nrBoards / 2; ++i)
    {
        if (!Pos2D[i])
            continue;

        // clean low-stat bins
        for (int ix = 1; ix <= Pos2D[i]->GetNbinsX(); ++ix)
        {
            for (int iy = 1; iy <= Pos2D[i]->GetNbinsY(); ++iy)
            {
                if (Pos2D[i]->GetBinContent(ix, iy) <= 2)
                    Pos2D[i]->SetBinContent(ix, iy, 0);
            }
        }
        Pos2D[i]->Write();

        if (i < nrBoards / 2 - 1)
        {
            if (H_Pos[i])
                H_Pos[i]->Write();
            if (V_Pos[i])
                V_Pos[i]->Write();

            // H maps
            if (H_Pos[i] && H_Pos2[i] && H_PosBias[i] && H_PosRes[i])
            {
                for (int ix = 1; ix <= H_Pos[i]->GetNbinsX(); ++ix)
                {
                    for (int iy = 1; iy <= H_Pos[i]->GetNbinsY(); ++iy)
                    {
                        double mean = H_Pos[i]->GetBinContent(ix, iy);
                        double mean2 = H_Pos2[i]->GetBinContent(ix, iy);
                        double var = mean2 - mean * mean;
                        double stdev = var > 0 ? std::sqrt(var) : 0;

                        int bin = H_Pos[i]->GetBin(ix, iy);
                        if (H_Pos[i]->GetBinEntries(bin) > 3)
                        {
                            H_PosBias[i]->SetBinContent(ix, iy, mean);
                            H_PosRes[i]->SetBinContent(ix, iy, stdev);
                        }
                    }
                }
                // H_PosBias[i]->GetZaxis()->SetRangeUser(-2, 2);
                H_PosBias[i]->Write();
                // H_PosRes[i]->GetZaxis()->SetRangeUser(0, 0.5);
                H_PosRes[i]->Write();
            }

            // V maps
            if (V_Pos[i] && V_Pos2[i] && V_PosBias[i] && V_PosRes[i])
            {
                for (int ix = 1; ix <= V_Pos[i]->GetNbinsX(); ++ix)
                {
                    for (int iy = 1; iy <= V_Pos[i]->GetNbinsY(); ++iy)
                    {
                        double mean = V_Pos[i]->GetBinContent(ix, iy);
                        double mean2 = V_Pos2[i]->GetBinContent(ix, iy);
                        double var = mean2 - mean * mean;
                        double stdev = var > 0 ? std::sqrt(var) : 0;

                        int bin = V_Pos[i]->GetBin(ix, iy);
                        if (V_Pos[i]->GetBinEntries(bin) > 3) // FIX: was H_Pos
                        {
                            V_PosBias[i]->SetBinContent(ix, iy, mean);
                            V_PosRes[i]->SetBinContent(ix, iy, stdev);
                        }
                    }
                }
                V_PosBias[i]->GetZaxis()->SetRangeUser(-1, 1);
                V_PosBias[i]->Write();
                V_PosRes[i]->GetZaxis()->SetRangeUser(0, 0.9);
                V_PosRes[i]->Write();
            }
        }
    }

    file_->Close();
}