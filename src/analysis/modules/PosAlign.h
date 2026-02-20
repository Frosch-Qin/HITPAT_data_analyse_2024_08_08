// PosAlign.h

#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class PosAlign : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "PosAlign";
    }

    void begin_run(const RunContext &ctx) override
    {
        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }
        }

    void process( Fullframe &frame, long frame_index,  FrameTags &tags) override
    {
        // Position alignment logic goes here
        for (int i = 0; i < nrBoards; ++i)
        {
            if (tags.boardTags[i].Cluster_num < 1)
            {
                continue; // Skip processing if no cluster is found
            }
            tags.boardTags[i].Position = tags.boardTags[i].Position * slope_scaled[i] + intercept_scaled[i];
        }
    }

    void end_run(const RunContext &ctx) override
    {
    }

private:
    int nrBoards = 6;

    // values from the alignment run17,18,19 alignment/align4.C
    double intercept_scaled[6] = {129.79469, 133.77857, 128.53562, 130.49730, 124.82089, 128.73066};
    double slope_scaled[6] = {-1.0132711, -1.0135994, -1.0000000, -1.0000000, -0.98736398, -0.98927401};
};
