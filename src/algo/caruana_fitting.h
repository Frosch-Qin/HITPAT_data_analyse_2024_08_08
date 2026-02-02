
template <class T>
void caruana_fitting(T *p_frame_data, int boardNum, beamRecon *beam)
{
    double threshold;
    if (boardNum == 0)
        threshold = HITNamespace::INCLUSTER_THRESHOLD_b0;
    else if (boardNum == 1)
        threshold = HITNamespace::INCLUSTER_THRESHOLD_b1;
    double SumX, SumX2, SumX3, SumX4, SumLnY, SumXLnY, SumX2LnY;
    TMatrixD M1(3, 3);
    TMatrixD M1inv(3, 3);
    TVectorD ABC(3);
    TVectorD M2(3);

    std::vector<double> signal_list;
    std::vector<double> channel_list;

    // use the Cluster_windows and threshold sieve data
    for (int i = beam->Windowleft; i < (1 + beam->Windowright); i++)
    {
        if (p_frame_data->boards[boardNum].data[i] > threshold)
        {
            signal_list.push_back(p_frame_data->boards[boardNum].data[i]);
            channel_list.push_back(i * 0.8 + floor(i / 64) * 0.2);
        }
    }

    const int vector_length = channel_list.size();

    // calculate the elements for the matrix
    SumX = 0.;
    SumX2 = 0.;
    SumX3 = 0.;
    SumX4 = 0.;
    SumLnY = 0.;
    SumXLnY = 0.;
    SumX2LnY = 0.;
    for (int i = 0; i < vector_length; i++)
    {
        SumX += channel_list[i];
        SumX2 += channel_list[i] * channel_list[i];
        SumX3 += channel_list[i] * channel_list[i] * channel_list[i];
        SumX4 += channel_list[i] * channel_list[i] * channel_list[i] * channel_list[i];
        SumLnY += log(signal_list[i]);
        SumXLnY += channel_list[i] * log(signal_list[i]);
        SumX2LnY += channel_list[i] * channel_list[i] * log(signal_list[i]);
    }

    M1(0, 0) = vector_length;
    M1(0, 1) = SumX;
    M1(0, 2) = SumX2;
    M1(1, 0) = SumX;
    M1(1, 1) = SumX2;
    M1(1, 2) = SumX3;
    M1(2, 0) = SumX2;
    M1(2, 1) = SumX3;
    M1(2, 2) = SumX4;

    M2(0) = SumLnY;
    M2(1) = SumXLnY;
    M2(2) = SumX2LnY;

    M1inv = M1.Invert();
    ABC = M1inv * M2;

    double a = ABC(0);
    double b = ABC(1);
    double c = ABC(2);

    beam->Peak = exp(a - b * b / (4 * c));
    beam->Position = -b / (2 * c);
    beam->Sigma = sqrt(-1 / (2 * c));

    //the fitting part! to get a nice Peak
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
