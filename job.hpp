#pragma once
#include <vector>
#include <cmath>
#include <cassert>
#include "connection.hpp"
#include "utils.hpp"
using namespace std;

class Job{
public:
	uint32_t id;
	uint32_t pixelsWide, pixelsHigh;
	double x1,y1,x2,y2;
public:
	static const uint16_t size = 44;
	Job(){}
	Job(SerialData s){
		uint16_t i=0;
		memcpy(&id,        &s[i],4); i+=4;
		memcpy(&pixelsWide,&s[i],4); i+=4;
		memcpy(&pixelsHigh,&s[i],4); i+=4;
		memcpy(&x1,        &s[i],8); i+=8;
		memcpy(&y1,        &s[i],8); i+=8;
		memcpy(&x2,        &s[i],8); i+=8;
		memcpy(&y2,        &s[i],8);
	}
	Job(uint32_t _id, uint32_t _pixelsWide, uint32_t _pixelsHigh,
		double _x1, double _y1, double _x2, double _y2):
		id(_id),pixelsWide(_pixelsWide),pixelsHigh(_pixelsHigh),
		x1(_x1),y1(_y1),x2(_x2),y2(_y2){
		if((x1>x2)||(y1>y2))
			cout<<"Job constructor warning: (x1,y1)>(x2,y2)"<<endl;
	}
	SerialData serialize() const{
		SerialData r(size);
		uint16_t i=0;
		memcpy(&r[i],&id,        4); i+=4;
		memcpy(&r[i],&pixelsWide,4); i+=4;
		memcpy(&r[i],&pixelsHigh,4); i+=4;
		memcpy(&r[i],&x1,        8); i+=8;
		memcpy(&r[i],&y1,        8); i+=8;
		memcpy(&r[i],&x2,        8); i+=8;
		memcpy(&r[i],&y2,        8);
		return r;
	}
};

using Batch = vector<Job>;

Batch deserialize(const SerialData s){
	Batch r;
	unsigned nrOfJobs = s.size()/Job::size;
	r.resize(nrOfJobs);
	auto from = s.begin();
	auto to   = s.begin()+Job::size;
	for(Job &job: r){
		job  =Job(SerialData(from,to));
		from+=Job::size;
		to  +=Job::size;
	}
	return r;
}

SerialData serialize(const Batch v) {
	SerialData r;
	r.reserve(v.size()*Job::size);
	for(Job job:v){
		SerialData s = job.serialize();
		r.insert( r.end(), s.begin(), s.end() );
	}
	return r;
}

Batch interpolate(const Job a, const Job b){
	assert(a.id<b.id);
	assert(a.pixelsHigh==b.pixelsHigh);
	assert(a.pixelsWide==b.pixelsWide);

	Batch batch(b.id-a.id);
	batch.front() = a;
	int i=1;
	for(auto it=batch.begin()+1; it<batch.end(); it++, i++){
		it->id = a.id+i;
		it->x1 = i*(fabs(a.x1-b.x1)/double(b.id-a.id))+a.x1;
		it->y1 = i*(fabs(a.y1-b.y1)/double(b.id-a.id))+a.y1;
		it->x2 = i*(fabs(a.x2-b.x2)/double(b.id-a.id))+a.x2;
		it->y2 = i*(fabs(a.y2-b.y2)/double(b.id-a.id))+a.y2;
		it->pixelsHigh = a.pixelsHigh;
		it->pixelsWide = a.pixelsWide;
	}

	for(Job job:batch){
		cout<<job.id<<" "<<job.x1<<" "<<job.y1<<" "<<job.x2<<" "<<job.y2<<endl;
	}
	return batch;
}
