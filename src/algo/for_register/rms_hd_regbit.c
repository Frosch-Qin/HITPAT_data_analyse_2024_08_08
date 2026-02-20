// recon_result: height_ob{}, mean_ob{},  sigma_ob{};
// center of Gravity and rms
// to get the bitsize of each REG, we need to store it in root file
#pragma once

#define Print_bitsize(x)                                                                 \
    // {                                                                                    \
    //     std::cout << #x << " = " << x.douval << " bitsize = " << x.bitsize << std::endl; \
    // }

void rms_hd_regbit(int16_t *beam_in, beamRecon *beam, regArray *regs)
{

    int point_bit = HDNamespace::POINT_BITSIZE;

    WATCH_REG X, Y, XY;
    WATCH_REG X_leftshift;
    REG MeanX; // they are all zero by declaration
    WATCH_REG DiffxMeanX, DiffxMeanX2, DiffxMeanX2Yi;
    REG Sigma2, Sigma0;

    double pos_mm; // in mm
    int posInt;    // 0.2mm is one unit
    // get mean X by gravity center
    for (int i = beam->Windowleft; i < beam->Windowright; i++)
    {
        pos_mm = i * 0.8 + 0.4 + floor(i / 64) * 0.2;
        posInt = pos_mm / 0.2;
        // std::cout << "posInt = " << posInt << std::endl;
        X.setvalue(posInt);
        Y.accumulate(p_frame_data->boards[boardNum].data[i]);
        XY.accumulate(X.temp * Y.temp);
    }
    // MeanX = SumXY / SumY;
    // left shift SumXY by point_bit
    XY.set(XY.leftshift(point_bit));
    MeanX.set_mpzval(XY.Sum / Y.Sum);

    REG unit1;
    unit1.set_mpzval(1);
    for (int i = beam->Windowleft; i < beam->Windowright; i++)
    {
        // std::cout << "i  " << i << std::endl;
        pos_mm = i * 0.8 + 0.4 + floor(i / 64) * 0.2;
        posInt = pos_mm / 0.2;
        // Sigma0 += p_frame_data->boards[boardNum].data[i] * (MeanX-i) * (MeanX-i);
        X.setvalue(posInt);
        X_leftshift.set(X.leftshift(point_bit));
        Y.setvalue(p_frame_data->boards[boardNum].data[i]);
        DiffxMeanX.setvalue(MeanX - X_leftshift.temp);
        DiffxMeanX2.setvalue(DiffxMeanX.temp * DiffxMeanX.temp);
        DiffxMeanX2Yi.accumulate(DiffxMeanX2.temp * Y.temp);
        Print_bitsize(Y.temp);
    }

    Y.Sum.set_mpzval(Y.Sum - unit1);
    Sigma2.set_mpzval(DiffxMeanX2Yi.Sum / Y.Sum);
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
    double MaxY_d = Y.Max.douval;

    // convert to decimal before store in beamrecon.
    beam->Position = MeanX_d;
    beam->Sigma = Sigma0_d;
    beam->Peak = MaxY_d;

    // print the bitsize of all the REG
    //  Print_bitsize(X.Max);
    //  Print_bitsize(X_leftshift.Max);
    //  Print_bitsize(Y.Max);
    //  Print_bitsize(Y.Sum);
    //  Print_bitsize(XY.Max);
    //  Print_bitsize(XY.Sum);
    //  Print_bitsize(MeanX);
    //  Print_bitsize(DiffxMeanX.Max);
    //  Print_bitsize(DiffxMeanX2.Max);
    //  Print_bitsize(DiffxMeanX2Yi.Sum);
    //  Print_bitsize(DiffxMeanX2Yi.Max);
    //  Print_bitsize(Sigma2);
    //  Print_bitsize(Sigma0);

    regs->douval[0] = X.Max.douval;
    regs->douval[1] = Y.Max.douval;
    regs->douval[2] = Y.Sum.douval;
    regs->douval[3] = XY.Max.douval;
    regs->douval[4] = XY.Sum.douval;
    regs->douval[5] = MeanX.douval;
    regs->douval[6] = DiffxMeanX.Max.douval;
    regs->douval[7] = DiffxMeanX2.Max.douval;
    regs->douval[8] = DiffxMeanX2Yi.Max.douval;
    regs->douval[9] = DiffxMeanX2Yi.Sum.douval;
    regs->douval[10] = Sigma2.douval;
    regs->douval[11] = Sigma0.douval;

    regs->bitsize[0] = X.Max.bitsize;
    regs->bitsize[1] = Y.Max.bitsize;
    regs->bitsize[2] = Y.Sum.bitsize;
    regs->bitsize[3] = XY.Max.bitsize;
    regs->bitsize[4] = XY.Sum.bitsize;
    regs->bitsize[5] = MeanX.bitsize;
    regs->bitsize[6] = DiffxMeanX.Max.bitsize;
    regs->bitsize[7] = DiffxMeanX2.Max.bitsize;
    regs->bitsize[8] = DiffxMeanX2Yi.Max.bitsize;
    regs->bitsize[9] = DiffxMeanX2Yi.Sum.bitsize;
    regs->bitsize[10] = Sigma2.bitsize;
    regs->bitsize[11] = Sigma0.bitsize;
}