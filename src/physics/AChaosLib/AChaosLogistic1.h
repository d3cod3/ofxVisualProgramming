/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosLogistic1 : public AChaosBase {
public:

	REAL seed, lambda, gamma;
	
	AChaosLogistic1(){}
	~AChaosLogistic1(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 3, 1);	
		if(params==NULL){	
			//init	
			seed = 0.777f;
			lambda = 3.9f;
			gamma = 3.43f;
			REAL p[3] = {seed,lambda,gamma};
			set(p);
		}  else { set(params); }
	}

	void reset(){
		seed = iv[0];
		lambda = iv[1];
		gamma = iv[2];
	}
	
 	void calc(){ 	
 		seed = (seed*lambda) -(gamma*seed*seed);

		ov[0] = seed;		
 	}
};