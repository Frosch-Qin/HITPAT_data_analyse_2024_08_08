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
    void on_begin_run(RunContext &ctx) override
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

            // if all the boards get cluster, then tag has signal
            bool check_clusters = true;
            for (int i = 0; i < nrBoards; ++i)
            {
                if (tags.boardTags[i].Cluster_num > 0)
                {
                    check_clusters = check_clusters && true;
                }
                else
                {
                    check_clusters = check_clusters && false;
                }
            }
            tags.Has_signal = check_clusters;

            // std::cout << "Clustering check for frame " << frame_index << ": " << (check_clusters ? "Signal found" : "No signal") << std::endl;
        }
    }
    void end_run(const RunContext &ctx) override
    {

    }

private:
    int clustering_threshold[6];
    int clustering_size[6];
};
