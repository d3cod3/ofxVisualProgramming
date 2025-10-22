/*
	32/64 bits A-Chaos Lib in openFrameworks	
	(c) s373.net/x 2004, 2012, 2015
	http://s373.net/code/A-Chaos-Lib/A-Chaos.html
	programmed by Andre Sier, revised 2015
	License: MIT
*/
#pragma once
#include "AChaosBase.h"

#define rosx(y,z)		-((y)+(z))
#define rosy(x,y,a)		(x)+((a)*(y))
#define rosz(x,z,b,c)	(b)+((x)*(z))-((c)*(z))

class AChaosRossler : public AChaosBase {
public:

	REAL a, b, c, nx, ny, nz,dt;
	
	AChaosRossler(){}
	~AChaosRossler(){}	

	void setup(REAL * params = NULL){
	
		AChaosBase::setup(params, 7, 3);	
		if(params==NULL){		
			//classic rossler
			a = 0.2;
			b = 0.2;
			c = 5.7;
			nx = 1;
			ny = 1;
			nz = 1;
			dt = 0.01;
			REAL p[7] = {a,b,c,nx,ny,nz,dt};
			set(p);
		} else { set(params); }

	}

	void reset(){
		a = iv[0];
		b = iv[1];
		c = iv[2];
		nx = iv[3];
		ny = iv[4];
		nz = iv[5];
		dt = iv[6];
	}
	

	void calc(){ 	
		REAL fx, fy, fz, ffx, ffy, ffz, dt2 = dt/2.;
		REAL dtfx, dtfy, dtfz;
			
		fx = rosx(ny,nz);
		fy = rosy(nx,ny,a);
		fz = rosz(nx,nz,b,c);

		ffx = rosx((ny+dt*fy), (nz+(dt*fz)));
		ffy = rosy((nx+dt*nx), (ny+dt*ny), a);
		ffz = rosz((nx+dt*fx), (nz+dt*nz), b, c);

		nx += (dt2*(fx+ffx));
		ny += (dt2*(fy+ffy));
		nz += (dt2*(fz+ffz));

		ov[0] = nx;
		ov[1] = ny;
		ov[2] = nz;
 	}
};