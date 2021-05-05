 #include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include "definitions.hpp"
#include "CompGraph.hpp"
#include "GenerateScenarios.hpp"
#include "2021getDataCSV.hpp"
#include <cmath>

#include "inputmodel.hpp"

using namespace std; 

int inputmodel(vector<double> &probabilities)
{

//--------------------------------------------------------------------//
//Controls
	bool setDesignsManually = true;
	vector<double> setDesign{7-1,6-1,2-1}; 

	bool useBattery = false;

	bool expectedValueProblem = false;

	bool printFile =true;
	string uncertiantyFolder = "Examples/GOSSIP_library/RES_master/Petter_files/UncertaintyData/";


//--------------------------------------------------------------------//		
// Input data and program set-up
	int numScen =0;							// Number of Scenarios [-]
	int numTimesteps 	= 24;				// Number of timesteps [-]
	double deltaT		= 1;				// length of a timestep [h]
	int numParams		= 4;		// NUmber of uncertain parameters
	int numTech 		= 3;		//1: PV
									//2: WT
									//3: BAT

	int concount	= -1;
	int varcount	= -1;
	char clabel[75];

	 
	//--------------------------------------------------------------------//
	// Cost Parameters
	double r = 0.06; 	//Interest rate 
	vector<double> L{30,20,15}; //Lifetime
	vector<double> C{130,2000000,150000}; //Linear cost per unit size $/{m²,no.,Mwh}
	vector<double> xi{0.05,0.05,0.02};	//Maintenance factor 

	double budget = 15*1000000; // M$

	//
	//--------------------------------------------------------------------//
	// Discretization params
	vector<int> numDiscrete{10,10,10}; //number of discrete design values
	vector<double> Z_UB {45000,9,45}; //Upper bounds {m²,no.,Mwh}
	vector<double>Z_LB{0,0,0}; // Lower bounds {m²,no.,Mwh}

	//--------------------------------------------------------------------//
	// Model Parameters: Grid
	double FiT 			= 100;		// Renewabel Feed-in-tariff $/MWh
	double FiT_extra	= 0.5*FiT;	// Earnings for feeding extra power into grid demand. 
	//--------------------------------------------------------------------//
	// Model Parameters: Renewable power gen
	double eta_PV = 0.2; //Efficiency PV

	double q_d 	= 1;	// MW Rated power for wind turbine
	double W_d = 12;	//Rated wind velocity m/s
	double W_min = 3.5; // Cut-in wind velocity m/s
	double W_max = 25; 	// Cut-off wind velocity m/s

	//--------------------------------------------------------------------//
	// Model Parameters: battery 		//Nom
	double SoC_initial	= 50;			//50;	// Initial state of charge of battery %
	double SoC_min 		= 10;			//10;	// min soc %
	double SoC_max 		= 90;			//90;	// max soc %
	double eta_ES_storage	= 0.99993;	//0.99993;		// Representing eergy storage self discharge losses												// Based on self-discharge loss of 5% per month
	double eta_ES_ch		= 0.95;		//0.95;		// Energy storage charging losses
	double eta_ES_dis		= 0.95;		//0.95;	// Energy storage efficiency losses per hour
	double C_rate_max		= 0.25;		//0.25;

	//Printing key constant params
	cout<<"Key params:"<<endl;
	cout<< "SOC_initial;"<<SoC_initial<<endl;
	cout<< "FiT;" << FiT<<endl;
	cout<< "FiT_extra;"<<FiT_extra<<endl;
	cout<< "Budget;"<<budget<<endl;
	cout<< "C_PV;"<<C[0]<<endl;
	cout<< "C_WT;"<<C[1]<<endl;
	cout<< "C_BAT;"<<C[2]<<endl;

//--------------------------------------------------------------------//
// Bounds
double bound_PV = Z_UB[0]*eta_PV*1000/1000000*1.1; //MW, 10% safety
double bound_WT = Z_UB[1]*q_d*1.1;
double bound_BAT = Z_UB[2]*C_rate_max*1.1;
double bound_DEM = 6;

//--------------------------------------------------------------------//
// Importing scenarios and uncertain data from csv-file
	 // initializing
    string param = "param0"; string paramName;
    int t=0;        
    int it_line=0;   int it_cell=0;
    ifstream file;   string filepath;
    string cell;     string line;
    map<string,vector<vector<double>>> uncertainParams;
   
    //Scenario probabilities from csv to program
    file.open(uncertiantyFolder+"probabilities.csv");
    if (!file.is_open())
    {
    	cout<<"Error opening file for probabilities"<<endl;
     	return -1;
    }
    while(getline(file, line))
    {
    	string comma(",");
        if(printFile){ cout<<line<<endl;}
        if(it_line!=0)
        {
            stringstream ss(line);
            while (getline(ss,cell,';'))
            {
                if(it_cell == 1)
                {
                	if (cell.find(comma) != string::npos)
                    {

                      	probabilities.push_back(stod(cell.replace(cell.find(comma),comma.length(),".")));
                   	}
                  	else
               		{
                   		probabilities.push_back(stod(cell));
                   	}
                    numScen++;
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
   // numScen = probabilities.size();
    vector<vector<double>> temp(numTimesteps,vector<double>(numScen));
    for (int i=0;i<numParams;i++)
    {
        it_line =0;
        //opening file
        param.pop_back();
        param+=to_string(i+1);
        filepath = uncertiantyFolder+param+".csv";
        file.open(filepath);
        string comma(",");
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
                    	if (cell.find(comma) != string::npos)
                    	{
                        	uncertainParams[paramName][t-1][it_cell-1] = stod(cell.replace(cell.find(comma),comma.length(),"."));
                   		}
                   		else
                   		{
                   			uncertainParams[paramName][t-1][it_cell-1] = stod(cell);
                   		}
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
    
    if (printFile)
    {cout <<"-------printing data---------"<<endl;
	    cout<<"prob: ";
	    for (int h=0;h<numScen;h++)
	    {
	        cout << "scen "<< h+1<<" "<< probabilities[h]<<endl;
	    }
	    
	    map<string,vector<vector<double>>>::iterator it_map;
	    for (it_map = uncertainParams.begin(); it_map != uncertainParams.end(); it_map++)
	    {
	        cout << "Param: "<<it_map->first<<endl;   // string (key)
	        for (int h=0;h<numScen;h++)
	        {
	            cout << " scen: "<<h+1<<endl;
	            for (int t=0;t<numTimesteps;t++)
	            {
	                cout <<" "<< it_map->second[t][h]<< " ";  // string's value
	            }
	            cout << endl;
	        }
	    }
    }
//--------------------------------------------------------------------//
//Expected value problem
    if(expectedValueProblem) 	//Alter the first scenario of uncertainParams and probability to be EVP
    {
    	cout<<"EVP running.. \n Calculating EVPs.."<<endl;
    	double avg;
    	map<string,vector<vector<double>>>::iterator it_map;
	    for (it_map = uncertainParams.begin(); it_map != uncertainParams.end(); it_map++)
	    {
	    	cout <<it_map->first<<": "<<endl;
	    	for (int t=0;t<numTimesteps;t++)
	   	    {	
	   	    	 avg=0;
	    	    for (int h=0;h<numScen;h++)
	        	{            
	                avg += probabilities[h]*it_map->second[t][h];  // string's value
	            }
	            it_map->second[t][0]=avg;
	            cout <<it_map->second[t][0]<<" ";
	        }
	        cout <<endl;
	    }
	    probabilities.erase(probabilities.begin()+1,probabilities.end());
	    probabilities[0] = 1;
	    numScen = 1;
    }

//--------------------------------------------------------------------//
// 1st stage variables and discretization
 
    vector<vector<double>> z_d(numTech);
    vector<vector<Variables>> z_b(numTech);
    for (int tech =0;tech<numTech;tech++)
    {
    	z_d[tech].resize(numDiscrete[tech]);
    	z_b[tech].resize(numDiscrete[tech]);
    }
    cout << "Discrete Sets of technologies:"<<endl;
	for (int tech=0;tech<numTech;tech++)
	{
		cout <<"Tech " <<tech+1<<": "<<endl;
		for (int n=0;n<numDiscrete[tech];n++)
		{	
			if(!useBattery && tech==2)
			{
				continue;
			}
			z_d[tech][n] = Z_LB[tech] +  n*Z_UB[tech]/(numDiscrete[tech]-1);
			cout << z_d[tech][n]<<"\t";

			sprintf(clabel, "z_b[%d][%d]",tech+1,n+1); 
			z_b[tech][n].setIndependentVariable(++varcount,compgraph::BINARY,I(0,1),0.,-1,clabel);
		}
		cout<<endl;
	}
	cout<<endl;

cout<<"-- Discretization performed.. --"<<endl;
//--------------------------------------------------------------------//
// 2nd Stage Variables

	//Basic structure
	vector<vector<Variables>>f_renewables(numTimesteps,
			vector<Variables>(numScen)); // [MW]

	vector<vector<Variables>>f_demand(numTimesteps,
			vector<Variables>(numScen)); // [MW]

	// PV
	vector<vector<Variables>>f_PV(numTimesteps,
			vector<Variables>(numScen)); // [MW]
	//WT
	vector<vector<Variables>>f_WT(numTimesteps,
			vector<Variables>(numScen)); // [MW]
	//grid
	vector<vector<Variables>>f_grid_export(numTimesteps,
			vector<Variables>(numScen)); // [MW]

	vector<vector<Variables>>f_grid_import(numTimesteps,
			vector<Variables>(numScen)); // [MW]
	//Battery
	vector<vector<Variables>>f_bat_charge(numTimesteps,
			vector<Variables>(numScen)); // [MW]
	vector<vector<Variables>>f_bat_discharge(numTimesteps,
			vector<Variables>(numScen)); // [MW]
	vector<vector<Variables>>E_BAT(numTimesteps,
			vector<Variables>(numScen)); // [%]

	for (int t=0;t<numTimesteps;t++)
	{
		for (int h=0;h<numScen;h++)
		{
			sprintf(clabel,"f_renewables[%d][%d]",t+1,h+1);
			f_renewables[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,(bound_WT+bound_PV)),0.,h+1,clabel);
			sprintf(clabel,"f_demand[%d][%d]",t+1,h+1);
			f_demand[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,bound_DEM),0.,h+1,clabel);
			sprintf(clabel,"f_PV[%d][%d]",t+1,h+1);
			f_PV[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,bound_PV),0.,h+1,clabel);
			sprintf(clabel,"f_WT[%d][%d]",t+1,h+1);
			f_WT[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,bound_WT),0.,h+1,clabel);
			sprintf(clabel,"f_grid_export[%d][%d]",t+1,h+1);
			f_grid_export[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,(bound_WT+bound_PV)),0.,h+1,clabel);
			sprintf(clabel,"f_grid_import[%d][%d]",t+1,h+1);
			f_grid_import[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,bound_DEM),0.,h+1,clabel);
			if(!useBattery)
			{
				continue;
			}
			sprintf(clabel,"f_bat_charge[%d][%d]",t+1,h+1);
			f_bat_charge[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,bound_BAT),0.,h+1,clabel);
			sprintf(clabel,"f_bat_discharge[%d][%d]",t+1,h+1);
			f_bat_discharge[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,bound_BAT),0.,h+1,clabel);
			sprintf(clabel,"E_BAT[%d][%d]",t+1,h+1);
			E_BAT[t][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(0,Z_UB[2]*1.1),0.,h+1,clabel);
		}
	}

	//Design decisions: Linking 1st stage to second stage
	vector<vector<Variables>>Z(numTech,vector<Variables>(numScen));
	for (int tech =0;tech<numTech;tech++)
	{
		if(!useBattery && tech ==2)
			{
				continue;
			}
		for (int h=0;h<numScen;h++)
		{
			sprintf(clabel,"Z[%d][%d]",tech,h);
			Z[tech][h].setIndependentVariable(++varcount,compgraph::CONTINUOUS,I(Z_LB[tech],Z_UB[tech]),0.,h+1,clabel);
		}
	}
