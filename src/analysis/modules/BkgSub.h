// BkgSub.h
// pass 
#include <vector>
#include <memory>

#include "../IAnalyzer.h"
#include "../FrameTags.h"

class BkgSub : public IAnalyzer<Fullframe>
{
public:
    std::string name() const override
    {
        return "BkgSub";
    }

    void begin_run(const RunContext &ctx) override
    {
        
    }

    void process(Fullframe &frame, long frame_index, FrameTags &tags) override
    {
       tags.BKG_SUB_ON = frame.boards[0].fpgas.BKG_SUB_ON;
    }

    void end_run(const RunContext &ctx) override
    {
        
    }


};
