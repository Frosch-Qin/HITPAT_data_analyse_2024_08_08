
// the system's namespace
// include all system parameters

namespace HITNamespace
{
    std::string ALGO_INCODE; // fas or recon_hardware or recon_gravity_rms

    int THRESHOLD_b0;
    int CLUSTER_SIZE_b0;
    int THRESHOLD_b1;
    int CLUSTER_SIZE_b1;

    // the threshold for selecting the signal in the cluster
    int INCLUSTER_THRESHOLD_b0;
    int INCLUSTER_THRESHOLD_b1;


    void Initialize(std::string algo_incode, int threshold_b0, int cluster_size_b0, int incluster_threshold_b0, int threshold_b1, int cluster_size_b1, int incluster_threshold_b1)
    {
        ALGO_INCODE = algo_incode;
        THRESHOLD_b0 = threshold_b0;
        CLUSTER_SIZE_b0 = cluster_size_b0;
        INCLUSTER_THRESHOLD_b0 = incluster_threshold_b0;

        THRESHOLD_b1 = threshold_b1;
        CLUSTER_SIZE_b1 = cluster_size_b1;
        INCLUSTER_THRESHOLD_b1 = incluster_threshold_b1;

    }

}
