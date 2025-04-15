/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/

#pragma once
#include "AChaosBase.h"

class AChaosBaker : public AChaosBase {
public:

	REAL eval, init;
	bool fold_cut;

	AChaosBaker(){}
	~AChaosBaker(){}	

	void setup(REAL * params = NULL){
		AChaosBase::setup(params, 2, 1);

		if(params==NULL){
			eval = 0.85f;
			init = 0.85f;
			fold_cut = false;
			REAL p[2] = {init,(REAL)fold_cut};
			set(p);
		} else {
			set(params);
		}

	}

	void reset(){		
		init = iv[0];
		fold_cut = iv[1] > 0.0f;
		eval = init;
	}
	
 	void calc(){
 		
		if 	(!fold_cut)    /* fold */
		{
			if (eval > 0.5)
				eval = 2. - (eval*2.);
			else
				eval *= 2.;
		}
		else					/* cut */
		{
			if (eval < 0.5)
				eval *= 2.;
			else
				eval = (eval*2.) - 1.;
		}

		ov[0] = eval;

 	}
};
