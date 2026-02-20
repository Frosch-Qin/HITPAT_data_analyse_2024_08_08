//Common Mode Subtraction
//CommonModeSub.h

#pragma once

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class CommonModeSub : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "CommonModeSub";
    }

    void begin_run(const RunContext &ctx) override
    {

        nrBoards = ctx.nrBoards > 6 ? 6 : ctx.nrBoards;
        if (nrBoards > 6)
        {
            std::cerr << "Warning: nrBoards in RunContext is greater than 6, limiting to 6." << std::endl;
        }
        for (int i = 0; i < nrBoards; ++i)
        {
            common_mode[i] = ctx.common_mode[i];
        }
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
        for (int i = 0; i < nrBoards; ++i)
        {
            if (common_mode[i])
            {
                double avg4 = get_avg(frame.boards[i].data, 4);
                for (int j = 0; j < frame.boards[i].nrChannels; ++j)
                {
                    frame.boards[i].data[j] -= avg4;
                }
            }
        }
    }

    void end_run(const RunContext &ctx) override
    {
        
    }

private:


    int nrBoards = 6;
    bool common_mode[6] = {false};

    
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

};
