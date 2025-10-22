/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"
#define	pi  3.14159265358979323846264338327950288419716939937510f
#define twopi 2.*pi
class AChaosTorus : public AChaosBase {
public:

	REAL x0, y0, cr;
	
	AChaosTorus(){}
	~AChaosTorus(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 3, 2);	
		if(params==NULL){		
			//init	
			x0 = 0.777f;
			y0 = 1.2f;
			cr = 1.26f;

			REAL p[3] = {x0,y0,cr};
			set(p);
		} else { set(params); }

	}

	void reset(){
		x0 = iv[0];
		y0 = iv[1];
		cr = iv[2];
	}
	
 	void calc(){ 	
 		x0 = fmodf((x0+cr*SIN(y0)), twopi);
		y0 = fmodf((x0+y0), twopi);

		ov[0] = x0;
		ov[1] = y0;
 	}
};