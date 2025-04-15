/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosDuffing : public AChaosBase {
public:

	REAL a, b, w, t, dt, nx, ny;
	
	AChaosDuffing(){}
	~AChaosDuffing(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 7, 2);	
		if(params==NULL){	
			//init	
			a = 0.25f;
			b = 0.3f;
			w = 1.0f;
			nx = 0.0f;
			ny = 0.0f;
			dt = 0.01f;
			t = 0.0f;
			REAL p[7] = {a,b,w,nx,ny,dt,t};
			set(p);
		}  else { set(params); }
	
	}

	void reset(){
		a = iv[0];
		b = iv[1];
		w = iv[2];
		nx = iv[3];
		ny = iv[4];
		dt = iv[5];
		t = iv[6];
	}
	
 	void calc(){ 		
		t += dt;
		nx = ny;
		ny = nx-(nx*nx*nx)-(a*ny)+(b*COS(w*t));

		ov[0] = nx;
		ov[1] = ny;
 	}
};