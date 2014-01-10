#pragma once

#include <vector>
#include <cstdint>
using namespace std;

#include "connection.hpp"
#include "job.hpp"
#include "utils.hpp"

SerialData mandelbrot(Job job){

    cout << "calculating job " << job.id << ":\n width:" << job.pixelsWide << "\n height:" << job.pixelsHigh << endl;

	SerialData ret = {
		0,0,2,0,0,0,0,0,0,0,0,0,
		uint8_t(job.pixelsWide%256),uint8_t(job.pixelsWide/256),
		uint8_t(job.pixelsHigh%256),uint8_t(job.pixelsHigh/256), 24,0
	};
	ret.reserve(3*job.pixelsHigh*job.pixelsWide+18);
	for(unsigned x=0; x<job.pixelsHigh; x++)
	for(unsigned y=0; y<job.pixelsWide; y++){
		ret.push_back(x%256);
		ret.push_back(y%256);
		ret.push_back(0xBB);
	}

	return ret;
}
