#include "lhcbStyle.h"
#include <vector>
#include <iostream>
#include <cmath>

#include <vector>
#include <iostream>

TGraph *get_SumX(const TH2D *h2, double x_min = -1e30, double x_max = 1e30, double y_min = -1e30, double y_max = 1e30)
{
    if (!h2)
        return nullptr;

    std::vector<double> vx;
    std::vector<double> vy;

    const int nBinsX = h2->GetNbinsX();
    const int nBinsY = h2->GetNbinsY();

    for (int ix = 1; ix <= nBinsX; ++ix)
    {
        const double x = h2->GetXaxis()->GetBinCenter(ix);
        if (x < x_min || x > x_max)
            continue;

        double sum = 0.0;
        for (int iy = 1; iy <= nBinsY; ++iy)
        {
            if (h2->GetYaxis()->GetBinCenter(iy) < y_min || h2->GetYaxis()->GetBinCenter(iy) > y_max)
                continue;

            sum += h2->GetBinContent(ix, iy);
        }

        vx.push_back(x);
        vy.push_back(sum);
    }

    if (vx.empty())
        return nullptr;

    // normalize the vector by the first index
    const double first = vy.front();
    for (double &y : vy)
        y /= first;
    const double first_x = vx.front();
    for (double &x : vx)
        x -= first_x;

    TGraph *g = new TGraph((int)vx.size(), vx.data(), vy.data());
    g->SetName(Form("%s_SumX", h2->GetName()));
    g->SetTitle(Form("%s; %s; Sum", h2->GetTitle(), h2->GetXaxis()->GetTitle()));
    g->SetMarkerStyle(20);
    g->SetMarkerSize(0.5);
    g->SetLineWidth(2);

    return g;
}

TGraph *get_SumY(const TH2D *h2, double x_min = -1e30, double x_max = 1e30, double y_min = -1e30, double y_max = 1e30)
{
    if (!h2)
        return nullptr;

    std::vector<double> vx;
    std::vector<double> vy;

    const int nBinsX = h2->GetNbinsX();
    const int nBinsY = h2->GetNbinsY();

    for (int iy = 1; iy <= nBinsY; ++iy)
    {
        const double y = h2->GetYaxis()->GetBinCenter(iy);
        if (y < y_min || y > y_max)
            continue;

        double sum = 0.0;
        for (int ix = 1; ix <= nBinsX; ++ix)
        {
            if (h2->GetXaxis()->GetBinCenter(ix) < x_min || h2->GetXaxis()->GetBinCenter(ix) > x_max)
                continue;

            sum += h2->GetBinContent(ix, iy);
        }

        vx.push_back(y);
        vy.push_back(sum);
    }

    if (vx.empty())
        return nullptr;

    // normalize the vector by the first index
    const double first = vy.front();
    for (double &y : vy)
        y /= first;
    const double first_x = vx.front();
    for (double &x : vx)
        x -= first_x;

    TGraph *g = new TGraph((int)vx.size(), vx.data(), vy.data());
    g->SetName(Form("%s_SumY", h2->GetName()));
    g->SetTitle(Form("%s; %s; Sum", h2->GetTitle(), h2->GetYaxis()->GetTitle()));
    g->SetMarkerStyle(20);
    g->SetMarkerSize(0.5);
    g->SetLineWidth(2);

    return g;
}

TH1D *get_TH2DZ(const TH2D *h2, double x_min = -1e30, double x_max = 1e30, double y_min = -1e30, double y_max = 1e30, double z_min = -1e30, double z_max = 1e30)
{
    if (!h2)
        return nullptr;

    const int nBinsX = h2->GetNbinsX();
    const int nBinsY = h2->GetNbinsY();

    TH1D *h1d = new TH1D(Form("%s_TH2DZ", h2->GetName()), Form("%s; %s; Z", h2->GetTitle(), h2->GetXaxis()->GetTitle()), 100, z_min, z_max);

    for (int ix = 1; ix <= nBinsX; ++ix)
    {
        const double x = h2->GetXaxis()->GetBinCenter(ix);
        if (x < x_min || x > x_max)
            continue;

        for (int iy = 1; iy <= nBinsY; ++iy)
        {
            if (h2->GetYaxis()->GetBinCenter(iy) < y_min || h2->GetYaxis()->GetBinCenter(iy) > y_max)
                continue;

            h1d->Fill(h2->GetBinContent(ix, iy));
        }
    }

    return h1d;
}

