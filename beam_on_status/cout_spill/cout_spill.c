

#include "../../hitreader/analyser.h"


int main(int argc, char *argv[])
{
    auto run_name = argv[1];
    int clsize = std::stoi(argv[2]);
    vector<double> spill_start_time;
    vector<double> spill_end_time;
    TFile *file = TFile::Open(Form("../rootfile/%s_%d_beam_on_status.root", run_name, clsize), "READ");
    if (!file || !file->IsOpen())
    {
        std::cerr << "Error opening file: " << Form("../rootfile/%s_%d_beam_on_status.root", run_name, clsize) << std::endl;
        return 1;
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
        std::cout << (y - x) << ",";
    }
    std::cout << std::endl;
    file->Close();

    return 0;
}
