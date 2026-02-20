// SpillIDAssigner.h
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class SpillIDAssigner : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "SpillIDAssigner";
    }

    void begin_run(const RunContext &ctx) override
    {
        file_ = new TFile(Form("output/SpillID/run%d_SpillID.root", ctx.run_number), "RECREATE");
        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }

        readout_rate = ctx.readout_rate;

        createHistograms(ctx.max_frames / ctx.readout_rate);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {

        if (tags.BKG_SUB_ON)
        {

            fillHistograms(tags.boardTags, frame_index / readout_rate);
        }
    }
    void end_run(const RunContext &ctx) override
    {
        writeHistograms();
    }

private:
    void fillHistograms(beamRecon CPUrecon[6], double eventTime)
    {

        for (int i = 0; i < 6; i++)
        {
            ClusterTimeHistogram[i]->Fill(eventTime, CPUrecon[i].Cluster_num);
            if (CPUrecon[i].Cluster_num > 0)
            {
                ClusterSizeHistogram[i]->Fill(CPUrecon[i].Windowright - CPUrecon[i].Windowleft);
            }
        }
    }

    void get_spill_info()
    {

        // the primay spill info
        std::vector<double> spill_start_time0;
        std::vector<double> spill_end_time0;

        for (int i = 1; i < BeamOnTimeGraph_12->GetN(); i++)
        {
            double x, y;
            double x_last, y_last;
            BeamOnTimeGraph_12->GetPoint(i, x, y);
            BeamOnTimeGraph_12->GetPoint(i - 1, x_last, y_last);
            if (y > 0 && y_last == 0)
            {
                spill_start_time0.push_back(x);
                // std::cout << "spill start time: " << x << std::endl;
            }
            if (y == 0 && y_last > 0)
            {
                spill_end_time0.push_back(x);
                // std::cout << "spill end time: " << x << std::endl;
            }
        }

        // if spill_end_time0 - spill_start_time0 is too short, and the next spill_start_time0 is too close, then we merge them
        for (size_t i = 0; i < spill_start_time0.size() - 1; i++)
        {
            if (spill_start_time0[i + 1] - spill_end_time0[i] < 0.8) // two spills must space more than 0.8 seconds!!!! reasonalble accelerator assumption
            {
                spill_end_time0.erase(spill_end_time0.begin() + i);
                spill_start_time0.erase(spill_start_time0.begin() + i + 1);
                std::cout << "merge spill " << i << " and " << i + 1 << std::endl;
                i--;
            }
        }

        // assign the final values to the class variables
        spill_start_time = spill_start_time0;
        spill_end_time = spill_end_time0;

        // draw the spill time graph
        for (int i = 0; i < time_bins; i++)
        {
            double x = ClusterTimeHistogram[0]->GetXaxis()->GetBinCenter(i + 1);
            double y = 0;
            for (size_t j = 0; j < spill_start_time.size(); j++)
            {
                if (x >= spill_start_time[j] && x <= spill_end_time[j])
                {
                    y = 1; // spill on
                    break;
                }
            }
            spill_time_graph->SetPoint(i, x, y);
        }
    }

    void fillTree()
    {
        file_->cd();
        // create a tree to store the spill time
        tree = new TTree("spill_info_tree", "spill_info_tree");

        double spill_start;
        double spill_end;

        get_spill_info();

        tree->Branch("spill_start", &spill_start, "spill_start/D");
        tree->Branch("spill_end", &spill_end, "spill_end/D");
        for (size_t i = 0; i < spill_start_time.size(); i++)
        {
            spill_start = spill_start_time[i];
            spill_end = spill_end_time[i];
            std::cout << " spill " << i << " start time: " << spill_start << ", end time: " << spill_end << " time last " << spill_end - spill_start << std::endl;
            tree->Fill();
        }

        tree->Write();
        file_->cd();
    }

    void writeHistograms()
    {
        file_->cd();

        ClusterTimeDir->cd();
        for (auto &hist : ClusterTimeHistogram)
            hist->Write();

        ClusterSizeDir->cd();
        for (auto &hist : ClusterSizeHistogram)
            hist->Write();
        BeamOnTimeDir->cd();
        for (int i = 0; i < 6; i++)
        {
            BeamOnTimeGraph[i]->SetMarkerStyle(20);
            BeamOnTimeGraph[i]->SetMarkerSize(0.5);
            BeamOnTimeGraph[i]->SetMarkerColor(2);
            BeamOnTimeGraph[i]->SetLineColor(2);
            // get beamon
            for (int j = 0; j < time_bins; j++)
            {
                double y = 0;
                if (ClusterTimeHistogram[i]->GetBinContent(j + 1, 2) >= 2) // for board1 and 2 rho~0; the chance get a false positive is very low...
                {
                    y = 2; // beam on
                }
                // else if (ClusterTimeHistogram[i]->GetBinContent(j + 1, 2) >= 2)
                // {
                //     y = 1; // beam edge
                // }
                else
                {
                    y = 0; // beam off
                }
                BeamOnTimeGraph[i]->SetPoint(j, ClusterTimeHistogram[i]->GetXaxis()->GetBinCenter(j + 1), y);
            }
            BeamOnTimeGraph[i]->Write();
        }

        // for BeamOnTimeGraph_12
        for (int j = 0; j < time_bins; j++)
        {
            double y = 0;
            if (ClusterTimeHistogram[1]->GetBinContent(j + 1, 2) >= 2 || ClusterTimeHistogram[2]->GetBinContent(j + 1, 2) >= 2)
            {
                y = 2; // beam on
            }
            // else if (ClusterTimeHistogram[1]->GetBinContent(j + 1, 2) >= 2 || ClusterTimeHistogram[2]->GetBinContent(j + 1, 2) >= 2)
            // {
            //     y = 1; // beam edge
            // }
            else
            {
                y = 0; // beam off
            }
            BeamOnTimeGraph_12->SetPoint(j, ClusterTimeHistogram[1]->GetXaxis()->GetBinCenter(j + 1), y);
        }
        BeamOnTimeGraph_12->Write();
        file_->cd();
        fillTree();
        spill_time_graph->Write();
        file_->Close();
    }

    void createHistograms(double totaltime) // totaltime in seconds for the focusTime histogram
    {
        // create a directory for the histogram
        time_bins = int(totaltime * 100); // 100 bins for 1 second
        ClusterTimeDir = file_->mkdir("ClusterTimeDir");
        ClusterTimeDir->cd();
        for (int i = 0; i < 6; i++)
        {
            ClusterTimeHistogram[i] = new TH2D(Form("ClusterTimeHistogram_%d", i), Form("ClusterTimeHistogram_%d", i), time_bins, 0, totaltime, 2, -0.5, 1.5);
        }
        ClusterTimeDir->cd(".."); // Go back to the main directory

        ClusterSizeDir = file_->mkdir("ClusterSizeDir");
        ClusterSizeDir->cd();
        for (int i = 0; i < 6; i++)
        {
            ClusterSizeHistogram[i] = new TH1D(Form("ClusterSizeHistogram_%d", i), Form("ClusterSizeHistogram_%d", i), 320, -0.5, 319.5);
        }
        ClusterSizeDir->cd(".."); // Go back to the main directory

        BeamOnTimeDir = file_->mkdir("BeamOnTimeDir");
        BeamOnTimeDir->cd();
        for (int i = 0; i < 6; i++)
        {
            BeamOnTimeGraph[i] = new TGraph(time_bins);
            BeamOnTimeGraph[i]->SetName(Form("BeamOnTimeGraph_%d", i));
        }
        BeamOnTimeGraph_12 = new TGraph(time_bins);
        BeamOnTimeGraph_12->SetName("BeamOnTimeGraph_12");
        BeamOnTimeDir->cd(".."); // Go back to the main directory

        spill_time_graph = new TGraph(time_bins);
        spill_time_graph->SetName("spill_time_graph");
        spill_time_graph->SetTitle("spill_time_graph");
        spill_time_graph->GetXaxis()->SetTitle("Time (s)");
        spill_time_graph->GetYaxis()->SetTitle("Spill Status");
    }

    int nrBoards = 6;

    TFile *file_;
    TTree *tree;

    TDirectory *ClusterTimeDir;
    TDirectory *ClusterSizeDir;
    TDirectory *BeamOnTimeDir;

    // TDirectory *BeamNotDetectTimeDir;

    TH2D *ClusterTimeHistogram[6];
    TH1D *ClusterSizeHistogram[6];
    TGraph *BeamOnTimeGraph[6];
    TGraph *BeamOnTimeGraph_12;
    int time_bins;

    std::vector<double> spill_start_time;
    std::vector<double> spill_end_time;

    TGraph *spill_time_graph;

    double readout_rate;
};