void plot_Sum1DMap(const char *run_name,
                   const char *group,
                   double z_min,
                   double z_max,
                   std::vector<TGraph *> &gx_list,
                   std::vector<TGraph *> &gy_list,
                   std::vector<TH1D *> &h1_list)
{
    TFile *fline = TFile::Open(Form("output2025/%s_Sum1DMap.root", run_name));
    if (!fline || fline->IsZombie())
    {
        std::cerr << "Cannot open file for run " << run_name << std::endl;
        return;
    }

    TH2D *h2 = (TH2D *)fline->Get(Form("Sum1DMap/Sum1DMap_%s", group));
    if (!h2)
    {
        std::cerr << "Cannot find histogram Sum1DMap/Sum1DMap_" << group << std::endl;
        fline->Close();
        delete fline;
        return;
    }

    // detach from file, so object survives after file close
    h2 = (TH2D *)h2->Clone(Form("h2_%s_%s", group, run_name));
    h2->SetDirectory(nullptr);

    // -------- draw TH2D --------
    TCanvas *c1 = new TCanvas(Form("c1_%s_%s", group, run_name),
                              Form("c1_%s_%s", group, run_name),
                              800, 800);
    h2->GetXaxis()->SetRangeUser(-70, 45);
    h2->GetYaxis()->SetRangeUser(-65, 60);
    h2->GetZaxis()->SetRangeUser(z_min, z_max);
    h2->SetStats(0);
    h2->Draw("COLZ");
    c1->SaveAs(Form("output2025/plots/Sum1DMap_%s_%s.png", group, run_name));
    delete c1;

    // -------- build graph / hist --------
    TGraph *gx = get_SumX(h2, -70, 45, -65, 60);
    TGraph *gy = get_SumY(h2, -70, 45, -65, 60);
    TH1D *h1 = get_TH2DZ(h2, -70, 45, -65, 60, 0.55, 1.0);

    if (gx)
    {
        //     gx->SetName(Form("gx_%s_%s", group, run_name));
        //     gx->SetTitle(Form("Sum1DMap_%s_%s", group, run_name));

        //     TCanvas *c2 = new TCanvas(Form("c2_%s_%s", group, run_name),
        //                               Form("c2_%s_%s", group, run_name),
        //                               800, 800);
        //     gx->GetXaxis()->SetTitle("X (mm)");
        //     gx->GetYaxis()->SetTitle("Sum");
        //     gx->Draw("APL");
        //     c2->SaveAs(Form("output2025/plots/Sum1DMap_X_%s_%s.png", group, run_name));
        //     delete c2;

        gx_list.push_back(gx);
    }

    if (gy)
    {
        //     gy->SetName(Form("gy_%s_%s", group, run_name));
        //     gy->SetTitle(Form("Sum1DMap_%s_%s", group, run_name));

        //     TCanvas *c3 = new TCanvas(Form("c3_%s_%s", group, run_name),
        //                               Form("c3_%s_%s", group, run_name),
        //                               800, 800);
        //     gy->GetXaxis()->SetTitle("Y (mm)");
        //     gy->GetYaxis()->SetTitle("Sum");
        //     gy->Draw("APL");
        //     c3->SaveAs(Form("output2025/plots/Sum1DMap_Y_%s_%s.png", group, run_name));
        //     delete c3;

        gy_list.push_back(gy);
    }

    if (h1)
    {
        //     h1->SetName(Form("h1_%s_%s", group, run_name));
        //     h1->SetTitle(Form("Sum1DMap_%s_%s", group, run_name));

        //     TCanvas *c4 = new TCanvas(Form("c4_%s_%s", group, run_name),
        //                               Form("c4_%s_%s", group, run_name),
        //                               800, 800);
        //     h1->GetXaxis()->SetTitle("Z");
        //     h1->GetYaxis()->SetTitle(" ");
        //     h1->Draw("HIST");
        //     c4->SaveAs(Form("output2025/plots/Sum1DMap_Z_%s_%s.png", group, run_name));
        //     delete c4;

        h1_list.push_back(h1);
    }

    delete h2;
    fline->Close();
    delete fline;
}

