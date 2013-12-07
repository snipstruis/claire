#pragma once
#include <vector>
#include <array>
using namespace std;

template<class T>
vector<vector<T> > interleave(vector<T> v, unsigned l){
	vector<vector<T> > r(l);
	int i=0;
	for(T &t:v)	r[(i++)%l].push_back(t);
	return r;
}

template<class T>
vector<T> flatten(vector< vector<T> > w){
	vector<T> r;
	for(const vector<T> &v:w)
		r.insert(r.end(),v.begin(),v.end());
	return r;
}

array<uint8_t,4> serialize(uint32_t x){
	array<uint8_t,4> r;
	r[0] = static_cast<uint8_t>(x);
	r[1] = static_cast<uint8_t>(x>> 8);
	r[2] = static_cast<uint8_t>(x>>16);
	r[3] = static_cast<uint8_t>(x>>24);
	return r;
}

unsigned deserialize(array<uint8_t,4> x){
	return uint32_t(x[0])
		 | uint32_t(x[1])<<8
		 | uint32_t(x[2])<<16
		 | uint32_t(x[3])<<24;
}
