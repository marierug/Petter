//
//  getDataCSV.cpp
//  readCSV
//
//  Created by Petter Engblom Nordby on 19/11/2020.
//

#include "2021getDataCSV.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

namespace decomposition{

int getDataCSVfile5D(const string filepath, vector<double> &w, vector<vector<vector<vector<vector<double>>>>> &m,const int params,const int timesteps,const int days,const int seasons,bool printFile){
    //Opening file
    ifstream file;
    file.open(filepath);
    if (!file.is_open()){
            cout<<"Error opening file"<<endl;
            return -1;
        }
    if (printFile){
        cout <<"CSV input file printing: "<<endl;
    }   

    // Initializing indices
    int p = 0; int t = 0; int d = 0; int s = 0; int h = 0;
    string cell; string line;
    int it = 0; //

    getline(file, line);
    if (printFile){
        cout <<line<<endl;
    } 
    getline(file, line);
    if (printFile){
        cout <<line<<endl;
    }
    getline(file, line);
    if (printFile){
        cout <<line<<endl;
    }
    getline(file, line);
    if (printFile){
        cout <<line<<endl;
    }
   
    //Resize output 5d vector
    m.resize(params,vector<vector<vector<vector<double>>>>(timesteps,vector<vector<vector<double>>>(days,vector<vector<double>>(seasons))));
    
    //Creating probability 2-D vector; scenarios, days*params
    vector<vector<vector<double>>> paramProb;
    paramProb.resize(params,vector<vector<double>>>(days));

    
    
    //File operations
    while (getline(file,line)){
        if (printFile){
            cout <<line<<endl;
        }
        t=0; 
        it=0;
        stringstream ss(line);
        while (getline(ss,cell,'\t')){
            it++;
            if(it>6){
                m[p][t][d][s].push_back(stod(cell));
                t++;
            }
            else if(it==1 && h<stoi(cell)){
                h=stoi(cell);
//                prob.push_back(1);
//                prob[h-1]=1;
            }
            else if(it==2){d=stoi(cell)-1;}
            else if(it==3){p=stoi(cell)-1;}
            else if(it==4){s=stoi(cell)-1;}
            else if(it==5){continue;}
            else if(it==6){
                cout <<h;
                if (s==0) // only for s=0 as the seasons share the scenario tree
                {
                    paramProb[p][d].push_back(stod(cell));
                }
//                cout <<h;
//                cout<<"::"<<stod(cell)<<"::"<<prob[h-1]<<"::"<<endl;
//                prob[h]*=stod(cell);
//                cout<<"::"<<stod(cell)<<"::"<<prob[h-1]<<"::"<<endl;
            }
        }
    }
    //check file specs vs user specified specs
    if(params != p+1 || timesteps != t+1 || days != d+1 || seasons != s+1)
    {
        cout <<"Warning: File does not match user input"<<endl;
        cout << "User input: p t d s \t"<< params<<" "<<timesteps<<" "<<days<<" "<<seasons<<endl;
        cout << "File input: p t d s \t"<< p<<" "<<t<<" "<<d<<" "<<s<<endl;
    }

    int numScen = h;
    double probSum=0;
    
    vector<double>prob;
    prob.resize(numScen);
    for(int h=0;h<numScen;h++){
        prob[h]=1;
        for (int p=0;p<params;p++){
            for (int d =0;d<days;d++){
                    prob[h]*=paramProb[p][d][h];
            }
        }
        probSum += prob[h];
    }
    
    w.resize(numScen);
    for (int i=0;i<numScen;i++){
        w[i]=prob[i]/probSum;
    }

    file.close();
     if (file.is_open()){
            cout<<"Error closing file"<<endl;
            return -1;
        }
    
    return numScen;
}

//3d vector of paramrealizations: Several parameters and timesteps
int getDataCSVfile3D(const string filepath, vector<double> &w, vector<vector<vector<double>>> &m,const int params,const int timesteps,bool printFile){
    

    //Resize output 3d vector
    m.resize(params,vector<vector<double>>(timesteps));
    
    // Initializing indices
    int p = 0; int t = 0; int s = 0;
    string cell; string line;
    int i; //
    
    //Opening file
    ifstream file;
    file.open(filepath);
    if (!file.is_open()){
            cout<<"Error opening file"<<endl;
            return -1;
        }
    if (printFile){
        cout <<"CSV input file printing: "<<endl;
    }
    //File operations
    while (getline(file,line)){
        if (printFile){
            cout <<line<<endl;
        }
        i=0; p=0; t=0;
        stringstream ss(line);
        while (getline(ss,cell,'\t')){
            if(i!=0){
                m[p][t].push_back(stod(cell));
                t++;
                if(t%timesteps==0){
                    p++;
                    t=0;
                }
            }else{
                w.push_back(stod(cell));
                i++;
            }
        }
        s++;
    }
    
    int numScen = static_cast<int>(w.size());
    file.close();
     if (file.is_open()){
            cout<<"Error closing file"<<endl;
            return -1;
        }
    
    return numScen;
}

//1d-vector of param realizations; 1 param, 1 timestep
int getDataCSVfile1D(const string filepath, vector<double> &w, vector<double> &m,const int params,const int timesteps,bool printFile){
    
    
    // Initializing indices and counts
    int s = 0;
    string cell; string line;
    int i; //cellCount
    int lineCount=0;
    
    //Opening file
    ifstream file;
    file.open(filepath);
    if (!file.is_open()){
            cout<<"Error opening file"<<endl;
            return -1;
        }
    if (printFile){
        cout <<"CSV input file printing: "<<endl;
    }
    //File operations
    while (getline(file,line)){
        if (printFile){
                cout <<line<<endl;
            }
        if (lineCount > 3){          
            i=0;
            stringstream ss(line);
            while (getline(ss,cell,'\t')){
                if(i!=0){
                    m.push_back(stod(cell));
                    
                }else{
                    w.push_back(stod(cell));
                }
                i++;
            }
            s++;
        }
        lineCount++;
    }
    cout << "-------------------------------------"<<endl;
    cout <<"File Read: "<<endl;
    for (int h=0;h<3;h++){
        cout <<w[h]<<"\t"<<m[h]<<endl;
    }
    cout << "-------------------------------------"<<endl;
    int numScen = static_cast<int>(w.size());
    file.close();
    if (file.is_open()){
            cout<<"Error closing file"<<endl;
            return -1;
        }
   
    
    return numScen;
}

}
