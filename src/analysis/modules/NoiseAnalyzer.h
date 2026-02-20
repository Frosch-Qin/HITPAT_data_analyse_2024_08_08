// NoiseAnalyzer.h
#pragma once

#include "../AnalysisPipeline.h"

class NoiseAnalyzer final : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "NoiseAnalyzer";
    }

    void begin_run(const RunContext &ctx) override
    {

        file_ = new TFile(Form("output/Noise/run%d_Noise.root", ctx.run_number), "RECREATE");
        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }

        is_noise_run = ctx.noise_run;
        if (is_noise_run)
        {
            std::cout << "this is a noise run " << std::endl;
        }

        createHistograms(ctx);
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {

        // if is noise only, fill the hist
        if (tags.BKG_SUB_ON)
        {
            for (int i = 0; i < nrBoards; ++i)
            {

                if (is_noise_run || (tags.SpillID == -2 || tags.SpillID != -1))  //noise run or between spills
                {
                    double mean_first_4_channel = get_avg(frame.boards[i].data, 4);
                    double mean_all = get_avg(frame.boards[i].data, frame.boards[i].nrChannels);
                    double stddev_all = get_standard_deviation(frame.boards[i].data, frame.boards[i].nrChannels, mean_all);

                    for (int j = 0; j < frame.boards[i].nrChannels; ++j)
                    {

                        if (frame.boards[i].data != 0)
                        {
                            noise_all[i]->Fill(frame.boards[i].data[j]);
                            noise_all_sub_cc[i]->Fill(frame.boards[i].data[j] - mean_first_4_channel);

                            noise_channel[i]->Fill(j, frame.boards[i].data[j]);
                            noise_channel_sub_cc[i]->Fill(j, frame.boards[i].data[j] - mean_first_4_channel);
                        }
                        else
                        {

                            noise_channel[i]->Fill(j, 0);
                            noise_channel_sub_cc[i]->Fill(j, 0);
                        }
                    }
                    noise_frame_mean[i]->Fill(mean_all);
                    noise_frame_std_hist[i]->Fill(stddev_all);

                    noise_correlation[i]->Fill(frame.boards[i].data[318], frame.boards[i].data[319]);
                }
            }
        }
    }
    // ~NoiseAnalyzer()
    // {
    //     for (auto hist : noise_total_hist)
    //     {
    //         delete hist;
    //     }
    // } //should not deconstruct the hist, because rootfile needs to save them later

    void end_run(const RunContext &ctx) override
    {
        file_->cd();
        if (!dir_)
            return;
        dir_->cd();
        for (int i = 0; i < nrBoards; ++i)
        {
            noise_all[i]->Scale(1.0 / noise_all[i]->Integral());
            noise_all[i]->GetXaxis()->SetTitle("noise (ADC)");
            noise_all[i]->GetYaxis()->SetTitle("Fraction");
            noise_frame_mean[i]->Scale(1.0 / noise_frame_mean[i]->Integral());
            noise_frame_mean[i]->GetXaxis()->SetTitle("noise (ADC)");
            noise_frame_mean[i]->GetYaxis()->SetTitle("Fraction");
            noise_all_sub_cc[i]->Scale(1.0 / noise_all_sub_cc[i]->Integral());
            noise_all_sub_cc[i]->GetXaxis()->SetTitle("noise (ADC)");
            noise_all_sub_cc[i]->GetYaxis()->SetTitle("Fraction");
            noise_channel[i]->Scale(1.0);
            noise_channel[i]->GetXaxis()->SetTitle("Channel ID");
            noise_channel[i]->GetYaxis()->SetTitle("Noise (ADC)");
            noise_channel_sub_cc[i]->Scale(1.0);
            noise_channel_sub_cc[i]->GetXaxis()->SetTitle("Channel ID");
            noise_channel_sub_cc[i]->GetYaxis()->SetTitle("Noise (ADC)");
            noise_frame_std_hist[i]->Scale(1.0 / noise_frame_std_hist[i]->Integral());
            noise_frame_std_hist[i]->GetXaxis()->SetTitle("Noise frame std (ADC)");
            noise_frame_std_hist[i]->GetYaxis()->SetTitle("Fraction");

            noise_correlation[i]->GetXaxis()->SetTitle("Channel 318");
            noise_correlation[i]->GetYaxis()->SetTitle("Channel 319");

            noise_all[i]->Write();
            noise_frame_mean[i]->Write();
            noise_all_sub_cc[i]->Write();
            noise_channel[i]->Write();
            noise_channel_sub_cc[i]->Write();
            noise_frame_std_hist[i]->Write();
            noise_correlation[i]->Write();
        }
        file_->Close();
    }

private:
    int nrBoards = 6;

    TFile *file_ = nullptr;
    TDirectory *dir_ = nullptr;
    bool is_noise_run = false;

    TH2D *noise_channel[6];
    TH2D *noise_channel_sub_cc[6]; // after common mode subtraction

    TH2D *noise_correlation[6];    // correlation between channels
    TH1D *noise_all[6];            // all the channels
    TH1D *noise_frame_mean[6];     // the common mode noise， the avg of all the channels
    TH1D *noise_all_sub_cc[6];     // the uncommon mode
    TH1D *noise_frame_std_hist[6]; // noise frame

    void createHistograms(const RunContext &ctx)
    {

        if (!file_)
            return;
        dir_ = file_->GetDirectory("NoiseAnalysis");
        if (!dir_)
            dir_ = file_->mkdir("NoiseAnalysis");
        dir_->cd();

        for (int i = 0; i < nrBoards; ++i)
        {
            noise_all[i] = new TH1D(Form("noise_total_%d", i), Form("noise_total_%d", i), 201, -100.5, 100.5);
            noise_frame_mean[i] = new TH1D(Form("noise_baseline_%d", i), Form("noise_baseline_%d", i), 201, -100.5, 100.5);
            noise_all_sub_cc[i] = new TH1D(Form("noise_after_cc_%d", i), Form("noise_after_cc_%d", i), 201, -100.5, 100.5);
            noise_frame_std_hist[i] = new TH1D(Form("noise_frame_std_%d", i), Form("noise_frame_std_%d", i), 100, 0, 20); // standard deviation per frame
            noise_channel[i] = new TH2D(Form("noise_channel_%d", i), Form("noise_channel_%d", i), 321, -0.5, 320.5, 201, -100.5, 100.5);
            noise_channel_sub_cc[i] = new TH2D(Form("noise_channel_sub_cc_%d", i), Form("noise_channel_sub_cc_%d", i), 321, -0.5, 320.5, 201, -100.5, 100.5);
            noise_correlation[i] = new TH2D(Form("noise_correlation_%d", i), Form("noise_correlation_%d", i), 201, -100.5, 100.5, 201, -100.5, 100.5);
        }
    }

    double get_avg(double *v, int n)
    {
        double return_value = 0.0;

        int dead_channels = 0;

        for (int i = 0; i < n; i++)
        {
            if (v[i] == 0)
            {
                dead_channels++;
                continue;
            }
            return_value += v[i];
        }
        return (return_value / double(n - dead_channels));
    }
    //****************End of average funtion****************

    // Function for variance
    double get_standard_deviation(double *v, int n, double mean)
    {
        double sum = 0.0;
        double temp = 0.0;
        double std = 0.0;

        int dead_channels = 0;
        for (int j = 0; j < n; j++)
        {
            if (v[j] == 0)
            {
                dead_channels++;
                continue;
            }
            temp = pow((v[j] - mean), 2);
            sum += temp;
        }

        return std = std::sqrt(sum / double(n - 1 - dead_channels));
    }
};
