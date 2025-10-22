/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosGinger : public AChaosBase {
public:

	REAL seed, nx, ny;
	
	AChaosGinger(){}
	~AChaosGinger(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 3, 2);	
		if(params==NULL){		
			//init	
			seed = 0.82f;
			nx = 0.7f;
			ny = 1.2f;

			REAL p[3] = {seed,nx,ny};
			set(p);
		} else { set(params); }
	}

	void reset(){
		seed = iv[0];
		nx = iv[1];
		ny = iv[2];
	}
	
 	void calc(){ 		
		// ginger formulae
 		REAL x1,y1;

		y1 = nx;
		if (nx<0.) nx = -nx;
		x1 = 1. - ny - seed*nx;

		nx = x1;
		ny = y1;

		ov[0] = nx;
		ov[1] = ny;
 	}
};