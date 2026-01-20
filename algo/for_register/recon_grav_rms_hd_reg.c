// recon_result: height_ob{}, mean_ob{},  sigma_ob{};
#define Print_bitsize(x) {std::cout << #x << " = " << x.douval << " bitsize = "<< x.bitsize <<std::endl;}
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void recon_gravity_rms_hd_reg(T *p_frame_data, int boardNum, beamRecon *beam)
{

    int point_bit = 2;//HDNamespace::POINT_BITSIZE;
    REG X, Y, MaxY, Pos_MaxY, XY, Sigma0, SumY, SumXY, MeanX, beamX; // they are all zero by declaration

    double pos_mm; // in mm
    int posInt;    // 0.2mm is one unit
    // get mean X by gravity center
    for (int i = beam->Windowleft; i < beam->Windowright; i++)
    {
        pos_mm = i * 0.8 + 0.4 + floor(i / 64) * 0.2;
        posInt = pos_mm / 0.2;
        X.set_mpzval(posInt);
        Y.set_mpzval(p_frame_data->boards[boardNum].data[i]);
        std::cout <<"Y" << i << " = " << Y.douval << std::endl;
        SumY.set_mpzval(SumY + Y);
        XY.set_mpzval(X * Y);
        SumXY.set_mpzval(SumXY + XY);
        // get max Y
        if (MaxY.douval < Y.douval)
        {
            MaxY.set_mpzval(Y);
            Pos_MaxY.set_mpzval(X);
        }
    }
    // MeanX = SumXY / SumY;
    // left shift SumXY by point_bit
    SumXY.leftshift(point_bit);
    MeanX.set_mpzval(SumXY / SumY);

    REG DiffxMeanX;
    REG DiffxMeanX2;
    REG DiffxMeanX2Yi;
    REG SumDiffxMeanX2Yi;
    REG Sigma2;

    REG unit1;
    unit1.set_mpzval(1);

    for (int i = beam->Windowleft; i < beam->Windowright; i++)
    {
        // std::cout << "i  " << i << std::endl;
        pos_mm = i * 0.8 + 0.4 + floor(i / 64) * 0.2;
        posInt = pos_mm / 0.2;
        // Sigma0 += p_frame_data->boards[boardNum].data[i] * (MeanX-i) * (MeanX-i);
        X.set_mpzval(posInt);
        X.leftshift(point_bit);
        Y.set_mpzval(p_frame_data->boards[boardNum].data[i]);
        DiffxMeanX.set_mpzval(MeanX - X);
        DiffxMeanX2.set_mpzval(DiffxMeanX * DiffxMeanX);
        DiffxMeanX2Yi.set_mpzval(DiffxMeanX2 * Y);
        SumDiffxMeanX2Yi.set_mpzval(SumDiffxMeanX2Yi + DiffxMeanX2Yi);
    }

    SumY.set_mpzval(SumY - unit1);
    Sigma2.set_mpzval(SumDiffxMeanX2Yi / SumY);
    if (Sigma2.douval > 0)
    {
        Sigma0.set_mpzval(Sigma2.sqrt());
    }
    else
    {
        Sigma0.set_mpzval(0);
    }

    double MeanX_d = MeanX.douval / pow(2, point_bit) * 0.2;
    double Sigma0_d = Sigma0.douval / pow(2, point_bit) * 0.2;
    double MaxY_d = MaxY.douval;

    // convert to decimal before store in beamrecon.
    beam->Position = MeanX_d;
    beam->Sigma = Sigma0_d;
    beam->Peak = MaxY_d;

    //print the bitsize of all the REG
    Print_bitsize(X);
    Print_bitsize(Y);
    Print_bitsize(MaxY);
    Print_bitsize(Pos_MaxY);
    Print_bitsize(XY);
    Print_bitsize(Sigma0);
    Print_bitsize(SumY);
    Print_bitsize(SumXY);
    Print_bitsize(MeanX);
    Print_bitsize(beamX);
    Print_bitsize(DiffxMeanX);
    Print_bitsize(DiffxMeanX2);
    Print_bitsize(DiffxMeanX2Yi);
    Print_bitsize(SumDiffxMeanX2Yi);
    Print_bitsize(Sigma2);
    Print_bitsize(unit1);
}