cout<<"-- 2nd stage varaiables declared.. --"<<endl;

//---------------------------------------------------------------//
// Objective function: 
	
	vector<double> CRF(numTech);	// Capital recovery factor
	for (int tech=0;tech<numTech;tech++)
	{
		if(!useBattery && tech==2)
			{
				continue;
			}
		CRF[tech] = r*pow(r+1,L[tech])/(pow(r+1,L[tech])-1);
	}

	vector<Objective>obj(numScen);	// Total cost per scenario
	for (int h=0; h<numScen;h++)
	{ 	
		obj[h]	= 0;
		// Invest Cost
		for (int tech=0;tech<numTech;tech++)
		{
			if(!useBattery && tech==2)
			{
				continue;
			}
			cout<<" CRF+xi = "<< CRF[tech]+xi[tech]<<endl;
			obj[h] += Z[tech][h]*C[tech]*(CRF[tech]+xi[tech]); // annual investemnt + maintenance
		}
		//Operating cost (earnings)
		for (int t=0 ; t<numTimesteps ; t++)
		{	

			obj[h] += 365*uncertainParams["OC_GRID"][t][h]*f_grid_import[t][h]*deltaT;
			obj[h] -= 365*f_grid_export[t][h]*FiT_extra*deltaT;
			obj[h] -= 365*f_demand[t][h]*FiT*deltaT;			
			
		}
		obj[h].setDependentVariable(++concount,compgraph::OBJ,true,h+1);
	}

