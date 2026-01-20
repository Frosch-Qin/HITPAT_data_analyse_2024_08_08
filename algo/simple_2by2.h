//window is established by the cluster locating algorithm
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void simple_2by2(T *p_frame_data, int boardNum, beamRecon *beam)
{

  double threshold;
  if (boardNum == 0)
    threshold = HITNamespace::THRESHOLD_b0;
  else if (boardNum == 1)
    threshold = HITNamespace::THRESHOLD_b1;

  ///////////////// linear regression using Integration by parts of gaussian function.

  double SumArea, Sum, SumX, SumX2, SumX3, SumX4, SumLnY, SumXLnY, SumX2LnY, Ymax, Pomax, fac_c, Yn, sigmaABC, sigma, mean, amp, SumYYP, SumYYM, MeanY, Cluster_window_start, Cluster_window_end;
  TMatrixD M1(2, 2);
  TMatrixD M1inv(2, 2);
  TVectorD ABC(2);
  TVectorD M2(2);
  vector<double> signal_list;
  vector<double> channel_list;
  channel_list.clear();
  Ymax = 0.;
  Pomax = 0.;
  SumArea = 0.; // area
  Sum = 0.;
  SumX = 0.;
  SumX2 = 0.;
  SumX3 = 0.;
  SumX4 = 0.;
  SumLnY = 0.;
  SumXLnY = 0.;
  SumX2LnY = 0.;
  amp = 0.;
  sigma = 0.;
  mean = 0.;
  MeanY = 0.;

  const int array_length = p_frame_data->boards[boardNum].nrChannels;
  for (int i = 0; i < array_length; i++)
  {
    if (p_frame_data->boards[boardNum].data[i] > Ymax)
    {
      Ymax = p_frame_data->boards[boardNum].data[i];
      Pomax = i;
    }
    if (i > 0)
    {
      SumArea += p_frame_data->boards[boardNum].data[i];
    }
  }

  // calculate sigma ----> y = amp*exp(-(x-mean)*(x-mean)/2/(sigma*sigma)) 2.5066=sqrt(2*pi) sigma IS SIGMA
  sigma = SumArea / Ymax / 2.5066;
  fac_c = -1. / (2 * sigma * sigma);
  // set a +-3 sigma Cluster_window
  Cluster_window_start = beam->Windowleft;
  Cluster_window_end = beam->Windowright;

  // use the Cluster_windows and threshold sieve data
  for (int i = 0; i < array_length; i++)
  {
    if (p_frame_data->boards[boardNum].data[i] > threshold & i > Cluster_window_start & i < Cluster_window_end)
    {

      signal_list.push_back(p_frame_data->boards[boardNum].data[i]);
      channel_list.push_back(i*0.8+0.4+floor(i/64)*0.2);
    }
  }
  // recalculate SumArea using the sieved data
  SumArea = 0;
  for (int i = 0; i < signal_list.size(); i++)
  {
    if (i > 0)
    {
      SumArea += signal_list[i] * (channel_list[i] - channel_list[i - 1]);
    }
  }

  const int vector_length = channel_list.size();
 // beam->n_channels = vector_length;
  if (vector_length <= 3){
    beam->Position = -1;
    return;
  }

  for (int k = 0; k < vector_length; k++)
  {

    Sum += 1;
    SumX += 1 * channel_list[k];
    SumX2 += 1 * channel_list[k] * channel_list[k];
    SumX3 += 1 * channel_list[k] * channel_list[k] * channel_list[k];

    SumLnY += 1 * log(signal_list[k]);
    SumXLnY += channel_list[k] * 1 * log(signal_list[k]);

    MeanY += signal_list[k];
  }
  MeanY /= vector_length;

  M1(0, 0) = Sum;
  M1(0, 1) = SumX; // M1(0,2) = SumX2;
  M1(1, 0) = SumX;
  M1(1, 1) = SumX2; // M1(1,2) = SumX3Y2;
  //  M1(2,0) = SumX2Y2;	      M1(2,1) = SumX3Y2;      M1(2,2) = SumX4Y2;

  M2(0) = SumLnY - fac_c * SumX2;
  M2(1) = SumXLnY - fac_c * SumX3; // M2(2) = SumX2LnY;
  M1inv = M1.Invert();
  ABC = M1inv * M2;

  // calculate amp,sigma,mean ---> y = amp*exp(-(x-mean)*(x-mean)/2/(sigma*sigma))
  //  cout << ABC(0) << "  " << ABC(1) << "  " << endl;
  //  sigma = sqrt(-1./2/ABC(2));
  mean = -ABC(1) / fac_c / 2;
  amp = exp(ABC(0) - ABC(1) * ABC(1) / 4 / fac_c);
  sigma = SumArea / amp / 2.5066;

  beam->Position = mean;
  beam->Sigma = sigma;
  beam->Peak = amp;
}
