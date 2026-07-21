#include "get_plot.h"
#include "lhcbStyle.h"
#include "RunContext_setting.h"

bool get_Ystats_at_X_clean(TH2D* h2, double x, double& mean, double& std)
{
    mean = 0.0;
    std  = 0.0;

    if (!h2) return false;

    int ix = h2->GetXaxis()->FindBin(x + 1e-6);
    int ny = h2->GetNbinsY();

    double sum_w  = 0.0;
    double sum_y  = 0.0;
    double sum_y2 = 0.0;

    for (int iy = 1; iy <= ny; ++iy)
    {
        double content = h2->GetBinContent(ix, iy);

        //  your condition: remove low-stat bins
        if (content < 4) continue;

        double y = h2->GetYaxis()->GetBinCenter(iy);

        sum_w  += content;
        sum_y  += content * y;
        sum_y2 += content * y * y;
    }

    if (sum_w < 10) return false; // avoid unstable result

    mean = sum_y / sum_w;
    std  = std::sqrt(sum_y2 / sum_w - mean * mean);

    return true;
}

void plot_HistSpill_TGraph(const char *run_name, const char *plot_name)
{
    const char *module_name = "HistSpill";
    const char *file_name = Form("output2025/%s_%s.root", run_name, module_name);

    auto *PosDiff = get_plot<TH2D>(file_name, Form("%s/%s", module_name, "PosDiffVV_V2_V1"));

    std::vector<double> snr_mean;
    std::vector<double> pos_res;

}


void plot_all_HistSpill()
{
    lhcbStyle();

    plot_HistSpill_TGraph("run3", "H_PosBias1D_H2_H1", -0.4, 0.4, 0.14);
    plot_HistSpill_TGraph("run3", "V_PosBias1D_V2_V1", -0.4, 0.4, 0.14);
    plot_HistSpill_TGraph("run3", "H_PosRes1D_H2_H1", 0.02, 0.08, 0.13);
    plot_HistSpill_TGraph("run3", "V_PosRes1D_V2_V1", 0.02, 0.08, 0.13);

    plot_HistSpill_TGraph("run4", "H_PosBias1D_H2_H1", -0.4, 0.4, 0.14);
    plot_HistSpill_TGraph("run4", "V_PosBias1D_V2_V1", -0.4, 0.4, 0.14);
    plot_HistSpill_TGraph("run4", "H_PosRes1D_H2_H1", 0.02, 0.08, 0.13);
    plot_HistSpill_TGraph("run4", "V_PosRes1D_V2_V1", 0.02, 0.08, 0.13);

    plot_1Dhist_donot_norm("run4", "HistSpill", "hz_abnormal_-55_-31_0", -1, 1, 6);

    std::vector<double> detector_edges_X = {63, 64, 127, 128, 191, 192, 255, 256, 68, 218};
    std::vector<double> detector_edges_Y = {63, 64, 127, 128, 191, 192, 255, 256, 53, 261};

    // sort
    // std::sort(detector_edges_X.begin(), detector_edges_X.end());
    // std::sort(detector_edges_Y.begin(), detector_edges_Y.end());

    convert_channelID_to_mm(detector_edges_X);
    convert_channelID_to_mm(detector_edges_Y);

    plot_HistSpill_2D("run3", "H_PosBias_H2_H1", -0.4, 0.4, detector_edges_X, // x marks on top
                     detector_edges_Y);
    plot_HistSpill_2D("run3", "V_PosBias_V2_V1", -0.4, 0.4, detector_edges_X, // x marks on top
                     detector_edges_Y);
    plot_HistSpill_2D("run3", "H_PosRes_H2_H1", 0.02, 0.08, detector_edges_X, // x marks on top
                     detector_edges_Y);
    plot_HistSpill_2D("run3", "V_PosRes_V2_V1", 0.02, 0.08, detector_edges_X, // x marks on top
                     detector_edges_Y);

    plot_HistSpill_2D("run4", "H_PosBias_H2_H1", -0.4, 0.4, detector_edges_X, // x marks on top
                     detector_edges_Y);
    plot_HistSpill_2D("run4", "V_PosBias_V2_V1", -0.4, 0.4, detector_edges_X, // x marks on top
                     detector_edges_Y);
    plot_HistSpill_2D("run4", "H_PosRes_H2_H1", 0.02, 0.08, detector_edges_X, // x marks on top
                     detector_edges_Y);
    plot_HistSpill_2D("run4", "V_PosRes_V2_V1", 0.02, 0.08, detector_edges_X, // x marks on top
                     detector_edges_Y);
}