cout<<"-- Objective function set.. --"<<endl;

//--------------------------------------------------------------------//
//1st stage constraints 
	// Binary
	vector<Constraints> con_binary(numTech);
	for (int tech=0;tech<numTech;tech++)
	{
		if(!useBattery && tech==2)
			{
				continue;
			}
		con_binary[tech] = -1;
		for (int n=0;n<numDiscrete[tech];n++)
		{
			con_binary[tech] += z_b[tech][n];
		}
		con_binary[tech].setDependentVariable(++concount,compgraph::EQUALITY,false,-1);
	}

	// 1st stage binary variable --> 2nd stage continuous
	
	vector<vector<Constraints>> con_designs(numTech,vector<Constraints>(numScen));
	for (int tech=0;tech<numTech;tech++)
	{
		if(!useBattery && tech==2)
			{
				continue;
			}
		for (int h=0;h<numScen;h++)
		{
			con_designs[tech][h] = -Z[tech][h];
			for (int n=0;n<numDiscrete[tech];n++)
			{
				con_designs[tech][h] += z_b[tech][n]*z_d[tech][n];
			}
			con_designs[tech][h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);
		}
	}

	// Budget constraint

	vector<Constraints> con_budget(numScen);
	for (int h=0;h<numScen;h++)
	{
		con_budget[h] = -budget;
		for (int tech=0;tech<numTech;tech++)
		{
			if(!useBattery && tech==2)
			{
				continue;
			}
			con_budget[h] += Z[tech][h]*C[tech];
		}
		con_budget[h].setDependentVariable(++concount,compgraph::LEQ,true,h+1);
	}


