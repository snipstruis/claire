#pragma once

class Job{
public:
	uint32_t id;
	uint32_t pixelsWide, pixelsHigh;
	double x1,y1,x2,y2;
public:
	static const uint16_t size = 44;
	Job(){}
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

class Batch{
public:
	vector<Job> jobs;
public:
	Batch(){}
	Batch(SerialData s){
		unsigned nrOfJobs = s.size()/Job::size; // <2 jobs
		jobs.resize(nrOfJobs);
		unsigned i= 0;
		for(Job &job:jobs){
			job.id         = s[i++];
			job.pixelsWide = s[i++];
			job.pixelsHigh = s[i++];
			job.x1         = s[i++];
			job.y1         = s[i++];
			job.x2         = s[i++];
			job.y2         = s[i++];
		}
	}
};
