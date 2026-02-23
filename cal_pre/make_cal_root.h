#include <iostream>
#include <fstream>
#include <string>

void make_cal_root(int nrBoards){

    int cal[nrBoards][320];

    for (int i = 0; i < nrBoards; i++)
    {
        std::ifstream calfile(Form("output/board%d.txt",i));//+to_string(i)+".txt");
        if (!calfile.is_open())
        {
            std::cout << "Error: file open failed" << std::endl;
            return;
        }
        for (int j = 0; j < 320; j++)
        {
            calfile >> cal[i][j];
        }
        calfile.close();
    }

    TGraph *cal_graph[nrBoards];
    for (int i = 0; i < nrBoards; i++){
        cal_graph[i] = new TGraph(320);
        cal_graph[i]->SetName(Form("cal%d",i));
        for (int j = 0; j < 320; j++)
        {
            cal_graph[i]->SetPoint(j, j, cal[i][j]);
        }
    }

    TFile *cal_file = new TFile("output/cal_run19.root", "RECREATE");
    for (int i = 0; i < nrBoards; i++)
    {
        cal_graph[i]->Write();
    }
    cal_file->Close();
    

}