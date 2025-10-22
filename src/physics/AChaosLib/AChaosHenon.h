/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosHenon : public AChaosBase {
public:

	REAL a, b, nx, ny;
	
	AChaosHenon(){}
	~AChaosHenon(){}	

	void setup(REAL * params = NULL){
		AChaosBase::setup(params, 4,2);
		if(params==NULL){
			//init	
			a = 1.4f;
			b = 0.3f;
			nx = 1.0f;
			ny = 1.0f;

			REAL p[4] = {a,b,nx,ny};
			set(p);
		} else {
			set(params);
		}
	}

	void reset(){
		a = iv[0];
		b = iv[1];
		nx = iv[2];
		ny = iv[3];
	}
	
 	void calc(){ 		
		// henon 		
		nx = (ny + 1) - (a * (nx*nx));
		ny = b * nx;

		ov[0] = nx;
		ov[1] = ny;
 	}
};