// recon_result: height_ob{}, mean_ob{},  sigma_ob{};
// #define Print_mpz(x)                                                                             \
//     {                                                                                            \
//         std::cout << #x << " = " << x << " bitsize = " << mpz_sizeinbase(x, 2) + 1 << std::endl; \
//     }
// template <class T>
// void recon_gravity_rms_hd(T *p_frame_data, int boardNum, beamRecon *beam)
// {

//     int point_bit = 2;
//     mpz_t MaxY{};
//     mpz_t Pos_MaxY{};
//     mpz_t Sigma0{};
//     mpz_t SumY{};
//     mpz_t SumXY{};
//     mpz_t MeanX{};
//     mpz_t beamX{}; // for beamsignal, beamX=p_frame_data->boards[boardNum].data[i]

//     mpz_init(MaxY);
//     mpz_init(Pos_MaxY);
//     mpz_init(Sigma0);
//     mpz_init(SumY);
//     mpz_init(SumXY);
//     mpz_init(MeanX);
//     mpz_init(beamX);

//     double positioni; // in mm
//     int positionInt;  // 0.2mm is one unit
//     // get mean X by gravity center
//     int right_most = (beam->Windowright == 319) ? 319 : beam->Windowright;
//     for (int i = beam->Windowleft; i <= right_most; i++)
//     {
//         positioni = i * 0.8 + floor(i / 64) * 0.2;
//         positionInt = positioni / 0.2;
//         // beamX = p_frame_data->boards[boardNum].data[i];
//         mpz_set_si(beamX, p_frame_data->boards[boardNum].data[i]);
//         // SumY += beamX;
//         mpz_add(SumY, SumY, beamX);
//         // SumXY += beamX * i;
//         mpz_addmul_ui(SumXY, beamX, positionInt);
//         // get max Y
//         if (mpz_get_d(beamX) > mpz_get_d(MaxY))
//         {
//             mpz_set_si(MaxY, p_frame_data->boards[boardNum].data[i]);
//             mpz_set_ui(Pos_MaxY, positionInt);
//         }
//     }
//     Print_mpz(MaxY);
//     Print_mpz(SumY);
//     Print_mpz(SumXY);

//     // MeanX = SumXY / SumY;
//     // left shift SumXY by point_bit
//     mpz_mul_2exp(SumXY, SumXY, point_bit);
//     //std::cout << "SumXYleftshift  ";
//     Print_mpz(SumXY);
//     mpz_cdiv_q(MeanX, SumXY, SumY);
//     //std::cout << "MeanXleftshift  ";
//     Print_mpz(MeanX);

//     // get sigma by rms
//     mpz_t diff_meanx_i;
//     mpz_init(diff_meanx_i);
//     for (int i = beam->Windowleft; i <= right_most; i++)
//     {
//         //std::cout << "i  " << i << std::endl;
//         positioni = i * 0.8 + floor(i / 64) * 0.2;
//         positionInt = positioni / 0.2;
//         // Sigma0 += p_frame_data->boards[boardNum].data[i] * (MeanX-i) * (MeanX-i);
//         // beamX = p_frame_data->boards[boardNum].data[i];
//         mpz_set_si(beamX, p_frame_data->boards[boardNum].data[i]);
//         // diff_meanx_i = MeanX - i;
//         mpz_sub_ui(diff_meanx_i, MeanX, positionInt * pow(2, point_bit));
//         std::cout << "DiffxMeanX  ";
//         Print_mpz(diff_meanx_i);
//         // diff_meanx_i = diff_meanx_i * diff_meanx_i;
//         mpz_mul(diff_meanx_i, diff_meanx_i, diff_meanx_i);
//         std::cout << "DiffxMeanX2  ";
//         Print_mpz(diff_meanx_i);
//         // diff_meanx_i = beamX * diff_meanx_i;
//         mpz_mul(diff_meanx_i, diff_meanx_i, beamX);
//         std::cout << "DiffxMeanX2Yi  ";
//         Print_mpz(diff_meanx_i);
//         // Sigma0 += diff_meanx_i;
//         mpz_add(Sigma0, Sigma0, diff_meanx_i); // here
//         //std::cout << "SumDiffxMeanX2Yi  ";
//         //Print_mpz(Sigma0);
//     }
//     std::cout << "SumDiffxMeanX2Yi  ";
//     Print_mpz(Sigma0);
//     // Sigma0 = sqrt(Sigma0 / (SumY-1));
//     // SumY = SumY - 1;
//     mpz_sub_ui(SumY, SumY, 1);
//     // Sigma0 = Sigma0 / SumY;
//     mpz_cdiv_q(Sigma0, Sigma0, SumY);
//     //////std::cout << "Sigma0: " << mpz_get_d(Sigma0) << std::endl;
//     //std::cout << "Sigma02  ";
//     Print_mpz(Sigma0);
//     // Sigma0 = sqrt(Sigma0);
//     if (mpz_get_d(Sigma0) > 0)
//     {
//         mpz_sqrt(Sigma0, Sigma0);
//     }
//     //std::cout << "Sigma0  ";
//     Print_mpz(Sigma0);

//     double MeanX_d = mpz_get_d(MeanX)/pow(2,point_bit)*0.2;
//     double Sigma0_d = mpz_get_d(Sigma0)/pow(2,point_bit)*0.2;
//     // double MeanX_d = mpz_get_d(MeanX);
//     // double Sigma0_d = mpz_get_d(Sigma0);
//     double MaxY_d = mpz_get_d(MaxY);

//     // convert to decimal before store in beamrecon.
//     beam->Position = MeanX_d;
//     beam->Sigma = Sigma0_d;
//     beam->Peak = MaxY_d;

//     // clear memory
//     mpz_clear(MaxY);
//     mpz_clear(Pos_MaxY);
//     mpz_clear(Sigma0);
//     mpz_clear(SumY);
//     mpz_clear(SumXY);
//     mpz_clear(MeanX);
//     mpz_clear(beamX);
//     mpz_clear(diff_meanx_i);
// }




template <class T>
void recon_gravity_rms_hd(T *p_frame_data, int boardNum, beamRecon *beam)
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
    beam->Position = int(MeanX/0.05)*0.05;
    beam->Sigma = int(Sigma0/0.05)*0.05;
    beam->Peak = MaxY;
}