void plot_all_Sum1DMap()
{
    lhcbStyle();

    std::vector<TGraph *> g_att;
    std::vector<TGraph *> g_sum;
    std::vector<TH1D *> h1;

    // order in V2V2,V1V1,H2H2,H1H1
    plot_Sum1DMap("run4", "V2_vs_H1_sub_pedestalA_norm", 0.55, 1.0, g_att, g_sum, h1); // evenID run4 solid line
    plot_Sum1DMap("run3", "V2_vs_H1_sub_pedestalA_norm", 0.55, 1.0, g_att, g_sum, h1); // oddID  run3 dashed line

    plot_Sum1DMap("run4", "V1_vs_H1_sub_pedestalA_norm", 0.55, 1.0, g_att, g_sum, h1);
    plot_Sum1DMap("run3", "V1_vs_H1_sub_pedestalA_norm", 0.55, 1.0, g_att, g_sum, h1);

    plot_Sum1DMap("run4", "H2_vs_V1_sub_pedestalA_norm", 0.70, 1.0, g_sum, g_att, h1);
    plot_Sum1DMap("run3", "H2_vs_V1_sub_pedestalA_norm", 0.70, 1.0, g_sum, g_att, h1);

    plot_Sum1DMap("run4", "H1_vs_V1_sub_pedestalA_norm", 0.70, 1.0, g_sum, g_att, h1);
    plot_Sum1DMap("run3", "H1_vs_V1_sub_pedestalA_norm", 0.70, 1.0, g_sum, g_att, h1);

    std::cout << "Collected "
              << g_att.size() << " g_att graphs, "
              << g_sum.size() << " g_sum graphs, "
              << h1.size() << " h1 histograms."
              << std::endl;

    const char *BoardName[4] = {"V2", "V1", "H2", "H1"};
    int colors[] = {kBlue, kRed, kGreen + 2, kMagenta, kOrange + 1};
    int markers[] = {20, 21, 22, 23, 24};

    for (int i = 0; i < 8; ++i)
    {

        g_att[i]->SetLineColor(colors[i / 2]);
        g_att[i]->SetMarkerColor(colors[i / 2]);
        g_att[i]->SetMarkerSize(i % 2 ? 0.2 : 0.4);
        g_att[i]->SetLineStyle(i % 2 ? 2 : 1);

        g_sum[i]->SetLineColor(colors[i / 2]);
        g_sum[i]->SetMarkerColor(colors[i / 2]);
        g_sum[i]->SetMarkerSize(i % 2 ? 0.2 : 0.4);
        g_sum[i]->SetLineStyle(i % 2 ? 2 : 1);
    }

    // ************ FIT attenuation *******************
    TF1 *f_att[8];

    // single exponential fit
    // for (int i = 0; i < 8; ++i)
    // {
    //     f_att[i] = new TF1(
    //         Form("f_att_%d", i),
    //         "exp(-x/[0])",
    //         g_att[i]->GetXaxis()->GetXmin(),
    //         g_att[i]->GetXaxis()->GetXmax());

    //     // initial parameters: very important for double exponential
    //     f_att[i]->SetParameter(0, 500.0);
    //     f_att[i]->SetParNames("lambda");

    //     // optional: avoid crazy negative decay lengths
    //     f_att[i]->SetParLimits(0, 40, 40000);
    //     g_att[i]->Fit(f_att[i], "R"); // R = fit range, Q = quiet
    // }

    // for H1,H2 single exponential fit; for V1,V2 exponential with reflection term

    for (int i = 0; i < 4; ++i)
    {
        f_att[i] = new TF1(
            Form("f_att_%d", i),
            "exp(-x/[0])",
            g_att[i]->GetXaxis()->GetXmin(),
            g_att[i]->GetXaxis()->GetXmax());

        // initial parameters: very important for double exponential
        f_att[i]->SetParameter(0, 500.0);
        f_att[i]->SetParNames("lambda");

        // optional: avoid crazy negative decay lengths
        f_att[i]->SetParLimits(0, 40, 40000);
        g_att[i]->Fit(f_att[i], "R"); // R = fit range, Q = quiet
    }

    for (int i = 4; i < 8; ++i)
    {
        f_att[i] = new TF1(
            Form("f_att_%d", i),
            "(exp(-x/[0]) + [1]*exp(-x/[0]))/(1 + [1])",
            g_att[i]->GetXaxis()->GetXmin(),
            g_att[i]->GetXaxis()->GetXmax());

        // initial parameters: very important for double exponential
        f_att[i]->SetParameters(500, 0.5);

        f_att[i]->SetParNames("lambda", "reflection");

        // optional: avoid crazy negative decay lengths
        f_att[i]->SetParLimits(1, 0.0, 1.0);
        f_att[i]->SetParLimits(0, 1.0, 4000.0);

        g_att[i]->Fit(f_att[i], "R"); // R = fit range, Q = quiet
    }

    // exponential with reflection term

    // for (int i = 0; i < 8; ++i)
    // {
    //     f_att[i] = new TF1(
    //         Form("f_att_%d", i),
    //         "(exp(-x/[0]) + [1]*exp(-x/[0]))/(1 + [1])",
    //         g_att[i]->GetXaxis()->GetXmin(),
    //         g_att[i]->GetXaxis()->GetXmax());

    //     // initial parameters: very important for double exponential
    //     f_att[i]->SetParameters(500, 0.5);

    //     f_att[i]->SetParNames( "lambda", "reflection");

    //     // optional: avoid crazy negative decay lengths
    //     f_att[i]->SetParLimits(1, 0.0, 1.0);
    //     f_att[i]->SetParLimits(0, 1.0, 4000.0);

    //     g_att[i]->Fit(f_att[i], "R"); // R = fit range, Q = quiet
    // }

    // double exponential fit

    // for (int i = 0; i < 8; ++i)
    // {
    //     f_att[i] = new TF1(
    //         Form("f_att_%d", i),
    //         "[1]*exp(-x/[0]) +(1-[1])*exp(-x/[2])",
    //         g_att[i]->GetXaxis()->GetXmin(),
    //         g_att[i]->GetXaxis()->GetXmax());

    //     // initial parameters: very important for double exponential
    //     f_att[i]->SetParameters(900.0, 1.0, 4000.0);

    //     f_att[i]->SetParNames("lambda","amp","lambda_L");

    //     // optional: avoid crazy negative decay lengths
    //     f_att[i]->SetParLimits(1, 0.5, 1.0);
    //     f_att[i]->SetParLimits(0, 100.0, 2000.0);
    //     f_att[i]->SetParLimits(2, 2000.0, 10000.0);

    //     g_att[i]->Fit(f_att[i], "R"); // R = fit range, Q = quiet
    // }

    // draw the first 4 g_att and last 4 g_sum on the same canvas
    TCanvas *c5 = new TCanvas("c5", "c5", 800, 800);

    // draw sum
    g_sum[0]->GetYaxis()->SetRangeUser(0.8, 1.2);
    g_sum[0]->Draw("APL");
    for (int i = 1; i < 8; ++i)
    {
        g_sum[i]->Draw("PL same");
    }
    c5->SaveAs("output2025/plots/Sum1DMap_XY_overlay1.png");

    // draw att
    g_att[0]->GetYaxis()->SetRangeUser(0.55, 1.2);
    g_att[0]->Draw("APL");
    f_att[0]->Draw("same");

    for (int i = 1; i < 8; ++i)
    {
        g_att[i]->Draw("PL same");
        f_att[i]->Draw("same");
    }

    // add legend
    TLegend *leg = new TLegend(0.35, 0.70, 0.70, 0.85);
    leg->SetFillStyle(0);  // transparent background
    leg->SetFillColor(0);  // no fill color
    leg->SetBorderSize(0); // no border
    for (int i = 0; i < 4; ++i)
    {
        double lambda = f_att[i * 2]->GetParameter(0);
        // double reflection = f_att[i * 2]->GetParameter(1);

        leg->AddEntry(
            g_att[i * 2],
            Form("%s, %.1f, %.1f", BoardName[i], lambda, f_att[i * 2 + 1]->GetParameter(0)),
            "PL");
        // leg->AddEntry(g_att[i * 2], Form(BoardName[i], i), "PL");
    }
    leg->Draw();
    c5->SetLogy();
    c5->SaveAs("output2025/plots/Sum1DMap_XY_overlay.png");

    delete c5;
}