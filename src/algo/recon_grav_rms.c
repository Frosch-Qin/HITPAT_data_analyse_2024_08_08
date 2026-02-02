// recon_result: height_ob{}, mean_ob{},  sigma_ob{};
// the precise version of recon_gravity_rms
template <class T>
void recon_gravity_rms(T *p_frame_data, int boardNum, beamRecon *beam)
{


    double SumY{};
    double SumXY{};
    double MeanX{};
    double MaxY{};
    double Pos_MaxY{};
    double Sigma0{};

    //get mean X by gravity center
    int right_most = (beam->Windowright==319)?319:beam->Windowright; //318?
    for (int i = beam->Windowleft; i <= right_most; i++)
    {
       // std::cout << "i: " << i << std::endl;
        SumY += p_frame_data->boards[boardNum].data[i];
        SumXY += p_frame_data->boards[boardNum].data[i] * (i*0.8+floor(i/64)*0.2);
        if (p_frame_data->boards[boardNum].data[i] > MaxY)
        {
            MaxY = p_frame_data->boards[boardNum].data[i];
            Pos_MaxY = i*0.8+floor(i/64)*0.2;
        }
    }
     if (SumY != 0)
    {
        MeanX = SumXY / SumY;
        if (MeanX < 0 || SumXY < 0 || SumY < 0 || MeanX > 409.6 ) //
        {
            std::cout << "Warning: MeanX SumXY or SumY is negative, or MeanX is more than 13 bits setting to -1." << std::endl;
            MeanX = -1;  // Set to 0 if negative
            Sigma0 = -1; // Set to -1 if no valid data
            beam->Position = MeanX;
            beam->Sigma = Sigma0;
            beam->Peak = MaxY;
            return; // Exit early if no valid data
        }
    }
    else
    {
        std::cout << "Warning: SumY is 0, setting MeanX to -1." << std::endl;
        MeanX = -1;  // Set to -1 if no valid data
        Sigma0 = -1; // Set to -1 if no valid data
        beam->Position = MeanX;
        beam->Sigma = Sigma0;
        beam->Peak = MaxY;
        return; // Exit early if no valid data
    }

    // get sigma by rms
    for (int i = beam->Windowleft; i <= beam->Windowright; i++)
    {
        Sigma0 += p_frame_data->boards[boardNum].data[i] * (i * 0.8 + floor(i / 64) * 0.2 - MeanX) * (i * 0.8 + floor(i / 64) * 0.2 - MeanX);
    }
    if (SumY > 1 && Sigma0 > 0)
    {
        Sigma0 = sqrt(Sigma0 / (SumY - 1));
    }
    else
    {
        std::cout << "Warning: SumY is " << SumY <<" SumXY is " << SumXY << " or Sigma0 is " << Sigma0 << ", setting Sigma0 to -1." << std::endl;
        Sigma0 = -1; // Set to -1 if no valid data
    }

    // convert to decimal before store in beamrecon.
    beam->Position = MeanX;
    beam->Sigma = Sigma0;
    beam->Peak = MaxY;
}