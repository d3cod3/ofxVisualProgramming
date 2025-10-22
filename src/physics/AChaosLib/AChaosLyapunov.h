/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/

#pragma once
#include "AChaosBase.h"
#define lorx(x, y, a) 		(a) * ((y) - (x));		
#define lory(x, y, z, a) 	((x) * ((r) - (z))) - (y);
#define lorz(x, y, z, c) 	((x) * (y)) - ((z) * (c));

class AChaosLyapunov : public AChaosBase {
public:

	REAL	a[6], b[6], nx, ny;
	
	AChaosLyapunov(){}
	~AChaosLyapunov(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 14, 2);	
		if(params==NULL){		

			a[0] = -1.4f;
			a[1] = 0.7f;
			a[2] = -0.16f;
			a[3] = -1.21f;
			a[4] = 1.11f;
			a[5] = -1.46f;
			b[0] = -0.19f;
			b[1] = 0.04f;
			b[2] =  1.59f;
			b[3] = 1.3f;
			b[4] = 1.68f;
			b[5] = 1.76f;
			nx = ny = 0.0f;
			REAL p[14] = {a[0],a[1],a[2],a[3],a[4],a[5],
				b[0],b[1],b[2],b[3],b[4],b[5],nx,ny};
			set(p);
		} else { set(params); }
	
	}

	void reset(){
		nx = iv[12];
		ny = iv[13];
		for(int i=0; i<6; i++){
			a[i] = iv[i];
		}
		for(int i=0; i<6; i++){
			b[i] = iv[i+6];
		}
	}
	
 	void calc(){ 		
		// maybe lyapunov attractor : quadratic eq
		nx = a[0] + a[1]*nx + a[2]*nx*nx + a[3]*nx*ny + a[4]*ny + a[5]*ny*ny;
		ny = b[0] + b[1]*nx + b[2]*nx*nx + b[3]*nx*ny + b[4]*ny + b[5]*ny*ny;

		ov[0] = nx;
		ov[1] = ny;
 	}
};