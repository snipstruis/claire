#pragma once

#include <vector>
#include <cstdint>
using namespace std;

#include "connection.hpp"
#include "job.hpp"

SerialData mandelbrot(Job job){
	SerialData ret;
	ret.resize(job.pixelsHigh*job.pixelsWide+18);
	ret.assign({
		0,0,2,0,0,0,0,0,0,0,0,0,
		uint8_t(job.pixelsWide%256),uint8_t(job.pixelsWide/256),
		uint8_t(job.pixelsHigh%256),uint8_t(job.pixelsHigh/256), 24,0
	});
	unsigned i=18;
	for(unsigned x=0; x<job.pixelsHigh; x++)
	for(unsigned y=0; y<job.pixelsWide; y++){
		ret[i++]=x%256;
		ret[i++]=y%256;
		ret[i++]=0;
	}
	return ret;
}
