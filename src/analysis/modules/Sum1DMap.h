// Sum1DMap.h
// add up raw data; then apply pedestal correction (from Sum1D: pedestalA) and calibration.
// using the position information from H1,V1, calculate the signal amplitude for each board and expend it along the fiber direction.
// the results should be comparable with Sum1D
//h2Map_sub_pedestalA_norm shows the attenuation of each channel for each board
//the attenuation map shows the variation of attenuation between channels. which is relatively constant.

#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TH2D.h"
#include "TStyle.h"

#include "../IAnalyzer.h"
#include "../FrameTags.h"
#include "ScanBin.C" // convertPeaksToBinEdges

class Sum1DMap : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "Sum1DMap";
    }

protected:
    void on_begin_run(RunContext &ctx) override
    {
        file_ = new TFile(Form("output2025/run%d_%s.root",
                               ctx.run_number, name().c_str()),
                          "RECREATE");

        if (!file_ || file_->IsZombie())
        {
            std::cerr << "Sum1DMap: failed to create output file.\n";
            return;
        }

        if (!getBinEdges(ctx))
        {
            std::cerr << "Sum1DMap: failed to get H/V bin edges.\n";
            return;
        }

        buildChannelPositionEdges();
        createHistograms(ctx);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        (void)frame_index;

        if (!file_ || !dir_)
            return;

        if (!tags.BKG_SUB_ON)
            return;

        if (tags.SpillID < 0)
            return;

        if (!tags.Has_signal)
            return;

        const int hrefID = H_boardID[1]; // H1
        const int vrefID = V_boardID[1]; // V1

        if (tags.boardTags[hrefID].Cluster_num < 1)
            return;
        if (tags.boardTags[vrefID].Cluster_num < 1)
            return;

        const double hrefPos = tags.boardTags[hrefID].Position;
        const double vrefPos = tags.boardTags[vrefID].Position;

        for (int ib = 0; ib < nrBoards; ++ib)
        {
            if (!h2Map_raw[ib])
                continue;

            const bool isH = isHBoard(ib);
            const bool isV = isVBoard(ib);

            if (isH)
            {
                // H board:
                // x = channel position in mm
                // y = V reference position bin
                const int ybin = h2Map_raw[ib]->GetYaxis()->FindBin(vrefPos);

                if (ybin < 1 || ybin > h2Map_raw[ib]->GetNbinsY())
                    continue;

                const int nCh = std::min(frame.boards[ib].nrChannels, 320);

                for (int ch = 0; ch < nCh; ++ch)
                {
                    const double val = tags.raw_frame.boards[ib].data[ch];
                    const int xbin = ch + 1;

                    const double old = h2Map_raw[ib]->GetBinContent(xbin, ybin);
                    h2Map_raw[ib]->SetBinContent(xbin, ybin, old + val);
                }

                // add up g1_entries
                const double old_g = g1_entries[ib]->GetPointY(ybin - 1);
                g1_entries[ib]->SetPointY(ybin - 1, old_g + 1);
            }
            else if (isV)
            {
                // V board:
                // x = H reference position bin
                // y = channel position in mm
                const int xbin = h2Map_raw[ib]->GetXaxis()->FindBin(hrefPos);

                if (xbin < 1 || xbin > h2Map_raw[ib]->GetNbinsX())
                    continue;

                const int nCh = std::min(frame.boards[ib].nrChannels, 320);

                for (int ch = 0; ch < nCh; ++ch)
                {
                    const double val = tags.raw_frame.boards[ib].data[ch];
                    const int ybin = ch + 1;

                    const double old = h2Map_raw[ib]->GetBinContent(xbin, ybin);
                    h2Map_raw[ib]->SetBinContent(xbin, ybin, old + val);
                }

                // add up g1_entries
                const double old_g = g1_entries[ib]->GetPointY(xbin - 1);
                g1_entries[ib]->SetPointY(xbin - 1, old_g + 1);
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        writeHistograms(ctx);
    }

private:
    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;

    // reference position binning from Pos1D peak trees
    std::vector<double> HbinEdges;
    std::vector<double> VbinEdges;

    // 320 channel-position bin edges in mm
    std::vector<double> channelPosEdges;

    std::vector<double> peaksH;
    std::vector<double> peaksV;

    int nXbins = 0; // H position bins
    int nYbins = 0; // V position bins

    std::vector<TH2D *> h2Map_raw; // used as 3D TGraph, showing sum signal
    std::vector<TH2D *> h2Map_sub_pedestalA;
    std::vector<TH2D *> h2Map_sub_pedestalA_cali;
    std::vector<TH2D *> h2Map_sub_pedestalA_norm;

    std::vector<TGraph *> g1_entries; // the frame number for each bin in h2Map summing-up axis

    double pedestalA[6][320]{};
    double calFac[6][320] = {1}; // 6 boards, 320 channels

    double offset_origin = 128.4;

    bool isHBoard(int boardID) const
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            if (boardID == H_boardID[i])
                return true;
        }
        return false;
    }

    bool isVBoard(int boardID) const
    {
        for (int i = 0; i < nrBoards / 2; ++i)
        {
            if (boardID == V_boardID[i])
                return true;
        }
        return false;
    }

    double channelToMmCenter(int ch) const
    {
        return ch * 0.8 + std::floor(ch / 64.0) * 0.2 - offset_origin;
    }

    void buildChannelPositionEdges()
    {
        channelPosEdges.clear();
        channelPosEdges.resize(321, 0.0);

        std::vector<double> centers(320, 0.0);
        for (int ch = 0; ch < 320; ++ch)
            centers[ch] = channelToMmCenter(ch);

        channelPosEdges[0] = centers[0] - 0.5 * (centers[1] - centers[0]);

        for (int ch = 0; ch < 319; ++ch)
            channelPosEdges[ch + 1] = 0.5 * (centers[ch] + centers[ch + 1]);

        channelPosEdges[320] = centers[319] + 0.5 * (centers[319] - centers[318]);
    }

    bool edgesStrictlyIncreasing(const std::vector<double> &v) const
    {
        if (v.size() < 2)
            return false;

        for (size_t i = 1; i < v.size(); ++i)
        {
            if (v[i] <= v[i - 1])
                return false;
        }
        return true;
    }

    void createHistograms(const RunContext &ctx)
    {
        if (!file_)
            return;

        dir_ = file_->GetDirectory(name().c_str());
        if (!dir_)
            dir_ = file_->mkdir(name().c_str());
        dir_->cd();

        const int hrefID = H_boardID[1]; // H1
        const int vrefID = V_boardID[1]; // V1

        h2Map_raw.clear();
        h2Map_sub_pedestalA.clear();
        h2Map_sub_pedestalA_cali.clear();
        h2Map_sub_pedestalA_norm.clear();
        h2Map_raw.resize(nrBoards, nullptr);
        h2Map_sub_pedestalA.resize(nrBoards, nullptr);
        h2Map_sub_pedestalA_cali.resize(nrBoards, nullptr);
        h2Map_sub_pedestalA_norm.resize(nrBoards, nullptr);

        for (int ib = 0; ib < nrBoards; ++ib)
        {
            const char *bname = ctx.BoardName[ib];

            if (isHBoard(ib))
            {
                h2Map_raw[ib] = new TH2D(
                    Form("Sum1DMap_%s_vs_%s", bname, ctx.BoardName[vrefID]),
                    Form("%s summed by %s position;%s position [mm];%s position [mm]",
                         bname, ctx.BoardName[vrefID], bname, ctx.BoardName[vrefID]),
                    320, channelPosEdges.data(),
                    nYbins, VbinEdges.data());
            }
            else if (isVBoard(ib))
            {
                h2Map_raw[ib] = new TH2D(
                    Form("Sum1DMap_%s_vs_%s", bname, ctx.BoardName[hrefID]),
                    Form("%s summed by %s position;%s position [mm];%s position [mm]",
                         bname, ctx.BoardName[hrefID], ctx.BoardName[hrefID], bname),
                    nXbins, HbinEdges.data(),
                    320, channelPosEdges.data());
            }
        }

        // create g1_entries
        g1_entries.clear();
        g1_entries.resize(nrBoards, nullptr);

        for (int ib = 0; ib < nrBoards; ++ib)
        {
            const char *bname = ctx.BoardName[ib];

            if (isHBoard(ib))
            {
                g1_entries[ib] = new TGraph(nYbins);
                g1_entries[ib]->SetNameTitle(Form("Sum1DMap_%s_vs_%s_entries", bname, ctx.BoardName[vrefID]),
                                             Form("%s summed by %s position;%s position [mm];%s entries",
                                                  bname, ctx.BoardName[vrefID], ctx.BoardName[vrefID], bname));
                // write g1_entries[ib] as (VbinEdges,0)
                for (int i = 0; i < nYbins; i++)
                {
                    g1_entries[ib]->SetPoint(i, VbinEdges[i], 0.0);
                }
            }
            else if (isVBoard(ib))
            {
                g1_entries[ib] = new TGraph(nXbins);
                g1_entries[ib]->SetNameTitle(Form("Sum1DMap_%s_vs_%s_entries", bname, ctx.BoardName[hrefID]),
                                             Form("%s summed by %s position;%s position [mm];%s entries",
                                                  bname, ctx.BoardName[hrefID], ctx.BoardName[hrefID], bname));
                // write g1_entries[ib] as (HbinEdges,0)
                for (int i = 0; i < nXbins; i++)
                {
                    g1_entries[ib]->SetPoint(i, HbinEdges[i], 0.0);
                }
            }
        }
    }

    void set_fine_z_display(TH2D *h,
                            double zmin = 0.8,
                            double zmax = 1.2,
                            int nContours = 999)
    {
        if (!h)
            return;

        h->SetMinimum(zmin);
        h->SetMaximum(zmax);
        h->SetContour(nContours);
        gStyle->SetNumberContours(nContours);

        h->GetZaxis()->SetRangeUser(zmin, zmax);
        h->GetZaxis()->SetNdivisions(510, false);
    }

    // read in pedestalA number, and subtract pedestalA*number of entries

    void readinPedestalA(const RunContext &ctx)
    {
        TFile *pedestal_File = TFile::Open(Form("output2025/run%d_Sum1D.root", ctx.run_number), "READ");

        if (!pedestal_File || pedestal_File->IsZombie())
        {
            std::cout << "For Sum1DMap: cannot open pedestal file.\n";
            std::cout << "pedestalA = 0.\n";
            return;
        }

        for (int i = 0; i < nrBoards; ++i)
        {
            const char *boardNamei = ctx.BoardName[i];
            TGraph *pedestalA_graph = (TGraph *)pedestal_File->Get(Form("Sum1D/pedestalA_graph_%s", boardNamei));
            if (pedestalA_graph)
            {
                for (int j = 0; j < 320; ++j)
                {
                    pedestalA[i][j] = pedestalA_graph->GetPointY(j);
                    // std::cout << "pedestalA[" << i << "][" << j << "] = " << pedestalA[i][j] << std::endl;
                }
            }
            else
            {
                std::cout << "pedestalA_graph not found for board " << boardNamei << std::endl;
            }
        }

        pedestal_File->Close();
    }
    void pedestalCorrection(const RunContext &ctx, TH2D *h, int ib)
    {

        const char *bname = ctx.BoardName[ib];

        if (isHBoard(ib))
        {
            // go through Y bins; each Y bin has an entry number from g1_entries[ib]
            for (int ybin = 1; ybin <= nYbins; ++ybin)
            {
                const double num_entry = g1_entries[ib]->GetPointY(ybin - 1);

                // go through X bins, subtract pedestalA*num_entry
                for (int xbin = 1; xbin <= 320; ++xbin)
                {
                    const double old_h = h->GetBinContent(xbin, ybin);
                    h->SetBinContent(xbin, ybin, old_h - pedestalA[ib][xbin - 1] * num_entry);
                }
            }
        }
        else if (isVBoard(ib))
        {
            // go through X bins; each X bin has an entry number from g1_entries[ib]
            for (int xbin = 1; xbin <= nXbins; ++xbin)
            {
                const double num_entry = g1_entries[ib]->GetPointY(xbin - 1);

                // go through Y bins, subtract pedestalA*num_entry
                for (int ybin = 1; ybin <= 320; ++ybin)
                {
                    const double old_h = h->GetBinContent(xbin, ybin);
                    h->SetBinContent(xbin, ybin, old_h - pedestalA[ib][ybin - 1] * num_entry);

                    // std::cout << "Bin(" << xbin << ", " << ybin << ") = " << pedestalA[ib][ybin - 1] * num_entry << " -> " << h->GetBinContent(xbin, ybin) << std::endl;
                }
            }
        }
    }

    void calibration(const RunContext &ctx, TH2D *h, int ib)
    {
        const char *bname = ctx.BoardName[ib];

        if (isHBoard(ib))
        {
            for (int ybin = 1; ybin <= nYbins; ++ybin)
            {
                for (int xbin = 1; xbin <= 320; ++xbin)
                {
                    const double old_h = h->GetBinContent(xbin, ybin);
                    h->SetBinContent(xbin, ybin, old_h * calFac[ib][xbin - 1]);
                }
            }
        }
        else if (isVBoard(ib))
        {
            for (int xbin = 1; xbin <= nXbins; ++xbin)
            {
                for (int ybin = 1; ybin <= 320; ++ybin)
                {
                    const double old_h = h->GetBinContent(xbin, ybin);
                    h->SetBinContent(xbin, ybin, old_h * calFac[ib][ybin - 1]);
                }
            }
        }
    }

    void writeHistograms(const RunContext &ctx)
    {
        readinPedestalA(ctx);
        get_calfac(ctx);
        if (!file_ || !dir_)
            return;

        file_->cd();
        dir_->cd();

        for (int i = 0; i < nrBoards; ++i)
        {
            if (!h2Map_raw[i])
                continue;

            h2Map_sub_pedestalA[i] = (TH2D *)h2Map_raw[i]->Clone(Form("%s_sub_pedestalA", h2Map_raw[i]->GetName()));
            pedestalCorrection(ctx, h2Map_sub_pedestalA[i], i);

            if (!h2Map_sub_pedestalA[i])
                continue;

            h2Map_sub_pedestalA_cali[i] = (TH2D *)h2Map_sub_pedestalA[i]->Clone(Form("%s_cali", h2Map_sub_pedestalA[i]->GetName()));
            calibration(ctx, h2Map_sub_pedestalA_cali[i], i);

            if (!h2Map_sub_pedestalA_cali[i])
                continue;

            h2Map_sub_pedestalA_norm[i] = (TH2D *)h2Map_sub_pedestalA_cali[i]->Clone(Form("%s_norm", h2Map_sub_pedestalA[i]->GetName()));

            const char *name = ctx.BoardName[i];

            if (name[0] == 'V')
            {
                normalize_by_leftmost_xbin(h2Map_sub_pedestalA_norm[i]);
            }
            else if (name[0] == 'H')
            {
                normalize_by_bottom_ybin(h2Map_sub_pedestalA_norm[i]);
            }

            // make color variation around 1 visible
            // set_fine_z_display(h2Map_sub_pedestalA[i], 0.8, 1.2, 999);

            h2Map_raw[i]->Write();
            h2Map_sub_pedestalA[i]->Write();
            h2Map_sub_pedestalA_cali[i]->Write();
            h2Map_sub_pedestalA_norm[i]->Write();
            g1_entries[i]->Write();
        }

        file_->Close();
        delete file_;
        file_ = nullptr;
        dir_ = nullptr;
    }

    void normalize_by_leftmost_xbin(TH2D *h)
    {
        if (!h)
            return;

        const int nx = h->GetNbinsX();
        const int ny = h->GetNbinsY();

        for (int iy = 1; iy <= ny; ++iy)
        {
            const double ref = h->GetBinContent(1, iy); // leftmost x-bin

            if (ref < 10.0)
                continue;

            for (int ix = 1; ix <= nx; ++ix)
            {
                const double val = h->GetBinContent(ix, iy);
                double new_val = val / ref * 1.0;
                if (abs(new_val) > 5)
                    h->SetBinContent(ix, iy, 0);
                else
                    h->SetBinContent(ix, iy, new_val);
                // std::cout << "Bin(" << ix << ", " << iy << ") = " << h->GetBinContent(ix, iy) << std::endl;
            }
        }
    }

    void normalize_by_bottom_ybin(TH2D *h)
    {
        if (!h)
            return;

        const int nx = h->GetNbinsX();
        const int ny = h->GetNbinsY();

        for (int ix = 1; ix <= nx; ++ix)
        {
            const double ref = h->GetBinContent(ix, 1); // bottom y-bin

            if (ref < 10.0)
                continue;

            for (int iy = 1; iy <= ny; ++iy)
            {
                const double val = h->GetBinContent(ix, iy);
                double new_val = val / ref * 1.0;
                if (abs(new_val) > 5)
                    h->SetBinContent(ix, iy, 0);
                else
                    h->SetBinContent(ix, iy, new_val);
                // std::cout << "Bin(" << ix << ", " << iy << ") = " << h->GetBinContent(ix, iy) << std::endl;
            }
        }
    }

    bool getBinEdges(const RunContext &ctx)
    {
        TFile *binFile = TFile::Open(Form("output2025/run%d_Pos1D.root",
                                          ctx.run_number),
                                     "READ");
        if (!binFile || binFile->IsZombie())
        {
            std::cerr << "Error: cannot open file to get bin edges.\n";
            return false;
        }

        TTree *t_H = (TTree *)binFile->Get("Pos1D/t_Hpos");
        TTree *t_V = (TTree *)binFile->Get("Pos1D/t_Vpos");

        if (!t_H || !t_V)
        {
            std::cerr << "Error: cannot find Pos1D/t_Hpos or Pos1D/t_Vpos\n";
            binFile->Close();
            return false;
        }

        const char *HrefName = ctx.BoardName[H_boardID[1]]; // H1
        const char *VrefName = ctx.BoardName[V_boardID[1]]; // V1

        peaksH.clear();
        peaksV.clear();

        double peak_Href = 0.0;
        double peak_Vref = 0.0;

        t_H->SetBranchAddress(HrefName, &peak_Href);
        t_V->SetBranchAddress(VrefName, &peak_Vref);

        for (Long64_t i = 0; i < t_H->GetEntries(); ++i)
        {
            t_H->GetEntry(i);
            peaksH.push_back(peak_Href - offset_origin);
        }

        for (Long64_t i = 0; i < t_V->GetEntries(); ++i)
        {
            t_V->GetEntry(i);
            peaksV.push_back(peak_Vref - offset_origin);
        }

        HbinEdges.clear();
        VbinEdges.clear();

        convertPeaksToBinEdges(peaksH, HbinEdges);
        convertPeaksToBinEdges(peaksV, VbinEdges);

        nXbins = (int)HbinEdges.size() - 1;
        nYbins = (int)VbinEdges.size() - 1;

        if (nXbins <= 0 || nYbins <= 0)
        {
            std::cerr << "FATAL: invalid nbins (nXbins=" << nXbins
                      << ", nYbins=" << nYbins << ")\n";
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

    void get_calfac(const RunContext &ctx)
    {
        TFile *calFile = TFile::Open(Form("cal_pre/output2025/cal_%s.root", ctx.CAL_runname), "READ");

        if (!calFile || calFile->IsZombie())
        {
            std::cout << "Warning Sum1DMap: cannot open calibration file.\n";
            return;
        }

        TGraph *cal[nrBoards];
        for (int i = 0; i < nrBoards; i++)
        {
            cal[i] = (TGraph *)calFile->Get(Form("cal%d", i));
        }

        for (int j = 0; j < nrBoards; j++)
        {
            for (int i = 0; i < 320; i++)
            {
                calFac[j][i] = cal[j]->GetPointY(i) / 8192;
                // std::cout << "Board " << j << " Channel " << i << " Uncalibration Factor: " << calFac[j][i] << std::endl;
                // calFac[j][i] = 1; //for run1 to run5
            }
        }
        calFile->Close();
    }
};