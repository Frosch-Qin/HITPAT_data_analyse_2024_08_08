// threshold for judge the beam is on or off
// on condition: at least CLUSTER_SIZE of signal higher than threshold continuously
#define THRESHOLD 14
#define CLUSTER_SIZE 4
#define INCLUSTER_THRESHOLD 14
#define FAC_SIGMA2FWHM 2.35482

// the structure for output for the beam reconstruction algorithm
struct beamRecon
{
  double Position;
  double Sigma;
  double Peak;
  double Cluster_num; // get cluster
  double Windowleft;  // get cluster
  double Windowright; // get cluster
  double Chi2;
  double BKG_slope;   // for reconstruction with bkg slope and offset
  double BKG_offset;  // for reconstruction with bkg slope and offset
  double Sum;         // for evaluation: in cluster window sum; sum_var_rsqr
  double Variance;    // for evaluation: in cluster window ;sum_var_rsqr
  double Rsqr;        // for evaluation: in cluster window ;sum_var_rsqr
  double Baseline;    // get baseline //the average of signal out of the gaussian 4 sigma range
  double Snr;         // get_beaminfo // max/noise_std
  double Noise_mean;  // get_beaminfo //first 30 and last 30 channels
  double Noise_std;   // get_beaminfo
  double Beam_on;     // has cluster 10 in row
  double Beam_on_judge1; // has cluster 3 in row
  double Interlock;   // when beam_on is 1, lost 3 in a row;
  double False_alarm; // when beam_on is 0, 3 in a row with cluster;
  // double Channel_avg4;    // get_beaminfo
  // double Channel_avg16;   // get_beaminfo
  // double Channel_avg8;    // get_beaminfo
  // double Channel_last_avg4;     // get_beaminfo
  // double Channel_last_avg8;    // get_beaminfo
  // double Channel_last_avg16;   // get_beaminfo
  int n_channels; // get_beaminfo
};

beamRecon beamRecon_beamoff = {0};

// Function for average
double get_avg(vector<double> v)
{
  double return_value = 0.0;
  int n = v.size();

  for (int i = 0; i < n; i++)
  {
    return_value += v[i];
  }
  return (return_value / double(n));
}

//****************End of average funtion****************

// Function for variance
double get_standard_deviation(vector<double> v, double mean)
{
  double sum = 0.0;
  double temp = 0.0;
  double std = 0.0;

  for (int j = 0; j < v.size(); j++)
  {
    temp = pow((v[j] - mean), 2);
    sum += temp;
  }

  return std = std::sqrt(sum / double(v.size() - 2));
}

