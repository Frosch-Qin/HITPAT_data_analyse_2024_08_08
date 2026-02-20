// Algo.h
#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class Algo : public IAnalyzer<Fullframe>
{
public:
    explicit Algo(const std::string &name) : algo_name(name) {}

    std::string name() const override
    {
        return algo_name;
    }

    void begin_run(const RunContext &ctx) override
    {
        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }

        define_algorithmMap();
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        // Processing code here
        auto it = algorithmMap.find(algo_name);
        if (it != algorithmMap.end())
        {

            for (int i = 0; i < nrBoards; ++i)
            {
                if (tags.boardTags[i].Cluster_num < 1)
                {
                    continue; // Skip processing if no cluster is found
                }
                else
                {
                    it->second(&frame, i, &tags.boardTags[i]);
                    // std::cout << "posi " << tags.boardTags[i].Position << std::endl;
                }
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        // Finalization code here
    }

private:
    std::string algo_name;
    std::map<std::string, std::function<void(Fullframe *, int, beamRecon *)>> algorithmMap;

    void define_algorithmMap()
    {
        algorithmMap["fascluster"] = fas_cluster<Fullframe>;
        algorithmMap["grarms"] = recon_gravity_rms<Fullframe>;
    }

    int nrBoards;
};
