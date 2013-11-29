#pragma once
#include <vector>
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
