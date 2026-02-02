// FrameTags.h
#pragma once

#include <string>
#include "../algo/HITNamespace.h"

#include "../algo/beamrecon.h"
#include "../algo/beamrecon.h"
#include "../algo/cluster_find.c"
#include "../algo/sum_peak.h"
#include "../algo/gaussian_fit.h"
#include "../algo/fas.h"
#include "../algo/fas_cluster.h"
#include "../algo/fas_3sigma.h"
#include "../algo/recon_grav_rms.c"
#include "../algo/recon_grav_rms_hd.c"
#include "../algo/simple_2by2.h"
#include "../algo/caruana.h"
#include "../algo/guo.h"
#include "../algo/rms_fitting.h"
#include "../algo/caruana_fitting.h"
// #include "../algo/for_register/recon_grav_rms_hd_reg.c"


struct FrameTags {

  long frame_index   = -1;
  bool beam_on    = false;
  bool BKG_SUB_ON = false;

  // when SpillID = -2, it belongs to groupA; 
  // when SpillID = -1, it belongs to groupB; 
  // when SpillID >= 0, it is the actual SpillID
  // see myPhD thesis Fig.7.6 for the defination of groupA, groupB and spillID
  int SpillID; 


  std::string to_string() const {
    return
      "frame=" + std::to_string(frame_index) +
      " has_signal=" + (boardTags[0].Cluster_num? "YES" : "NO") + 
      " SpillID= " + std::to_string(SpillID);
  }

  beamRecon boardTags[6]; // up to 6 boards
};
