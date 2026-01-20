#include <iostream>
#include <fstream>
#include <string>

{

    int cal[6][320];

    for (int i = 0; i < 6; i++)
    {
        ifstream calfile(Form("board%d.txt",i));//+to_string(i)+".txt");
        if (!calfile.is_open())
        {
            cout << "Error: file open failed" << endl;
            return 0;
        }
        for (int j = 0; j < 320; j++)
        {
            calfile >> cal[i][j];
        }
        calfile.close();
    }

    TGraph *cal_graph[6];
    for (int i = 0; i < 6; i++){
        cal_graph[i] = new TGraph(320);
        cal_graph[i]->SetName(Form("cal%d",i));
        for (int j = 0; j < 320; j++)
        {
            cal_graph[i]->SetPoint(j, j, cal[i][j]);
        }
    }

    TFile *cal_file = new TFile("cal_run19.root", "RECREATE");
    for (int i = 0; i < 6; i++)
    {
        cal_graph[i]->Write();
    }
    cal_file->Close();
    

}