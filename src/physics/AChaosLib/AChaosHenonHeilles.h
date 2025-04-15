/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosHenonHeilles : public AChaosBase {
public:

	REAL nx, ny, nxdot, nydot, e, dt;
	
	AChaosHenonHeilles(){}
	~AChaosHenonHeilles(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 5, 4);	
		if(params==NULL){	
			//init	
			nx = 1.0f;
			ny = 1.0f;
			nydot = 1.4f;
			e = 0.125f;
			dt = 0.02;
			REAL p[5] = {nx,ny, nydot,e,dt};
			set(p);
		}  else { set(params); }
	
	}

	void reset(){
		nx = iv[0];
		ny = iv[1];
		nydot = iv[2];
		e = iv[3];
		dt = iv[4];
	}
	
 	void calc(){ 		
		// henon_heilles_calc
 		REAL xdd,ydd;

		xdd = -nx-(2*nx*ny);
		ydd = ny*ny -ny -(nx*nx);
		nxdot += xdd*dt;
		nydot += ydd*dt;
		nx += nxdot*dt;
		ny += nydot*dt; 

		ov[0] = nx;
		ov[1] = ny;
		ov[2] = nxdot;
		ov[3] = nydot;
 	}
};