//--------------------------------------------------------------------//
	//EEVP
	if(setDesignsManually)
	{
		vector<Constraints> con_set_designs(numTech);
		for (int tech=0;tech<numTech;tech++)
		{
			if(!useBattery && tech==2)
			{
				continue;
			}
			con_set_designs[tech] = -1;
			con_set_designs[tech] += z_b[tech][setDesign[tech]];

			con_set_designs[tech].setDependentVariable(++concount,compgraph::EQUALITY,false,-1);
		}
	}


cout<<"-- 1st stage constraints set.. --"<<endl;
//--------------------------------------------------------------------//
// Second stage constraints
// Declaring constraints
	//Energy hub
	vector<vector<Constraints>> con_energyhub_eb(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<vector<Constraints>> con_renewables_eb(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<vector<Constraints>> con_demand(numTimesteps,
		 	vector<Constraints>(numScen));


	// Renwable power generation
	vector<vector<Constraints>> con_PV(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<vector<Constraints>> con_WT(numTimesteps,
		 	vector<Constraints>(numScen));

	// Energy storage: Battery
	vector<vector<Constraints>> con_BAT_eb(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<vector<Constraints>> con_BAT_SoC_max(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<vector<Constraints>> con_BAT_ch_max(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<vector<Constraints>> con_BAT_dis_max(numTimesteps,
		 	vector<Constraints>(numScen));
	vector<Constraints> con_BAT_periodicity(numScen);

// Constraints
	for (int h=0;h<numScen;h++)
	{
		for (int t=0;t<numTimesteps;t++)
		{
			con_energyhub_eb[t][h] = 0;
			con_energyhub_eb[t][h] -= uncertainParams["L_DEM"][t][h];
			//con_energyhub_eb[t][h] -= f_demand[t][h];
			con_energyhub_eb[t][h] += f_renewables[t][h];
			con_energyhub_eb[t][h] += f_grid_import[t][h];
			// Eb2
			con_renewables_eb[t][h] = 0;
			con_renewables_eb[t][h] -= f_renewables[t][h];
			con_renewables_eb[t][h] -= f_grid_export[t][h];
			con_renewables_eb[t][h] += f_PV[t][h];
			con_renewables_eb[t][h] += f_WT[t][h];
			if(useBattery)
			{
				con_renewables_eb[t][h] -= f_bat_charge[t][h];
				con_renewables_eb[t][h] += f_bat_discharge[t][h];
			}	

			con_demand[t][h] = -f_demand[t][h];
			con_demand[t][h] += uncertainParams["L_DEM"][t][h];

			
			// Demand flow; uncertain parameter

			//PV
			con_PV[t][h] = -f_PV[t][h];
			con_PV[t][h] += eta_PV*uncertainParams["IR_PV"][t][h]*Z[0][h]/1000000;

/*			for (int n=0;n<numDiscrete[0];n++)
			{
				con_PV[t][h] += eta_PV*uncertainParams["IR_PV"][t][h]*z_b[0][n]*z_d[0][n];
			}
*/
			// Wind
			con_WT[t][h] = -f_WT[t][h];
			
			if((uncertainParams["W_WT"][t][h] >= W_min) && (uncertainParams["W_WT"][t][h] <= W_max))
			{
				if (uncertainParams["W_WT"][t][h] <= W_d)
				{

					con_WT[t][h] += Z[1][h]*q_d * (pow(uncertainParams["W_WT"][t][h],3) - pow(W_min,3))/(pow(W_d,3)-pow(W_min,3));
				}
				else
				{
					con_WT[t][h] += Z[1][h]*q_d;
				}
			}
			else
			{
				con_WT[t][h] += Z[1][h]*0;
			}
			
			if(!useBattery)
			{
				continue;
			}
			// Energy balance for battery
			con_BAT_eb[t][h] = -E_BAT[t][h];
			if (t != 0)
			{
				con_BAT_eb[t][h] += E_BAT[t-1][h]*eta_ES_storage;
			}
			else
			{
				con_BAT_eb[t][h] += eta_ES_storage*(SoC_initial-SoC_min)*Z[2][h]/100;
			}
			con_BAT_eb[t][h] += f_bat_charge[t][h] * eta_ES_ch* deltaT;
			con_BAT_eb[t][h] -= deltaT* f_bat_discharge[t][h]/ eta_ES_dis;

				// State of charge constraints
			con_BAT_SoC_max[t][h] = E_BAT[t][h];
			con_BAT_SoC_max[t][h] -= (SoC_max-SoC_min)*Z[2][h]/100;
/*			for (int n=0;n<numDiscrete[2];n++)
			{
				con_BAT_SoC_min[t][h] +=  z_b[2][n]*z_d[2][n]*SoC_min/100;
				con_BAT_SoC_min[t][h] +=  z_b[2][n]*z_d[2][n]*SoC_max/100;
			}
*/			
				//charging/discharging constraints
			con_BAT_ch_max[t][h] = -deltaT*C_rate_max*Z[2][h];
			con_BAT_dis_max[t][h] = -deltaT*C_rate_max*Z[2][h];
			con_BAT_ch_max[t][h] += f_bat_charge[t][h];
			con_BAT_dis_max[t][h] += f_bat_discharge[t][h];
		}
		if(!useBattery)
		{
			continue;
		}
		// Periodicity
		con_BAT_periodicity[h] = -E_BAT[numTimesteps-1][h];
		con_BAT_periodicity[h] += (SoC_initial-SoC_min)*Z[2][h]/100;
		/*for (int n=0;n<numDiscrete[2];n++)
		{
			con_BAT_periodicity[h] += z_b[2][n]*z_d[2][n]*SoC_initial/100;
		}*/
		
	}

	

	// Setting constraints
	for (int h=0;h<numScen;h++)
	{
		for (int t=0;t<numTimesteps;t++)
		{
			// Energy Hub
			con_energyhub_eb[t][h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);
			con_renewables_eb[t][h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);

			//Demand
			//con_demand[t][h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);

			//PV
			con_PV[t][h].setDependentVariable(++concount,compgraph::GEQ,true,h+1);
			
			// Wind
			con_WT[t][h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);

			// Battery energy storage
			if(!useBattery)
			{
				continue;
			}
				// Energy balance for battery
			con_BAT_eb[t][h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);

				// State of charge constraints
			con_BAT_SoC_max[t][h].setDependentVariable(++concount,compgraph::LEQ,true,h+1);
				//Flows
			con_BAT_ch_max[t][h].setDependentVariable(++concount,compgraph::LEQ,true,h+1);
			con_BAT_dis_max[t][h].setDependentVariable(++concount,compgraph::LEQ,true,h+1);
		}
		if(!useBattery)
		{
			continue;
		}
		//Periodicity
		con_BAT_periodicity[h].setDependentVariable(++concount,compgraph::EQUALITY,true,h+1);
	}

cout<<"-- 2nd stage constraints set.. --"<<endl;
cout<<"-- Running program.. --"<<endl;


	return numScen;

}