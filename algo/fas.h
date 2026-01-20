//**************************************************************FAST ACCURACY SEPARABLE FAS****************************************
// FAS https://arxiv.org/pdf/1907.07241.pdf
// N_iter = 1 , iterate once
// calculate b,p,c ---> y = b*exp(-(x-c)*(x-c)/2/(p*p))
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void fas(T *p_frame_data, int boardNum, beamRecon *beam)
{

  double threshold;
  if (boardNum == 0)
    threshold = HITNamespace::THRESHOLD_b0;
  else if (boardNum == 1)
    threshold = HITNamespace::THRESHOLD_b1;
  int N_iter = 0;

  ///////////////// linear regression using Integration by parts of gaussian function.

  double SumArea, SumY2, SumXY2, SumX2Y2, SumX3Y2, SumX4Y2, SumY2LnY, SumXY2LnY, SumX2Y2LnY, Ymax, Pomax, fac_c, Yn, sigmaABC, sigma, mean, amp, SumYYP, SumYYM, MeanY, window_start, window_end;
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
  // set a +-3 sigma window
  window_start = Pomax - 3 * sigma;
  window_end = Pomax + 3 * sigma;

  // use the windows and threshold sieve data
  for (int i = 0; i < array_length; i++)
  {
    if (p_frame_data->boards[boardNum].data[i] > threshold & i > window_start & i < window_end)
    {

      signal_list.push_back(p_frame_data->boards[boardNum].data[i]);
      channel_list.push_back(i);
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
  //beam->n_channels = vector_length;
  // beam->Sum = std::accumulate(signal_list.begin(), signal_list.end(),0);
  if (vector_length < 3)
    return;

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
  M1inv = M1.Invert();
  ABC = M1inv * M2;

  // iteration
  for (int i = 0; i < N_iter; i++)
  {
    // Ymax=0.;
    // SumArea=0.;
    SumY2 = 0.;
    SumXY2 = 0.;
    SumX2Y2 = 0.;
    SumX3Y2 = 0.;
    SumY2LnY = 0.;
    SumXY2LnY = 0.;
    for (int k = 0; k < vector_length; k++)
    {

      Yn = exp(ABC(0) + ABC(1) * channel_list[k] + fac_c * channel_list[k] * channel_list[k]);
      // if (Yn>Ymax) Ymax=Yn;
      // SumArea += Yn*0.8;
      SumY2 += Yn * Yn;
      SumXY2 += Yn * Yn * channel_list[k];
      SumX2Y2 += Yn * Yn * channel_list[k] * channel_list[k];
      SumX3Y2 += Yn * Yn * channel_list[k] * channel_list[k] * channel_list[k];

      SumY2LnY += Yn * Yn * log(signal_list[k]);
      SumXY2LnY += channel_list[k] * Yn * Yn * log(signal_list[k]);
    }
    M1(0, 0) = SumY2;
    M1(0, 1) = SumXY2;
    M1(1, 0) = SumXY2;
    M1(1, 1) = SumX2Y2;

    M2(0) = SumY2LnY - fac_c * SumX2Y2;
    M2(1) = SumXY2LnY - fac_c * SumX3Y2;
    M1inv = M1.Invert();
    ABC = M1inv * M2;
  }

  // calculate amp,sigma,mean ---> y = amp*exp(-(x-mean)*(x-mean)/2/(sigma*sigma))
  //  cout << ABC(0) << "  " << ABC(1) << "  " << endl;
  //  sigma = sqrt(-1./2/ABC(2));
  mean = -ABC(1) / fac_c / 2;
  amp = exp(ABC(0) - ABC(1) * ABC(1) / 4 / fac_c);
  sigma = SumArea / amp / 2.5066;

  beam->Position = mean;
  beam->Sigma= sigma;
  beam->Peak = amp;
}
