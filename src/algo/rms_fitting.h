#pragma once

// recon_result: height_ob{}, mean_ob{},  sigma_ob{};
// the precise version of recon_gravity_rms
template <class T>
void rms_fitting(T *p_frame_data, int boardNum, beamRecon *beam)
{

    double SumY{};
    double SumXY{};
    double MeanX{};
    double MaxY{};
    double Pos_MaxY{};
    double Sigma0{};

    // get mean X by gravity center
    int right_most = (beam->Windowright == 319) ? 318 : beam->Windowright;
    for (int i = beam->Windowleft; i <= right_most; i++)
    {
        // std::cout << "i: " << i << std::endl;
        SumY += p_frame_data->boards[boardNum].data[i];
        SumXY += p_frame_data->boards[boardNum].data[i] * (i * 0.8 + floor(i / 64) * 0.2);
        if (p_frame_data->boards[boardNum].data[i] > MaxY)
        {
            MaxY = p_frame_data->boards[boardNum].data[i];
            Pos_MaxY = i * 0.8 + floor(i / 64) * 0.2;
        }
    }
    MeanX = SumXY / SumY;

    // get sigma by rms
    for (int i = beam->Windowleft; i <= right_most; i++)
    {
        Sigma0 += p_frame_data->boards[boardNum].data[i] * (i * 0.8 + floor(i / 64) * 0.2 - MeanX) * (i * 0.8 + floor(i / 64) * 0.2 - MeanX);
    }
    Sigma0 = sqrt(Sigma0 / (SumY - 1));

    // convert to decimal before store in beamrecon.
    beam->Position = MeanX;
    beam->Sigma = Sigma0;
    beam->Peak = MaxY;


    //the fitting part! to get a nice Peak
    double threshold;
    if (boardNum == 0)
        threshold = HITNamespace::INCLUSTER_THRESHOLD_b0;
    else if (boardNum == 1)
        threshold = HITNamespace::INCLUSTER_THRESHOLD_b1;

    TGraph *g = new TGraph();
    int j = 0;
    for (int i = beam->Windowleft; i < (1 + beam->Windowright); i++)
    {
        if (p_frame_data->boards[boardNum].data[i] > threshold)
        {
            g->SetPoint(j, i * 0.8 + floor(i / 64) * 0.2, p_frame_data->boards[boardNum].data[i]);
            j++;
        }
    }

    double windowleft_mm = beam->Windowleft * 0.8 + floor(beam->Windowleft / 64) * 0.2;
    double windowright_mm = beam->Windowright * 0.8 + floor(beam->Windowright / 64) * 0.2;

    // TF1 *fitFunc = new TF1("fitFunc", "gaus", windowleft_mm, windowright_mm);
    //  fitFunc->SetParLimits(1, 10, p_frame_data->boards[boardNum].nrChannels-10);   // Limit the mean parameter
    //  fitFunc->SetParLimits(2, 0, 10);  // Limit the sigma parameter
    TF1 *fitFunc = new TF1("fitFunc", "gaus", windowleft_mm, windowright_mm);
    fitFunc->SetParameters(100, 20, 5);
    fitFunc->SetParLimits(1, 0, 256);  // Limit the mean parameter
    fitFunc->SetParLimits(2, 0, 16.9); // Limit the sigma parameter
    g->Fit(fitFunc, "RQ");             //"R" for use the range for fitFunc; "B" for use the SetParLimits;"Q" for quite mode

    beam->Peak = fitFunc->GetParameter(0);

    delete fitFunc;
    delete g;
}