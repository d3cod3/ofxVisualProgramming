/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosLogistic : public AChaosBase {
public:

	REAL seed, lambda;
	
	AChaosLogistic(){}
	~AChaosLogistic(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 2, 1);	
		if(params==NULL){	
			//init	
			seed = 0.777f;
			lambda = 3.9f;
			REAL p[2] = {seed,lambda};
			set(p);
		}  else { set(params); }
	}

	void reset(){
		seed = iv[0];
		lambda = iv[1];
	}
	
 	void calc(){ 	
 		seed = lambda * seed * (1.0-seed);

		ov[0] = seed;		
 	}
};