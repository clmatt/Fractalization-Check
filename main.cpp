//using namespace std;
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <limits> 
#include <queue>
#include <tuple>
#include <fusion.h>
#include <omp.h>
#include <threads.h>
#include <boost/functional/hash.hpp>



//using namespace std::chrono;
//using namespace mosek::fusion;
//using namespace monty;


//My headers
#include "general.h"
#include "fractions.h"
#include "simplex.h"
#include "flags.h"


//Nauty
extern "C" {
	#include "nauty.h"   
	#include "naututil.h"
	#include "gtools.h"
}

//This is going to be extremely confusing, but graph is from Nauty and Graph is from me


int main() {
	//omp_set_num_threads(1); //Use for debugging
	auto start=std::chrono::high_resolution_clock::now();
	
	//Change this	
	const int n = 8;
	std::vector<std::vector<int> > num(14, std::vector<int>(14,0));
	
	
	/*num[6][6] = 2; //n = 5
	num[7][6] = 2;
	num[7][7] = 4;
	num[8][6] = 1;
	num[8][7] = 3;
	num[8][8] = 8;
	num[9][6] = 1;
	num[9][7] = 3;
	num[9][8] = 8;
	num[9][9] = 16;*/
	
	/*num[7][7] = 2; //n = 6
	num[8][7] = 1;
	num[8][8] = 4;
	num[9][7] = 1;
	num[9][8] = 3;
	num[9][9] = 8;
	num[10][7] = 1;
	num[10][8] = 3;
	num[10][9] = 7;
	num[10][10] = 16;*/
	
	/*num[8][8] = 2; //n = 7
	num[9][8] = 1;
	num[9][9] = 4;
	num[10][8] = 1;
	num[10][9] = 3;
	num[10][10] = 8*/;
	
	num[9][9] = 2; //n = 8
	num[10][9] = 1;
	num[10][10] = 4;
	num[9][11] = 1;
	num[10][11] = 3;
	num[11][11] = 8;
	num[12][9] = 1;
	num[12][10] = 2;
	num[12][11] = 6;
	num[12][12] = 16;
	num[13][9] = 1;
	num[13][10] = 2;
	num[13][11] = 5;
	num[13][12] = 13; //13 = 32 - floor(8*32/13) remove vertex in below average number of copies
	num[13][13] = 32; 
	
	
	std::vector<Graph> allGraphs = generate(n,2);
	std::vector<Graph> potentialFractalizers;
	
	std::cout << "Starting number of graphs: " << allGraphs.size() << std::endl;
	
	//Remove all graphs with too many edges (could just use complement)
	for(int i = 0; i < (int)allGraphs.size(); ++i) {
		if(allGraphs[i].getCanonLabel() < allGraphs[i].complement().getCanonLabel()) {
			allGraphs.erase(allGraphs.begin() +i);
			--i;
		}
	}
	
	std::cout << "After checking that they have few enough edges: " << allGraphs.size() << std::endl;
	
	//Remove all graphs with twins
	for(int i = 0; i < (int)allGraphs.size(); ++i) {
		if(allGraphs[i].containsTwins()) {
			allGraphs.erase(allGraphs.begin() + i);
			--i;
		}		
	}
	
	std::cout << "After removing all graphs with twins: " << allGraphs.size() << std::endl;
	
	//Remove all graphs which aren't connected
	for(int i = 0; i < (int)allGraphs.size(); ++i) {
		if(!allGraphs[i].connected()) {
			allGraphs.erase(allGraphs.begin()+i);
			--i;
		}
	}
	
	std::cout << "After removing all graphs who are not connected: " << allGraphs.size() << std::endl;
	
	//Remove all graphs whose complements aren't connected
	for(int i = 0; i < (int)allGraphs.size(); ++i) {
		if(!allGraphs[i].complement().connected()) {
			allGraphs.erase(allGraphs.begin()+i);
			--i;
		}
	}
	
	std::cout << "After removing all graphs whose complements are not connected: " << allGraphs.size() << std::endl;
	
	for(int i = 0; i < (int)allGraphs.size(); ++i) {
		Graph comp = allGraphs[i].complement();
		if(allGraphs[i].getNumEdges() > comp.getNumEdges()) {
			allGraphs[i] = comp;
		}
	}
	
	for(int i = n+1; i <= 13; ++i) { //CHANGE THIS
		int temp = 1;
		std::cout << std::endl;
		std::vector<Graph> newAllGraphs;
		
		for(int j = 0; j < allGraphs.size(); ++j) {
			std::cout << "(" << i << ", " << j << ") out of (12, " << allGraphs.size() << ")" << std::endl;
			std::vector<Graph> S = {allGraphs[j]};
			bool check = true;
			
			
			for(int k = n+1; k <= i; ++k) {
				S = expandGraphs(S,{});
				std::vector<Graph> newS;
				#pragma omp parallel shared(check)
				{
					std::vector<Graph> privateS;
					//Needs enough subgraphs
					#pragma omp for nowait 
					for(int l = 0; l < S.size(); ++l) {
						if(check) {
							int temp = numSubgraphs(allGraphs[j], S[l]);
							
							if((temp > num[i][k]) && (k == i)) {
								check = false;
							}
							
							else if(temp >= num[i][k]) {
								privateS.push_back(S[l]);
							}
						}
					}
					
					#pragma omp critical
					{
						newS.insert(newS.end(), privateS.begin(), privateS.end());
					}
				}
				
				S = newS;
				//if(!check) {
					//std::cout << "TEST";
				//}
			}
			
			/*for(int k = n+1; k <= i; ++k) {
				S = expandGraphs(S,{});
				
				//Needs enough subgraphs
				for(int l = 0; l < S.size(); ++l) {
					int temp = numSubgraphs(allGraphs[j], S[l]);
					
					if(temp < num[i][k]) {
						S.erase(S.begin() +l);
						--l;
					}
					
					if((temp > num[i][k]) && (k == i)) {
						l = S.size();
						check = false;
					}
				}
			}*/
			
			
			if(check) {
				#pragma omp parallel for shared(check)
				for(int k = 0; k < S.size(); ++k) {			
					if(check) {
						std::pair<int,int> noTwins;
						noTwins.first = -1;
						noTwins.second = -1;
					
						std::pair<int,int> twins = S[k].findTwins();
						while(twins != noTwins) {
							S[k].removeVertex(twins.first);
							twins = S[k].findTwins();
						}
					
						if(S[k].getN() != n)  {						
							check = false;
						}
					}
				}
			}
			
			if(check) {
				newAllGraphs.push_back(allGraphs[j]);
				std::cout << temp << std::endl;
				++temp;
			}
		}
		
		allGraphs = newAllGraphs;
		
		for(int j = 0; j < allGraphs.size(); ++j) {
			allGraphs[j].printEdges();
		}
	}
	
	std::cout << "Total number of potential fractalizers = " << allGraphs.size() << std::endl << std::endl;
	
	

	auto end=std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
	std::cout << "Running time in seconds: " << duration.count() << std::endl << std::endl;
	
	return 0;
}

