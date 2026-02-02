// SpillIDInput.h

#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class SpillIDInput : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "SpillIDInput";
    }

    void begin_run(const RunContext &ctx) override
    {
        get_spill_time(ctx);
        readout_rate = ctx.readout_rate;
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        // Processing code here
        double eventTime = frame_index / readout_rate;

        if (spillID_ctr >= spillNum)
        {
            // std::cout << "No more spill info available." << std::endl;
            tags.SpillID = ((spillID_ctr % 2 == 0) ? -2 : -1);
        }
        else
        {
            if (eventTime < spill_start_time[spillID_ctr])
            {
                // before the spill
                tags.SpillID = ((spillID_ctr % 2 == 0) ? -2 : -1);
            }
            else if (eventTime < spill_end_time[spillID_ctr])
            {
                // inside the spill
                tags.SpillID = spillID_ctr;
            }
            else
            {
                tags.SpillID = ((spillID_ctr % 2 == 0) ? -1 : -2);
                spillID_ctr++;
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        // Finalization code here
    }

private:

    int spillID_ctr = 0;
    int spillID = -3;
    int spillNum = 0; 

    std::vector<double> spill_start_time;
    std::vector<double> spill_end_time;
    double readout_rate;
    // run22 clsize = 4
    void get_spill_time(const RunContext &ctx)
    {
        TFile *file = TFile::Open(Form("../output/SpillID/run%d_SpillID.root", ctx.run_number), "READ");
        if (!file || !file->IsOpen())
        {
            std::cerr << "Error opening file: " << Form("../../../output/SpillID/run%d_SpillID.root", ctx.run_number) << std::endl;
            return;
        }
        TTree *tree = (TTree *)file->Get("spill_info_tree");

        double x;
        double y;
        tree->SetBranchAddress("spill_start", &x);
        tree->SetBranchAddress("spill_end", &y);

        std::cout << tree->GetEntries() << " spills read from beam_on_status.root file." << std::endl;

        for (int i = 0; i < tree->GetEntries(); i++)
        {
            tree->GetEntry(i);
            spill_start_time.push_back(x);
            spill_end_time.push_back(y);
            std::cout << "Spill " << i << ": Start = " << x << ", End = " << y << ", Duration = " << (y - x) << std::endl;
        }
        file->Close();

        spillNum = spill_start_time.size();

        return;
    }
};