/***** before calibration ***********/
/*
  int n_channels;  //
  double Noise_mean;  //  channel 0~30
  double Noise_std;   //  channel 0~30
  double Snr;         //
   double Channel_avg4;
  double Channel_avg16;
  double Channel_avg8;  */
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void get_snr(T *p_frame_data, int boardNum, beamRecon *beam, double noise_uncorrected = 0)
{

  beam->n_channels = p_frame_data->boards[boardNum].nrChannels;
  // get noise_std, the first 30 and last 30 channels
  // vector<double> noise;
  // for (int i = 0; i < 30; ++i)
  // {
  //   noise.push_back(p_frame_data->boards[boardNum].data[i]);
  // }

  // for (int i = p_frame_data->boards[boardNum].nrChannels - 30; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  // {
  //   noise.push_back(p_frame_data->boards[boardNum].data[i]);
  // }

  // double noise_mean = get_avg(noise);
  // double noise_std = get_standard_deviation(noise, noise_mean);
  // beam->Noise_std = noise_std;
  // beam->Noise_mean = noise_mean;

  // sum and avg
  double max_sig{};
  for (int i = 0; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  {
    if (p_frame_data->boards[boardNum].data[i] > max_sig)
      max_sig = p_frame_data->boards[boardNum].data[i];
  }
  double snr = max_sig / noise_uncorrected;
  beam->Snr = snr;
  // double avg_4=0;
  // double avg_16=0;
  // double avg_8=0;
  // double avg_4_last=0;
  // double avg_16_last=0;
  // double avg_8_last=0;
  // for (int i = 0; i < 4; ++i)
  // {
  //   avg_4 += p_frame_data->boards[boardNum].data[i];
  // }
  // for (int i = 0; i < 16; ++i)
  // {
  //   avg_16 += p_frame_data->boards[boardNum].data[i];
  // }
  // for (int i = 0; i < 8; ++i)
  // {
  //   avg_8 += p_frame_data->boards[boardNum].data[i];
  // }
  // for (int i = p_frame_data->boards[boardNum].nrChannels-4; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  // {
  //   avg_4_last += p_frame_data->boards[boardNum].data[i];
  // }
  // for (int i = p_frame_data->boards[boardNum].nrChannels-16; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  // {
  //   avg_16_last += p_frame_data->boards[boardNum].data[i];
  // }
  // for (int i = p_frame_data->boards[boardNum].nrChannels-8; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  // {
  //   avg_8_last += p_frame_data->boards[boardNum].data[i];
  // }
  // beam->Channel_avg4 = avg_4/4;
  // beam->Channel_avg16 = avg_16/16;
  // beam->Channel_avg8 = avg_8/8;
  // beam->Channel_last_avg4 = avg_4_last/4;
  // beam->Channel_last_avg16 = avg_16_last/16;
  // beam->Channel_last_avg8 = avg_8_last/8;
  return;
  // beam->Channel_avg4 = p_frame_data->boards[boardNum].data[0];
  // beam->Channel_avg16 = p_frame_data->boards[boardNum].data[1];
  // beam->Channel_avg8 = p_frame_data->boards[boardNum].data[2];
}

/*****add calculate for average, variance, total sum of variance, and residual sum of squares, Rsqr from the input data and reconstruction result****/
/* double Sum;         // in cluster window sum; sum_var_rsqr
  double Variance;    // in cluster window ;sum_var_rsqr
  double Rsqr;        // in cluster window ;sum_var_rsqr
*/
// with BKG_slope and BKG_offset
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void sum_var_rsqr(T *p_frame_data, int boardNum, beamRecon *beam)
{

  double avg = 0;
  double sum = 0;
  double var = 0;
  double sum_var = 0;
  double sum_res2 = 0;
  double rsqr = 0;
  double baseline = 0;

  double left_pointmm = (beam->Windowleft * 0.8 + floor(beam->Windowleft / 64) * 0.2);
  double right_pointmm = (beam->Windowright * 0.8 + floor(beam->Windowright / 64) * 0.2);

  if (beam->Cluster_num < 1)
  {
    return;
  }
  // sum and avg
  for (int i = beam->Windowleft; i <= beam->Windowright; ++i)
  {
    sum += p_frame_data->boards[boardNum].data[i];
  }
  avg = sum / (right_pointmm - left_pointmm);

  // variance, and total sum of variance
  for (int i = beam->Windowleft; i <= beam->Windowright; ++i)
  {
    sum_var += pow((p_frame_data->boards[boardNum].data[i] - avg), 2);
  }
  var = sum_var / (right_pointmm - left_pointmm);
  // residual sum of squares
  double temp_f = 0;
  double pos_pointmm;
  for (int i = beam->Windowleft; i <= beam->Windowright; ++i)
  {
    pos_pointmm = (i * 0.8 + floor(i / 64) * 0.2);
    temp_f = beam->Peak * exp(-0.5 * (pos_pointmm - beam->Position) / beam->Sigma * (pos_pointmm - beam->Position) / beam->Sigma) + beam->BKG_slope * pos_pointmm + beam->BKG_offset;
    //  cout << i << " " <<temp_f << endl;
    sum_res2 += pow((temp_f - p_frame_data->boards[boardNum].data[i]), 2);
  }

  // rsqr
  rsqr = 1 - sum_res2 / sum_var;

  // define result;
  beam->Variance = var;
  beam->Rsqr = rsqr;
  // beam->Sum = sum;

  double sum_4sigma = 0;
  for (int i = 0; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  {
    pos_pointmm = (i * 0.8 + floor(i / 64) * 0.2);
    if (pos_pointmm > (beam->Position - 4 * beam->Sigma) && pos_pointmm < (beam->Position + 4 * beam->Sigma))
    {
      sum_4sigma += p_frame_data->boards[boardNum].data[i];
    }
  }

  beam->Sum = sum_4sigma;
  return;
}

// get baseline; the average of signal out of the gaussian 3 sigma range
template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void get_baseline(T *p_frame_data, int boardNum, beamRecon *beam)
{
  double baseline = 0;
  vector<int> no_beam_position;

  // if there is no Cluster, return the mean of the signal
  if (beam->Cluster_num < 1)
  {
    for (int i = 0; i < p_frame_data->boards[boardNum].nrChannels; ++i)
    {
      baseline += p_frame_data->boards[boardNum].data[i];
    }
    beam->Baseline = baseline / p_frame_data->boards[boardNum].nrChannels;
    return;
  }

  // if there is a cluster, return the baseline outside of the gaussian 4 sigma range
  // double pos_pointmm;
  // for (int i = 0; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  // {
  //   pos_pointmm = (i * 0.8 + floor(i / 64) * 0.2);
  //   if (pos_pointmm < (beam->Position - 4 * beam->Sigma) || pos_pointmm > (beam->Position + 4 * beam->Sigma))
  //   {
  //     no_beam_position.push_back(i);
  //   }
  // }
  // for (int i = 0; i < no_beam_position.size(); i++)
  // {
  //   baseline += p_frame_data->boards[boardNum].data[no_beam_position.at(i)];
  // }
  // baseline = baseline / no_beam_position.size();


  // if there is a cluster, return the avg of first 30 and last 30 channels
  for (int i = 0; i < 30; ++i)
  {
    baseline += p_frame_data->boards[boardNum].data[i] + p_frame_data->boards[boardNum].data[p_frame_data->boards[boardNum].nrChannels - 1 - i];
  }
  baseline = baseline / 60.0;


  beam->Baseline = baseline;
  return;
}





template <typename T, typename std::enable_if<std::is_same<T, Fullframe>::value || std::is_same<T, Fullframe_float>::value, int>::type = 0>
void get_chi2(T *p_frame_data, Fullframe_float *calfac, int boardNum, beamRecon *beam, double noise_uncorrected = 0)
{



  double left_pointmm = (beam->Windowleft * 0.8 + floor(beam->Windowleft / 64) * 0.2);
  double right_pointmm = (beam->Windowright * 0.8 + floor(beam->Windowright / 64) * 0.2);


  // residual sum of squares
  double chi2 = 0;
  double temp_f = 0;
  double temp_err = 0;
  double pos_pointmm;
  for (int i = beam->Windowleft; i <= beam->Windowright; ++i)
  {
    pos_pointmm = (i * 0.8 + floor(i / 64) * 0.2);
    temp_f = beam->Peak * exp(-0.5 * (pos_pointmm - beam->Position) / beam->Sigma * (pos_pointmm - beam->Position) / beam->Sigma) ;
    temp_err = pow(calfac->boards[boardNum].data[i] * noise_uncorrected,2) + abs(p_frame_data->boards[boardNum].data[i])*calfac->boards[boardNum].data[i]; // the error is the noise_uncorrected times the calibration factor  + abs(y)
    //  cout << i << " " <<temp_f << endl;
    chi2 += pow((temp_f - p_frame_data->boards[boardNum].data[i]), 2)/ temp_err;
  }

  beam->Chi2 = chi2/(beam->Windowright - beam->Windowleft + 1 - 3); // average chi2
  return;
}
