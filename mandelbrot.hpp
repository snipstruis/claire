#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
using namespace std;

#include "connection.hpp"
#include "job.hpp"
#include "utils.hpp"

double smooth(double x, double y, double i){
	double zn = sqrt( x*x + y*y );
	double nu = log( log(zn) / log(2) ) / log(2);
	double iteration = double(i) + 1.0 - nu;
	return iteration;
}

double calculateMandelbrot(double px, double py){
	double x=px;
	double y=py;
	const int maxiter = 1000;
	for(int i=0; i<maxiter; i++){
		double xtemp = x*x - y*y + px;
		y = 2.0*x*y + py;
		x = xtemp;
		if(x*x + y*y > 4.0) return smooth(x, y, double(i))/double(maxiter);
	}
	return 0.0;
}

SerialData mandelbrot(Job job){

	cout << "calculating job " << job.id << ":0%" << flush;

	SerialData ret = {
		0,0,2,0,0,0,0,0,0,0,0,0,
		uint8_t(job.pixelsWide%256),uint8_t(job.pixelsWide/256),
		uint8_t(job.pixelsHigh%256),uint8_t(job.pixelsHigh/256), 24,0
	};

	ret.reserve(3*job.pixelsHigh*job.pixelsWide+18);
	for(unsigned y=0;	y<job.pixelsHigh;	y++){
		for(unsigned x=0;	x<job.pixelsWide;	x++){

			double color = calculateMandelbrot(
						job.x1 - (x * ((job.x1-job.x2)/double(job.pixelsWide))),
						job.y1 - (y * ((job.y1-job.y2)/double(job.pixelsHigh)))
						);

			ret.push_back(uint8_t(color*256));
			ret.push_back(uint8_t(color*256));
			ret.push_back(uint8_t(color*256));
		}
		if(y%int(ceil(job.pixelsHigh/100.0))==0) {
			if(int(y/(job.pixelsHigh/100.0)) <= 10)	cout << "\b\b" << int(y/(job.pixelsHigh/100.0)) << "%" << flush;
			else cout << "\b\b\b" << int(y/(job.pixelsHigh/100.0)) << "%" << flush;
		}
	}
	cout << endl;
	return ret;
}


