#pragma once
template <class T>
void caruana(T *p_frame_data, int boardNum, beamRecon *beam)
{
    double threshold = 15;
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

    if (M1.Determinant() == 0)
    {
        std::cout << "Matrix is singular" << std::endl;
        beam->Position = -1;
        return;
    }

    M1inv = M1.Invert();
    ABC = M1inv * M2;

    double a = ABC(0);
    double b = ABC(1);
    double c = ABC(2);

    beam->Peak = exp(a - b * b / (4 * c));
    beam->Position = -b / (2 * c);
    beam->Sigma = sqrt(-1 / (2 * c));

    return;
}
