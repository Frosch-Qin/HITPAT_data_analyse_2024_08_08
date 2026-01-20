// output the calibration factor to txt files; one file for one board; each line for one channel
// use line scan
// output sumsignal
// output sumsignal_diff
// output calibration_factor
// output calibration_factor_quantized
// output sumsignal_calibrated
// output sumsignal_calibrated with sumsignal and boundary
// input is run name and the boundary of the beam signal(how many sigma from the fit for sumsignal_diff to the center)
//  if no boundary is given, the default value is 1.5

#include "../hitreader/analyser.h"

double *mean_std(TGraph *g, double left_border, double right_border)
{
    double *results = new double[4]; // mean, std, min, max

    double min = 65535;
    double max = -1;

    TH1D *h = new TH1D("h", "h", 5000, 0, 100);
    for (int i = 0; i < g->GetN(); i++)
    {
        double x, y;
        g->GetPoint(i, x, y);
        if (x < left_border || x > right_border)
            continue;
        h->Fill(y);
        if (y < min && y > 0)
            min = y;
        if (y > max)
            max = y;
    }
    results[0] = h->GetMean();
    results[1] = h->GetStdDev();
    results[2] = min;
    results[3] = max;
    delete h;
    return results;
}

int cal_board(const char *run_name, int boardID, double left_nr_sigma = 1.5, double right_nr_sigma = 1.5)
{
    lhcbStyle();
    double boundary_nr_sigma[2] = {left_nr_sigma, right_nr_sigma};

    ofstream logfile;
    logfile.open(Form("calibration_factor_linescanH/%s_cal_board%d.log", run_name, boardID));
    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);

    /*****************open and read the roof file************************************************************/
    string rootFileName = Form("../sum_signal1d/rootfile/%s_4_sum1d.root", run_name);
    TFile *rootFile = TFile::Open(rootFileName.c_str(), "READ");

    if (rootFile->IsOpen())
    {
        std::cout << Form(" %s opened sucessfully", rootFileName.c_str()) << endl;
    }
    else
    {
        std::cout << Form("%s failed to open", rootFileName.c_str()) << endl;
        return 1;
    }

    TGraph *sumsignal_graph = (TGraph *)gDirectory->Get(Form("sum_all_pedestal0_sub_%d", boardID));
    sumsignal_graph->GetXaxis()->SetRangeUser(0, 319);
    sumsignal_graph->GetXaxis()->SetTitle("Channel ID");
    sumsignal_graph->GetYaxis()->SetTitle("avg amplitude");

    TGraph *pedestal_uncertainty_graph = (TGraph *)gDirectory->Get(Form("pedestal1_sub_pedestal0_%d", boardID));

    /*************************************for sum signal************************************************/
    // get the sumsignal
    int nrChannels = sumsignal_graph->GetN();
    double sumsignal[nrChannels]{};
    double max_sumsignal{};

    for (int j = 0; j < nrChannels; j++)
    {
        sumsignal[j] = sumsignal_graph->GetY()[j];

        if (sumsignal[j] > max_sumsignal)
        {
            max_sumsignal = sumsignal[j];
        }
    }

    // Calculate the sumsignal difference between neighboring channels
    double sumsignal_diff[nrChannels - 2] = {};
    double sumsignal_diff_reversed[nrChannels - 2] = {};

    for (int j = 0; j < nrChannels - 2; j++)
    {
        if (sumsignal[j + 2] == 0 || sumsignal[j] == 0)
        {
            sumsignal_diff[j] = 0;
            sumsignal_diff_reversed[j] = 0;
        }
        else
        {
            sumsignal_diff[j] = (sumsignal[j + 2] - sumsignal[j]) / 2;
            sumsignal_diff_reversed[j] = (sumsignal[j] - sumsignal[j + 2]) / 2;
        }
    }

    // Create the fit functions
    TF1 *fleft = new TF1("fleft", "gaus", 27, 66);
    TF1 *fright = new TF1("fright", "gaus", 250, 280);
    TF1 *fright_reversed = new TF1("fright_reversed", "gaus", 220, 319);

    // Create the canvases and graphs for sumsignal_diff
    TGraph *sumsignal_diff_graph;
    TGraph *sumsignal_diff_reversed_graph;

    sumsignal_diff_graph = new TGraph(nrChannels - 2);
    sumsignal_diff_reversed_graph = new TGraph(nrChannels - 2);
    for (int j = 0; j < nrChannels - 2; j++)
    {
        sumsignal_diff_graph->SetPoint(j, j + 1, sumsignal_diff[j]);
        sumsignal_diff_reversed_graph->SetPoint(j, j + 1, sumsignal_diff_reversed[j]);
    }
    sumsignal_diff_graph->SetName(Form("sumsignal_diff_%d", boardID));
    sumsignal_diff_graph->SetTitle(Form("sumsignal_diff_%d", boardID));
    sumsignal_diff_graph->GetXaxis()->SetTitle("Channel ID");
    sumsignal_diff_graph->GetXaxis()->SetRangeUser(1, 318);
    sumsignal_diff_graph->GetYaxis()->SetTitle("sumsignal_diff");
    sumsignal_diff_graph->Draw("APL");
    sumsignal_diff_reversed_graph->SetName(Form("sumsignal_diff_reversed_%d", boardID));
    sumsignal_diff_reversed_graph->SetTitle(Form("sumsignal_diff_reversed_%d", boardID));
    sumsignal_diff_reversed_graph->GetXaxis()->SetTitle("Channel ID");

    sumsignal_diff_graph->Fit("fleft", "QR");
    logfile << "board " << boardID << " boundary left mean: " << fleft->GetParameter(1) << " left sigma: " << fleft->GetParameter(2) << std::endl;
    sumsignal_diff_reversed_graph->Fit("fright", "QR");
    logfile << "board " << boardID << " boundary right mean: " << fright->GetParameter(1) << " right sigma: " << fright->GetParameter(2) << std::endl;

    fleft->Draw("same");
    fright_reversed->SetParameter(0, -fright->GetParameter(0));
    fright_reversed->SetParameter(1, fright->GetParameter(1));
    fright_reversed->SetParameter(2, fright->GetParameter(2));
    fright_reversed->Draw("same");
    c1->SaveAs(Form("sumsignal_diff_%s_board%d.png", run_name, boardID));

    // get the boundary of the beam signal
    double beam_boundary[2];

    // get and output the beam boundary

    beam_boundary[0] = fleft->GetParameter(1) + boundary_nr_sigma[0] * fleft->GetParameter(2);
    beam_boundary[1] = fright->GetParameter(1) - boundary_nr_sigma[1] * fright->GetParameter(2);
    logfile << "board " << boardID << " left boundary_nr_sigma: " << boundary_nr_sigma[0] << " right boundary_nr_sigma: " << boundary_nr_sigma[1] << std::endl;
    logfile << "board " << boardID << " beam boundary: " << beam_boundary[0] << " " << beam_boundary[1] << std::endl;

    // plot the boundary on sumsignal graph

    sumsignal_graph->Draw("APL");

    // l1->Draw("same");
    // l2->Draw("same");
    // c1->SaveAs(Form("sumsignal_boud_%d.png", boardID));

    // calulate the average beam signal in side the boundary
    double beam_signal{};
    double beam_signal_ctr{};

    for (int j = 0; j < nrChannels; j++)
    {
        if (j >= beam_boundary[0] && j <= beam_boundary[1])
        {
            beam_signal += sumsignal[j];
            beam_signal_ctr++;
        }
    }
    beam_signal = beam_signal / beam_signal_ctr;
    logfile << "board " << boardID << " plateau hight : " << beam_signal << std::endl;

    // use the average beam signal divide the sumsignal for each channel inside the boundary to get calibration factor
    // the channel outside the bouldery will be set to 1
    double calibration_factor[nrChannels]{};

    for (int j = 0; j < nrChannels; j++)
    {
        if (j >= beam_boundary[0] && j <= beam_boundary[1])
        {

            if (sumsignal[j] == 0)
            {
                calibration_factor[j] = 1;
                std::cout << "board " << boardID << " channel " << j << " ####sumsignal is zero######" << std::endl;
            }
            else
            {
                calibration_factor[j] = beam_signal / sumsignal[j];
            }
        }
        else
        {
            calibration_factor[j] = 1;
        }

        if (calibration_factor[j] < 0)
        {
            calibration_factor[j] = 1;
            logfile << "board " << boardID << " channel " << j << " #### bad channel ######" << std::endl;
        }
    }

    // plot the calibration factor and save it to the root file
    TGraph *calibration_factor_graph;

    calibration_factor_graph = new TGraph(nrChannels);
    for (int j = 0; j < nrChannels; j++)
    {
        calibration_factor_graph->SetPoint(j, j, calibration_factor[j]);
    }
    calibration_factor_graph->SetName(Form("calibration_factor_%d", boardID));
    calibration_factor_graph->SetTitle(Form("calibration_factor_%d", boardID));
    calibration_factor_graph->GetXaxis()->SetTitle("Channel ID");
    calibration_factor_graph->GetXaxis()->SetRangeUser(0, 319);
    calibration_factor_graph->GetXaxis()->SetRangeUser(0, pow(2, 16) - 1);
    calibration_factor_graph->GetYaxis()->SetTitle("calibration_factor");
    calibration_factor_graph->Draw("APL");

    c1->SaveAs(Form("calibration_factor_%s_board%d.png", run_name, boardID));

    // quantized the calibration factor, scale 1 to 8192
    unsigned short calibration_factor_quantized[nrChannels]{};

    for (int j = 0; j < nrChannels; j++)
    {
        calibration_factor_quantized[j] = calibration_factor[j] * 8192;
        if (calibration_factor_quantized[j] > pow(2, 16) - 1)
        {
            calibration_factor_quantized[j] = pow(2, 16) - 1;
            logfile << "board " << boardID << " channel " << j << " ####calibration factor is larger than 2^16-1######" << std::endl;
        }
    }

    // plot the quantized calibration factor and save it to the root file
    TGraph *calibration_factor_quantized_graph;

    calibration_factor_quantized_graph = new TGraph(nrChannels);
    for (int j = 0; j < nrChannels; j++)
    {
        calibration_factor_quantized_graph->SetPoint(j, j, calibration_factor_quantized[j]);
    }
    calibration_factor_quantized_graph->SetName(Form("calibration_factor_quantized_%d", boardID));
    calibration_factor_quantized_graph->SetTitle(Form("calibration_factor_quantized_%d", boardID));
    calibration_factor_quantized_graph->GetXaxis()->SetTitle("Channel ID");
    calibration_factor_quantized_graph->GetXaxis()->SetRangeUser(0, 319);
    calibration_factor_quantized_graph->GetYaxis()->SetTitle("calibration_factor_quantized");
    calibration_factor_quantized_graph->Draw("APL");

    c1->SaveAs(Form("calibration_factor_quantized_%s_board%d.png", run_name, boardID));

    // output the quantized calibration factor to txt files; one file for one board; each line for one channel
    ofstream outfile;

    std::string filename = Form("calibration_factor_run19/board%d.txt", boardID);
    outfile.open(filename);
    if (!outfile)
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return 1;
    }
    for (int j = 0; j < nrChannels; j++)
    {
        outfile << calibration_factor_quantized[j] << endl;
    }
    outfile.close();

    // use the calibration factor to calibrate the sumsignal
    double sumsignal_calibrated[nrChannels]{};

    for (int j = 0; j < nrChannels; j++)
    {
        sumsignal_calibrated[j] = (1.0 * sumsignal[j] * calibration_factor_quantized[j] / 8192);
    }

    // plot the calibrated sumsignal with the sumsignal and boundary

    TGraph *sumsignal_calibrated_graph = new TGraph(nrChannels);
    for (int j = 0; j < nrChannels; j++)
    {
        sumsignal_calibrated_graph->SetPoint(j, j, sumsignal_calibrated[j]);
    }
    sumsignal_calibrated_graph->SetName(Form("sumsignal_calibrated_%d", boardID));
    sumsignal_calibrated_graph->SetTitle(Form("sumsignal_calibrated_%d", boardID));
    sumsignal_calibrated_graph->GetXaxis()->SetTitle("Channel ID");
    sumsignal_calibrated_graph->GetXaxis()->SetRangeUser(0, 319);
    sumsignal_calibrated_graph->GetYaxis()->SetTitle("sumsignal_calibrated");
    sumsignal_calibrated_graph->SetLineColor(kRed);

    TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1);
    pad1->SetTopMargin(0.1);
    pad1->SetBottomMargin(0);
    pad1->Draw();
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);
    pad2->Draw();

    pad1->cd();

    // draw the sumsignal and boundary
    // sumsignal_graph->GetYaxis()->SetRangeUser(-1, 20);
    sumsignal_graph->GetYaxis()->SetTitleOffset(1.0); // try between 1.0–1.6
    sumsignal_graph->Draw("APL");

    sumsignal_calibrated_graph->Draw("PL same");

    // get the std of the pedestal and the sumsignal between the beam boundary

    double *results_sum = mean_std(sumsignal_graph, beam_boundary[0], beam_boundary[1]);
    double mean_sum = results_sum[0];
    double std_sum = results_sum[1];
    double min_sum = results_sum[2];
    double max_sum = results_sum[3];
    delete[] results_sum;
    double *results_pedestal = mean_std(pedestal_uncertainty_graph, beam_boundary[0], beam_boundary[1]);
    double mean_pedestal = results_pedestal[0];
    double std_pedestal = results_pedestal[1];
    double min_pedestal = results_pedestal[2];
    double max_pedestal = results_pedestal[3];
    delete[] results_pedestal;

    logfile << "board " << boardID << " pedestal uncertainty mean: " << mean_pedestal << " std: " << std_pedestal << std::endl;
    logfile << "board " << boardID << " sumsignal mean: " << mean_sum << " std: " << std_sum << std::endl;
    logfile << " mean uncertainty for calibration factor: " << std_pedestal / mean_sum * 100 << " percent with mean " << mean_sum << std::endl;
    logfile << " worst uncertainty for calibration factor: " << std_pedestal / min_sum * 100 << " percent with min  " << min_sum << std::endl;
    logfile << " best uncertainty for calibration factor: " << std_pedestal / max_sum * 100 << " percent with max " << max_sum<< std::endl;

    TLine *l1 = new TLine(beam_boundary[0], 0, beam_boundary[0], mean_sum);
    TLine *l2 = new TLine(beam_boundary[1], 0, beam_boundary[1], mean_sum);
    l1->SetLineColor(kRed);
    l2->SetLineColor(kRed);
    l1->Draw("same");
    l2->Draw("same");
    TLine *line1 = new TLine(beam_boundary[0], mean_sum, beam_boundary[1], mean_sum);
    line1->SetLineColor(kRed);
    // line1->SetLineStyle(2);
    line1->Draw("same");

    pad2->cd();
    pedestal_uncertainty_graph->GetXaxis()->SetRangeUser(0, 319);
    pedestal_uncertainty_graph->GetYaxis()->SetRangeUser(-0.2, 0.2);
    pedestal_uncertainty_graph->GetXaxis()->SetTitle("Channel ID");
    pedestal_uncertainty_graph->GetYaxis()->SetTitle("avg amplitude B");
    pedestal_uncertainty_graph->GetXaxis()->SetLabelSize(0.1);
    pedestal_uncertainty_graph->GetXaxis()->SetTitleSize(0.1);
    pedestal_uncertainty_graph->GetYaxis()->SetLabelSize(0.1);
    pedestal_uncertainty_graph->GetYaxis()->SetTitleSize(0.1);
    pedestal_uncertainty_graph->GetYaxis()->SetNdivisions(505);
    pedestal_uncertainty_graph->GetYaxis()->SetTitleOffset(0.5); // try between 1.0–1.6

    // pedestal_uncertainty_graph->GetYaxis()->SetTitle("Group B");
    pedestal_uncertainty_graph->Draw("APL");
    TLine *ll1 = new TLine(beam_boundary[0], -0.2, beam_boundary[0], 0.2);
    TLine *ll2 = new TLine(beam_boundary[1], -0.2, beam_boundary[1], 0.2);
    ll1->SetLineColor(kRed);
    ll2->SetLineColor(kRed);
    ll1->Draw("same");
    ll2->Draw("same");

    c1->SaveAs(Form("sumsignal_calibrated_%s_board%d.png", run_name, boardID));
    c1->SaveAs(Form("../plot/2024_08_measure/sumsignal_%s_board%d.png", run_name, boardID));

    //*********Draw the sum with the pleatue only************ */
    TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);
    sumsignal_graph->GetYaxis()->SetRangeUser(0, 25);
    sumsignal_graph->SetLineWidth(2);
    sumsignal_graph->Draw("APL");
    l1->Draw("same");
    l2->Draw("same");
    line1->Draw("same");
    c2->SaveAs(Form("sumsignal_uncal_%s_board%d.png", run_name, boardID));
    // c2->SaveAs(Form("../plot/2024_08_measure/sumsignal_uncal_%s_board%d.png", run_name, boardID));

    pedestal_uncertainty_graph->SetLineWidth(2);
    pedestal_uncertainty_graph->GetXaxis()->SetLabelSize(gStyle->GetLabelSize("X"));
    pedestal_uncertainty_graph->GetYaxis()->SetLabelSize(gStyle->GetLabelSize("Y"));
    pedestal_uncertainty_graph->GetXaxis()->SetTitleSize(gStyle->GetTitleSize("X"));
    pedestal_uncertainty_graph->GetYaxis()->SetTitleSize(gStyle->GetTitleSize("Y"));
    pedestal_uncertainty_graph->GetYaxis()->SetTitleOffset(gStyle->GetTitleOffset("Y")); 

    pedestal_uncertainty_graph->Draw("APL");


    ll1->Draw("same");
    ll2->Draw("same");
    c2->SaveAs(Form("pedestal_uncertainty_%s_board%d.png", run_name, boardID));
    // c2->SaveAs(Form("../plot/2024_08_measure/pedestal_uncertainty_%s_board%d.png", run_name, boardID));

    // output the quartise calibration factor to txt files; one file for one board; each line for one channel
    ofstream calfile;

    std::string calfile_name = Form("calibration_factor_run19/board%d.txt", boardID);
    calfile.open(calfile_name);
    if (!calfile)
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return 1;
    }
    for (int j = 0; j < nrChannels; j++)
    {
        calfile << calibration_factor_quantized[j] << endl;
    }
    calfile.close();

    // Clean up fit functions
    delete fleft;
    delete fright;

    return 0;
}
