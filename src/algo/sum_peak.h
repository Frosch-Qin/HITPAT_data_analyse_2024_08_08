//the algorithm for calculate gaussian peak, position and width.
//sum signal, find peak and do the division

template<typename T>
void sum_peak_fpga(T* p_frame_data, int boardNum, beamRecon *beam){


  int32_t amp=0;//per channel
  int32_t sum=0;
  int32_t position=0;
  int32_t peak=0;
  int32_t sigma=0;


  for(int i=0; i<p_frame_data->boards[boardNum].nrChannels; ++i){
    amp=p_frame_data->boards[boardNum].data[i];
    sum+=amp;
    if (amp>peak) {
      peak=amp;
      position=i;
    }
  }
  /*
  0.398942=1/sqrt(2*TMath::Pi())
  0.398942*16=6.3830720 
  here sum/peak multiple 6 and divide by 16 is the approximate number of 0.398942
  */
  
  //give one more digit accuracy
  if(peak !=0)
    sigma=((sum/peak*6)>>3);
  //here int is changed to double
  beam->Sigma=sigma/2.;
  beam->Position=position;
  beam->Peak=peak;
  //beam->Sum=sum;
  //beam->n_channels=p_frame_data->boards[boardNum].nrChannels;
  
}

template<typename T>
void sum_peak_c(T* p_frame_data, int boardNum, beamRecon *beam){

  float amp=0;//per channel
  float sum=0;
  float position=0;
  float peak=0;
  float sigma=0;


  for(int i=0; i<p_frame_data->boards[boardNum].nrChannels; ++i){
    amp=p_frame_data->boards[boardNum].data[i];
    sum+=amp;
    if (amp>peak) {
      peak=amp;
      position=i;
    }
  }

  //0.398942=1/sqrt(2*TMath::Pi())
  if(peak !=0)
    sigma=sum/peak*0.398942;
  beam->Sigma=sigma;
  beam->Position=position;
  beam->Peak=peak;
  //beam->Sum=sum;
 // beam->n_channels=p_frame_data->boards[boardNum].nrChannels;
}

