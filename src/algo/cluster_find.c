

bool cluster_locate(Fullframe *p_frame_data,int boardNum , beamRecon *beam, int threshold=18, int cluster_size=4)
{

    int cluster_num = 0;    
    std::vector<int> above_TH_position;
    std::vector<int> cluster_position;
    // std::cout <<"number of channels: " << p_frame_data->boards[boardNum].nrChannels << std::endl;
    for (int i = 0; i < p_frame_data->boards[boardNum].nrChannels; ++i)
    {
        
        if (p_frame_data->boards[boardNum].data[i] > threshold)
        {
            above_TH_position.push_back(i);
            // std::cout << p_frame_data->boards[boardNum].data[i] << " above " << threshold << " " << i << std::endl;
        }
    }

    if (above_TH_position.size() >(cluster_size - 1))
    {
        int length = 0;
    
        for (int i = 0; i<above_TH_position.size(); i=i+length+1){
            length = 0;
            for (int j = i; j < above_TH_position.size()-1; j++)
            {

                if ((above_TH_position.at(j) + 1) == above_TH_position.at(j + 1))
                    length++;
                else
                    break;
            }
            if (length > cluster_size-2) //2024.DEC.17 change cluster_size-1 to cluster_size-2
            {
                cluster_position.push_back(above_TH_position.at(i));
                // std::cout<<"abv "<< above_TH_position.at(i)<<std::endl;
                // std::cout<<"abv "<< above_TH_position.at(i+length)<<std::endl;

                cluster_position.push_back(above_TH_position.at(i + length)); 
                cluster_num++;
            }

        }
 
    }
    
    // std::cout << cluster_num << " " << cluster_position.front() << " " << cluster_position.back() <<"cluster" <<std::endl;
    if (cluster_num>0)
    {
        beam->Cluster_num = 1;
        beam->Windowleft = cluster_position.front();
        beam->Windowright = cluster_position.back();
        return true;
    }
    else
    {

      //  std::cout << Form("there is no cluster with %d channels above %d", cluster_size, threshold) << std::endl;
        beam->Cluster_num = 0;
        beam->Windowleft = 0;
        beam->Windowright = 0;
        return false;

    }

}