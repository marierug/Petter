//
//  getDataCSV.hpp
//  readCSV
//
//  Created by Petter Engblom Nordby on 19/11/2020.
//

#ifndef getDataCSV_hpp
#define getDataCSV_hpp

#include <stdio.h>
#include <string>
#include <vector>

namespace decomposition{

int getDataCSVfile5D(const std::string filepath, std::vector<double> &w, std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> &m,const int params,const int timesteps,const int days, const int seasons, bool printFile);

int getDataCSVfile3D(const std::string filepath, std::vector<double> &w, std::vector<std::vector<std::vector<double>>> &m,const int params,const int timesteps,bool printFile);

int getDataCSVfile1D(const std::string filepath, std::vector<double> &w, std::vector<double> &m,const int params,const int timesteps,bool printFile);

} 
#endif /* getDataCSV_hpp */

