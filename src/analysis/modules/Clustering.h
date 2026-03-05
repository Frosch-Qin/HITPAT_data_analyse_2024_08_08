// Clustering
#pragma once

#include <vector>
#include <memory>

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class Clustering : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "Clustering";
    }

    protected:
    void on_begin_run(const RunContext &ctx) override
    {

        for (int i = 0; i < nrBoards; ++i)
        {
            clustering_threshold[i] = ctx.clustering_threshold[i];
            clustering_size[i] = ctx.clustering_size[i];
        }
    }
    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        if (tags.BKG_SUB_ON)
        {
            for (int i = 0; i < nrBoards; ++i)
            {
                if (cluster_locate(&frame, i, &tags.boardTags[i], clustering_threshold[i], clustering_size[i]))
                {
                    // Do something if cluster is located
                }
            }
        }
    }
    void end_run(const RunContext &ctx) override
    {

    }

private:
    int clustering_threshold[6];
    int clustering_size[6];
};
