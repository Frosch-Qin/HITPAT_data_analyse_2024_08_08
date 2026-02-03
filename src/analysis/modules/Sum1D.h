// Sum1D.h

#include <vector>
#include <memory>

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class Sum1D : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "Sum1D";
    }

    void begin_run(const RunContext &ctx) override
    {
        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        if (tags.BKG_SUB_ON)
        {
            if (tags.SpillID == -2) // groupA
            {
                for (int i = 0; i < nrBoards; ++i)
                {
                    for (int j = 0; j < 320; ++j)
                    {
                        pedestalA[i][j] += frame.boards[i].data[j];
                    }
                }
                num_pedestalA++;
            }

            if (tags.SpillID == -1) // groupB
            {
                for (int i = 0; i < nrBoards; ++i)
                {
                    for (int j = 0; j < 320; ++j)
                    {
                        pedestalB[i][j] += frame.boards[i].data[j];
                    }
                }
                num_pedestalB++;
            }

            if (tags.SpillID >= 0) // signal spill
            {
                for (int i = 0; i < nrBoards; ++i)
                {
                    for (int j = 0; j < 320; ++j)
                    {
                        signal[i][j] += frame.boards[i].data[j];
                    }
                }
                num_signal++;
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        create_histograms(ctx);
        write_histograms(ctx);
    }

private:
    int nrBoards = 6;
    double pedestalA[6][320]{};
    double pedestalB[6][320]{};
    double signal[6][320]{};
    double signal_sub_pedestalA[6][320]{};
    double pedestalB_sub_pedestalA[6][320]{};

    TGraph *pedestalA_graph[6]{};
    TGraph *pedestalB_graph[6]{};
    TGraph *signal_graph[6]{};
    TGraph *signal_sub_pedestalA_graph[6]{};
    TGraph *pedestalB_sub_pedestalA_graph[6]{};

    long num_pedestalA = 0;
    long num_pedestalB = 0;
    long num_signal = 0;

    TDirectory *dir_ = nullptr;

    void create_histograms(const RunContext &ctx)
    {

        if (!ctx.rootfile)
            return;
        dir_ = ctx.rootfile->GetDirectory("Sum1D");
        if (!dir_)
            dir_ = ctx.rootfile->mkdir("Sum1D");
        dir_->cd();

        for (int i = 0; i < nrBoards; ++i)
        {
            pedestalA_graph[i] = new TGraph();
            pedestalB_graph[i] = new TGraph();
            signal_graph[i] = new TGraph();
            signal_sub_pedestalA_graph[i] = new TGraph();
            pedestalB_sub_pedestalA_graph[i] = new TGraph();

            // set name
            pedestalA_graph[i]->SetName(Form("pedestalA_graph_%d", i));
            pedestalB_graph[i]->SetName(Form("pedestalB_graph_%d", i));
            signal_graph[i]->SetName(Form("signal_graph_%d", i));
            signal_sub_pedestalA_graph[i]->SetName(Form("signal_sub_pedestalA_graph_%d", i));
            pedestalB_sub_pedestalA_graph[i]->SetName(Form("pedestalB_sub_pedestalA_graph_%d", i));


        }
    };

    void write_histograms(const RunContext &ctx)
    {

        if (!ctx.rootfile)
            return;

        ctx.rootfile->cd();
        dir_->cd();

        for (int i = 0; i < nrBoards; ++i)
        {
            // set data
            for (int j = 0; j < 320; ++j)
            {
                // average data
                pedestalA[i][j] /= num_pedestalA;
                pedestalB[i][j] /= num_pedestalB;
                signal[i][j] /= num_signal;
                signal_sub_pedestalA[i][j] = signal[i][j] - pedestalA[i][j];
                pedestalB_sub_pedestalA[i][j] = pedestalB[i][j] - pedestalA[i][j];

                pedestalA_graph[i]->SetPoint(j, j, pedestalA[i][j]);
                pedestalB_graph[i]->SetPoint(j, j, pedestalB[i][j]);
                signal_graph[i]->SetPoint(j, j, signal[i][j]);
                signal_sub_pedestalA_graph[i]->SetPoint(j, j, signal_sub_pedestalA[i][j]);
                pedestalB_sub_pedestalA_graph[i]->SetPoint(j, j, pedestalB_sub_pedestalA[i][j]);
            }

            //set X axis range 0 to 319, title ChannelID
            pedestalA_graph[i]->GetXaxis()->SetRangeUser(0, 319);
            pedestalA_graph[i]->GetXaxis()->SetTitle("ChannelID");
            pedestalB_graph[i]->GetXaxis()->SetRangeUser(0, 319);
            pedestalB_graph[i]->GetXaxis()->SetTitle("ChannelID");
            signal_graph[i]->GetXaxis()->SetRangeUser(0, 319);
            signal_graph[i]->GetXaxis()->SetTitle("ChannelID");
            signal_sub_pedestalA_graph[i]->GetXaxis()->SetRangeUser(0, 319);
            signal_sub_pedestalA_graph[i]->GetXaxis()->SetTitle("ChannelID");
            pedestalB_sub_pedestalA_graph[i]->GetXaxis()->SetRangeUser(0, 319);
            pedestalB_sub_pedestalA_graph[i]->GetXaxis()->SetTitle("ChannelID");

            pedestalA_graph[i]->Write();
            pedestalB_graph[i]->Write();
            signal_graph[i]->Write();
            signal_sub_pedestalA_graph[i]->Write();
            pedestalB_sub_pedestalA_graph[i]->Write();
        }
    }
};
