/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/

#pragma once
#include "AChaosBase.h"

class AChaosJong : public AChaosBase {
public:

	REAL a, b, c, d, nx, ny;	

	AChaosJong(){}
	~AChaosJong(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 6, 2);	
		if(params==NULL){	
			//init	
			a = 1.4f;
			b = -2.3f;
			c = 2.4f;
			d = -2.1f;
			nx = 0.0f;
			ny = 0.0f;
			REAL p[6] = {a,b,c,d,nx,ny};
			set(p);
		}  else { set(params); }
	
	}

	void reset(){
		a = iv[0];
		b = iv[1];
		c = iv[2];
		d = iv[3];
		nx = iv[4];
		ny = iv[5];
	}
	
 	void calc(){ 	
	// Peter de Jong attractor
	//	lx1 = sin(a*ny)-cos(b*nx);
	//	ly1 = sin(c*nx)-cos(d*ny);

		nx = SIN(a*ny)-COS(b*nx);
		ny = SIN(c*nx)-COS(d*ny);

		ov[0] = nx;
		ov[1] = ny;

 	}


	
};
