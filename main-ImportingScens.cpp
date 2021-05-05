//
//  main.cpp
//  GOSSIPtools
//
//  Created by Petter Nordby on 01/04/2021.
//

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream> 


using namespace std;


int main() {
    // program set-up
    int numP = 4;
    int numT = 24;
    int numS = 3;
    bool printFile = true;
    
    // initializing
    string param = "param0";
    string paramName;
    int t=0;
    int h=0;
    int it_line=0;
    int it_cell=0;
    string filepath;
    ifstream file;
    string cell;
    string line;
    map<string,vector<vector<double>>> uncertainParams;
    vector<vector<double>> temp(numT,vector<double>(numS));
    vector<double>probabilities;
    //Scenario probabilities from csv to program
    file.open("probabilities.csv");
    if (!file.is_open())
    {
        cout<<"Error opening file for probabilities"<<endl;
        return -1;
    }
    while(getline(file, line))
    {
        if(printFile){ cout<<line<<endl;}
        if(it_line!=0)
        {
            stringstream ss(line);
            while (getline(ss,cell,';'))
            {
                if(it_cell == 1)
                {
                    probabilities.push_back(stod(cell));
                }
                it_cell++;
            }
        }
        it_line++;
        it_cell=0;
        
    }
    file.close();
    if (file.is_open())
    {
        cout<<"Error closing file for probabilities"<<endl;
        return -1;
    }
    //uncertain parameter values from csv to program
    for (int i=0;i<numP;i++)
    {
        it_line =0;
        //opening file
        param.pop_back();
        param+=to_string(i+1);
        filepath = param+".csv";
        file.open(filepath);
        if (!file.is_open())
        {
            cout<<"Error opening file for "<<param<<endl;
            return -1;
        }
        
        //reading file and updating map
        if (printFile){cout << param <<":"<<endl;}
        
        while(getline(file, line))
        {
            if (printFile){cout <<line<<endl;}
            if(it_line!=1)
            {
                stringstream ss(line);
                while (getline(ss,cell,';'))
                {
                    
                    if(it_line==0 && it_cell==1)
                    {
                        paramName=cell;
                        uncertainParams.insert(pair<string,vector<vector<double>>>(paramName,temp));
                    }
                    else if (it_line>1 && it_cell==0)
                    {
                        t = stoi(cell);
                    }
                    else if(it_line>1 && it_cell>0 )
                    {
                        uncertainParams[paramName][t-1][it_cell-1] = stod(cell);
                    }
                    it_cell++;
                }
            }
            it_line++;
            it_cell=0;
        }
        file.close();
        if (file.is_open())
        {
            cout<<"Error closing file for "<< paramName<<endl;
            return -1;
        }
    }
    
    cout <<"-------printing data---------"<<endl;
    cout<<"probabilities: "<<endl;
    for (int h=0;h<numS;h++){
        cout << "scen "<< h<<": "<< probabilities[h]<<endl;
    }
    
    map<string,vector<vector<double>>>::iterator it_map;
    for (it_map = uncertainParams.begin(); it_map != uncertainParams.end(); it_map++)
    {
        cout << it_map->first << ":"<<endl;   // string (key)
        for (int h=0;h<numS;h++)
        {
            cout << " scen: "<<h+1<<endl;
            for (int t=0;t<numT;t++)
            {
                cout <<" "<< it_map->second[t][h]<< " ";  // string's value
            }
            cout << endl;
        }
    }
    
    cout<<uncertainParams["IR_PV"][23][1]<<endl;
    
    return 0;
}
