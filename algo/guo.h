template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void guo(T *p_frame_data, int boardNum, beamRecon *beam)
{

    double threshold;
    if (boardNum == 0)
        threshold = HITNamespace::THRESHOLD_b0;
    else if (boardNum == 1)
        threshold = HITNamespace::THRESHOLD_b1;
    double SumY2, SumXY2, SumX2Y2, SumX3Y2, SumX4Y2, SumY2LnY, SumXY2LnY, SumX2Y2LnY;

    TMatrixD M1(3, 3);
    TMatrixD M1inv(3, 3);
    TVectorD ABC(3);
    TVectorD M2(3);

    vector<double> signal_list;
    vector<double> channel_list;

    // use the Cluster_windows and threshold sieve data
    for (int i = beam->Windowleft; i < (1 + beam->Windowright); i++)
    {
        if (p_frame_data->boards[boardNum].data[i] > threshold)
        {
            signal_list.push_back(p_frame_data->boards[boardNum].data[i]);
            channel_list.push_back(i * 0.8 + 0.4 + floor(i / 64) * 0.2);
        }
    }

    const int vector_length = channel_list.size();

    SumY2 = 0.;
    SumXY2 = 0.;
    SumX2Y2 = 0.;
    SumX3Y2 = 0.;
    SumX4Y2 = 0.;
    SumY2LnY = 0.;
    SumXY2LnY = 0.;
    SumX2Y2LnY = 0.;

    for (int k = 0; k < vector_length; k++)
    {

        SumY2 += signal_list[k] * signal_list[k];
        SumXY2 += signal_list[k] * signal_list[k] * channel_list[k];
        SumX2Y2 += signal_list[k] * signal_list[k] * channel_list[k] * channel_list[k];
        SumX3Y2 += signal_list[k] * signal_list[k] * channel_list[k] * channel_list[k] * channel_list[k];
        SumX4Y2 += signal_list[k] * signal_list[k] * channel_list[k] * channel_list[k] * channel_list[k] * channel_list[k];

        SumY2LnY += signal_list[k] * signal_list[k] * log(signal_list[k]);
        SumXY2LnY += channel_list[k] * signal_list[k] * signal_list[k] * log(signal_list[k]);
        SumX2Y2LnY += channel_list[k] * channel_list[k] * signal_list[k] * signal_list[k] * log(signal_list[k]);
    }

    M1(0, 0) = SumY2;
    M1(0, 1) = SumXY2;
    M1(0, 2) = SumX2Y2;
    M1(1, 0) = SumXY2;
    M1(1, 1) = SumX2Y2;
    M1(1, 2) = SumX3Y2;
    M1(2, 0) = SumX2Y2;
    M1(2, 1) = SumX3Y2;
    M1(2, 2) = SumX4Y2;

    M2(0) = SumY2LnY;
    M2(1) = SumXY2LnY;
    M2(2) = SumX2Y2LnY;

    M1inv = M1.Invert();
    ABC = M1inv * M2;

    double a = ABC(0);
    double b = ABC(1);
    double c = ABC(2);

    beam->Peak = exp(a - b * b / (4 * c));
    beam->Position = -b / (2 * c);
    beam->Sigma = sqrt(-1 / (2 * c));
}