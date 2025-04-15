/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"
#define pie 3.14159265358979323846

class AChaosStein : public AChaosBase {
public:

	REAL seed, lambda;
	
	AChaosStein(){}
	~AChaosStein(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 2, 1);	
		if(params==NULL){	
			//init	
			seed = 0.777f;
			lambda = 1.7f;

			REAL p[2] = {seed, lambda};
			set(p);
		} else { set(params); }
	}


	void reset(){
		seed = iv[0];
		lambda = iv[1];
	}
	
 	void calc(){ 	
 		seed = lambda * SIN(pie*seed);

		ov[0] = seed;		
 	}
};