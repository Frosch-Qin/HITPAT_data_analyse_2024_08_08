// Sum1DPos.h //x with aligned position
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class Sum1DPos : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "Sum1DPos";
    }

protected:
    void on_begin_run(const RunContext &ctx) override
    {
        readIn_align(ctx);
        convert_sum1d_to_position(ctx);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        (void)frame;
        (void)frame_index;
        (void)tags;
        // nothing to do event-by-event
    }

    void end_run(const RunContext &ctx) override
    {
        (void)ctx;
    }

private:
    double intercept_scaled[6] = {0, 0, 0, 0, 0, 0};
    double slope_scaled[6] = {1, 1, 1, 1, 1, 1};

    const double offset_origin = 128.4; // same as PosAlign
    const double pitch_mm = 0.8;        // 256 mm / 320 channels

    void readIn_align(const RunContext &ctx);
    void convert_sum1d_to_position(const RunContext &ctx);

    double channel_to_mm(double ch) const
    {
        return (ch)*pitch_mm + 0.2 * floor(ch / 64);
    }

    TGraph *convert_graph(const TGraph *gin, int boardID, const char *newName)
    {
        if (!gin)
            return nullptr;

        const int n = gin->GetN();
        TGraph *gout = new TGraph();
        gout->SetName(newName);
        gout->SetTitle(gin->GetTitle());

        for (int i = 0; i < n; ++i)
        {
            double x = 0.0, y = 0.0;
            gin->GetPoint(i, x, y);

            // x is ChannelID -> convert to raw mm
            double xpos = channel_to_mm(x);

            // apply same alignment as PosAlign
            double xaligned =
                xpos * slope_scaled[boardID] + intercept_scaled[boardID] - offset_origin;

            gout->SetPoint(i, xaligned, y);
        }

        return gout;
    }
};

inline void Sum1DPos::readIn_align(const RunContext &ctx)
{
    TFile *file = TFile::Open(Form("output2025/%s_Pos1D.root", ctx.ALIGN_runname), "READ");
    if (!file || file->IsZombie())
    {
        std::cout << Form("output2025/%s_Pos1D.root does not exist", ctx.ALIGN_runname) << std::endl;
        return;
    }

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
        // std::cout << intercept_scaled[i] << " "<<  slope_scaled[i] << std::endl;
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

inline void Sum1DPos::convert_sum1d_to_position(const RunContext &ctx)
{
    TFile *fin = TFile::Open(Form("output2025/run%d_Sum1D.root", ctx.run_number), "READ");
    if (!fin || fin->IsZombie())
    {
        std::cout << Form("output2025/run%d_Sum1D.root does not exist", ctx.run_number) << std::endl;
        return;
    }

    TFile *fout = new TFile(Form("output2025/run%d_Sum1DPos.root", ctx.run_number), "RECREATE");
    TDirectory *dir = fout->mkdir("Sum1DPos");
    dir->cd();

    for (int i = 0; i < nrBoards; ++i)
    {
        const char *bname = ctx.BoardName[i];

        TGraph *g_pedA = (TGraph *)fin->Get(Form("Sum1D/pedestalA_graph_%s", bname));
        TGraph *g_pedB = (TGraph *)fin->Get(Form("Sum1D/pedestalB_graph_%s", bname));
        TGraph *g_sig = (TGraph *)fin->Get(Form("Sum1D/signal_graph_%s", bname));
        TGraph *g_sigSub = (TGraph *)fin->Get(Form("Sum1D/signal_sub_pedestalA_graph_%s", bname));
        TGraph *g_bSub = (TGraph *)fin->Get(Form("Sum1D/pedestalB_sub_pedestalA_graph_%s", bname));

        TGraph *out_pedA = convert_graph(g_pedA, i, Form("pedestalA_graph_%s", bname));
        TGraph *out_pedB = convert_graph(g_pedB, i, Form("pedestalB_graph_%s", bname));
        TGraph *out_sig = convert_graph(g_sig, i, Form("signal_graph_%s", bname));
        TGraph *out_sigSub = convert_graph(g_sigSub, i, Form("signal_sub_pedestalA_graph_%s", bname));
        TGraph *out_bSub = convert_graph(g_bSub, i, Form("pedestalB_sub_pedestalA_graph_%s", bname));

        if (out_pedA)
        {
            out_pedA->GetXaxis()->SetTitle(Form("%s aligned position [mm]", bname));
            out_pedA->Write();
        }
        if (out_pedB)
        {
            out_pedB->GetXaxis()->SetTitle(Form("%s aligned position [mm]", bname));
            out_pedB->Write();
        }
        if (out_sig)
        {
            out_sig->GetXaxis()->SetTitle(Form("%s aligned position [mm]", bname));
            out_sig->Write();
        }
        if (out_sigSub)
        {
            out_sigSub->GetXaxis()->SetTitle(Form("%s aligned position [mm]", bname));
            out_sigSub->Write();
        }
        if (out_bSub)
        {
            out_bSub->GetXaxis()->SetTitle(Form("%s aligned position [mm]", bname));
            out_bSub->Write();
        }
    }

    fout->Close();
    fin->Close();

    delete fout;
    delete fin;
}