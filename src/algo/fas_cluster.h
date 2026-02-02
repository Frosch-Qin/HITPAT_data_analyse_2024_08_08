// window is established by the cluster locating algorithm
// the output position with the unit of 0.05mm
template <class T>
void fas_cluster(T *p_frame_data, int boardNum, beamRecon *beam)
{

  double threshold = 15;
  ///////////////// linear regression using Integration by parts of gaussian function.

  double SumArea, SumY2, SumXY2, SumX2Y2, SumX3Y2, SumX4Y2, SumY2LnY, SumXY2LnY, SumX2Y2LnY, Ymax, Pomax, fac_c, Yn, sigmaABC, sigma, mean, amp, SumYYP, SumYYM, MeanY, Cluster_window_start, Cluster_window_end;
  TMatrixD M1(2, 2);
  TMatrixD M1inv(2, 2);
  TVectorD ABC(2);
  TVectorD M2(2);
  std::vector<double> signal_list;
  std::vector<double> channel_list;
  channel_list.clear();
  Ymax = 0.;
  Pomax = 0.;
  SumArea = 0.; // area
  SumY2 = 0.;
  SumXY2 = 0.;
  SumX2Y2 = 0.;
  SumX3Y2 = 0.;
  SumX4Y2 = 0.;
  SumY2LnY = 0.;
  SumXY2LnY = 0.;
  SumX2Y2LnY = 0.;
  amp = 0.;
  sigma = 0.;
  mean = 0.;
  MeanY = 0.;

  // use the Cluster_windows and threshold sieve data
  for (int i = beam->Windowleft; i < (1 + beam->Windowright); i++)
  {
    SumArea += p_frame_data->boards[boardNum].data[i] * (i == 0 ? 0.8 : (i % 64 ? 0.8 : 1.0));
    if (p_frame_data->boards[boardNum].data[i] > Ymax)
    {
      Ymax = p_frame_data->boards[boardNum].data[i];
      Pomax = i;
    }
    if (p_frame_data->boards[boardNum].data[i] > threshold)
    {
      signal_list.push_back(p_frame_data->boards[boardNum].data[i]);
      channel_list.push_back(i * 0.8 + floor(i / 64) * 0.2);
    }
  }

  // calculate sigma ----> y = amp*exp(-(x-mean)*(x-mean)/2/(sigma*sigma)) 2.5066=sqrt(2*pi) sigma IS SIGMA
  sigma = SumArea / Ymax / 2.5066;
  fac_c = -1. / (2 * sigma * sigma);

  const int vector_length = channel_list.size();
  // beam->n_channels = vector_length;
  if (vector_length <= 3)
  {
    beam->Position = -1;
    return;
  }

  for (int k = 0; k < vector_length; k++)
  {

    SumY2 += signal_list[k] * signal_list[k];
    SumXY2 += signal_list[k] * signal_list[k] * channel_list[k];
    SumX2Y2 += signal_list[k] * signal_list[k] * channel_list[k] * channel_list[k];
    SumX3Y2 += signal_list[k] * signal_list[k] * channel_list[k] * channel_list[k] * channel_list[k];

    SumY2LnY += signal_list[k] * signal_list[k] * log(signal_list[k]);
    SumXY2LnY += channel_list[k] * signal_list[k] * signal_list[k] * log(signal_list[k]);

    MeanY += signal_list[k];
  }
  MeanY /= vector_length;

  M1(0, 0) = SumY2;
  M1(0, 1) = SumXY2; // M1(0,2) = SumX2Y2;
  M1(1, 0) = SumXY2;
  M1(1, 1) = SumX2Y2; // M1(1,2) = SumX3Y2;
  //  M1(2,0) = SumX2Y2;	      M1(2,1) = SumX3Y2;      M1(2,2) = SumX4Y2;

  M2(0) = SumY2LnY - fac_c * SumX2Y2;
  M2(1) = SumXY2LnY - fac_c * SumX3Y2; // M2(2) = SumX2Y2LnY;

  if (M1.Determinant() == 0)
  {
    std::cout << "Matrix is singular" << std::endl;
    beam->Position = -1;
    return;
  }
  M1inv = M1.Invert();
  ABC = M1inv * M2;

  // calculate amp,sigma,mean ---> y = amp*exp(-(x-mean)*(x-mean)/2/(sigma*sigma))
  //  cout << ABC(0) << "  " << ABC(1) << "  " << endl;
  //  sigma = sqrt(-1./2/ABC(2));
  mean = -ABC(1) / fac_c / 2;
  amp = exp(ABC(0) - ABC(1) * ABC(1) / 4 / fac_c);
  // sigma = SumArea / amp / 2.5066;

  beam->Position = mean;
  beam->Sigma = sigma;
  beam->Peak = amp;
}
