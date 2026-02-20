#pragma once

// the gaussian fitting for reconstructing peak, position and width
// #define WINDOW_LENTH 40 //40 channels

template <typename T>
void gaussian_fit(T *p_frame_data, int boardNum, beamRecon *beam)
{

  // int WINDOW_LENTH = 40; // carefull this is not a globel variable

  // int32_t amp = 0; // per channel
  // int32_t position = 0;
  // int32_t peak = 0;
  // int32_t focus = 0;

  // // get the max set the windows
  // int max_position = 0;
  // float max_amp = 0;
  // int window = WINDOW_LENTH; // set 40 channel as window width.
  // for (int i = 0; i < p_frame_data->boards[boardNum].nrChannels; ++i)
  // {
  //   if (p_frame_data->boards[boardNum].data[i] > max_amp)
  //   {
  //     max_amp = p_frame_data->boards[boardNum].data[i];
  //     max_position = i;
  //   }
  // }
  // int window_low = ((max_position - window / 2) > 0) ? (max_position - window / 2) : 0;
  // int window_high = ((max_position + window / 2) < p_frame_data->boards[boardNum].nrChannels) ? (max_position + window / 2) : p_frame_data->boards[boardNum].nrChannels;

  // TGraph *g = new TGraph();
  // int j = 0;
  // for (int i = window_low; i < window_high; ++i)
  // {
  //   //  if (p_frame_data->boards[boardNum].data[i]>20){
  //   g->SetPoint(i - window_low, i, p_frame_data->boards[boardNum].data[i]);
  //   //	j++;}
  // }

  // TF1 *fitFunc = new TF1("fitFunc", "gaus", 0, p_frame_data->boards[boardNum].nrChannels);
  // //  fitFunc->SetParLimits(1, 10, p_frame_data->boards[boardNum].nrChannels-10);   // Limit the mean parameter
  // //  fitFunc->SetParLimits(2, 0, 10);  // Limit the sigma parameter
  // g->Fit(fitFunc, "RQ"); //"R" for use the range for fitFunc; "B" for use the SetParLimits;"Q" for quite mode

  // beam->Peak = fitFunc->GetParameter(0);
  // beam->Position = fitFunc->GetParameter(1);
  // // cout << beam->Position << endl;
  // beam->Sigma = fitFunc->GetParameter(2);
  // // beam->n_channels=p_frame_data->boards[boardNum].nrChannels;

  // delete fitFunc;
  // delete g;

  double threshold=1;
  // if (boardNum == 0)
  //   threshold = HITNamespace::INCLUSTER_THRESHOLD_b0;
  // else if (boardNum == 1)
  //   threshold = HITNamespace::INCLUSTER_THRESHOLD_b1;
  
  TGraph *g = new TGraph();
  int j = 0;
  //double baseline_ch30 = beam->Noise_mean;
  double baseline_ch30 = 0;
  for (int i = beam->Windowleft; i < (1 + beam->Windowright); i++)
  {
    if (p_frame_data->boards[boardNum].data[i] > threshold)
    {
      g->SetPoint(j, i * 0.8 + floor(i / 64) * 0.2, p_frame_data->boards[boardNum].data[i]-baseline_ch30);
      j++;
    }
  }

  double windowleft_mm = beam->Windowleft * 0.8 + floor(beam->Windowleft / 64) * 0.2;
  double windowright_mm = beam->Windowright * 0.8 + floor(beam->Windowright / 64) * 0.2;

  // TF1 *fitFunc = new TF1("fitFunc", "gaus", windowleft_mm, windowright_mm);
  //  fitFunc->SetParLimits(1, 10, p_frame_data->boards[boardNum].nrChannels-10);   // Limit the mean parameter
  //  fitFunc->SetParLimits(2, 0, 10);  // Limit the sigma parameter
  TF1 *fitFunc = new TF1("fitFunc", "gaus", windowleft_mm, windowright_mm);
  fitFunc->SetParameters(100, 20, 5);
  fitFunc->SetParLimits(1, 0, 256);  // Limit the mean parameter
  fitFunc->SetParLimits(2, 0, 50); // Limit the sigma parameter
  g->Fit(fitFunc, "RQ");             //"R" for use the range for fitFunc; "B" for use the SetParLimits;"Q" for quite mode

  beam->Peak = fitFunc->GetParameter(0);
  beam->Position = fitFunc->GetParameter(1);
  beam->Sigma = fitFunc->GetParameter(2);

  delete fitFunc;
  delete g;
}
