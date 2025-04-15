/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

class AChaosIkeda : public AChaosBase {
public:

	REAL a, b, k, p, nx, ny; 
	
	AChaosIkeda(){}
	~AChaosIkeda(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 6, 2);	
		if(params==NULL){	
			//init	
			a=0.85;
			 b=0.9; 
			 k=0.4;
			 p=7.7; 
			 nx=0; 
			 ny=0; 
			REAL pa[6] = {nx,ny,a,b,k,p};
			set(pa);
		}  else { set(params); }
	
	}

	void reset(){
		nx = iv[0];
		ny = iv[1];
		a = iv[2];
		b = iv[3];
		k = iv[4];
		p = iv[5];
	}
	
 	void calc(){ 	
 		REAL x0 = nx, y0 = ny;	
		REAL temp = (p/(1+x0*x0+y0*y0));
		REAL cosx = (COS(b-temp));
		REAL sinx = (SIN(b-temp));
	/*
	xn+1 = a + k*( x0*cos(b-(p/(1+x0*x0+y0*y0))) - y0*sin(b-(p/(1*x0*x0+y0*y0))) )
	yn+1 =     k*( x0*sin(b-(p/(1+x0*x0+y0*y0))) - y0*cos(b-(p/(1*x0*x0+y0*y0))) )
	*/
		nx = a + k*(x0*cosx)-y0*sinx;
		ny = k*sinx -y0*cosx;

		ov[0] = nx;
		ov[1] = ny;
 	}
};