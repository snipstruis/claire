#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <exception>
#include <thread>
#include <mutex>
#include "connection.hpp"
#include "mandelbrot.hpp"
#include "utils.hpp"
using namespace std;

#include <cstdlib>
#include <cstdio>

SerialData toSerial(string msg){
	SerialData s(msg.size());
	memcpy(s.data(),msg.c_str(),msg.size());
	return s;
}

string toString(SerialData data){
	string s;
	for(auto d:data) s.push_back((char)d);
	return s;
}

#include "job.hpp"
void client_thread(Connection* c,Batch batch){
	static mutex iolock;
	for(Job &job: batch){
		c->send(job.serialize());

		SerialData image;

		try{image = c->recieve();}
		catch(const exception &e){cout<<e.what()<<endl;}

		stringstream ss;
		ss<<job.id<<".tga";
		string filename = ss.str();

		iolock.lock();
		ofstream file (filename, ios::out | ios::binary);
		file.write(reinterpret_cast<const char*>(image.data()), image.size());
		file.flush();
		iolock.unlock();
	}
}

void client_test(int argc, char* argv[]){
	cout<<"client"<<endl;

	// establish connections
	vector<Connection*> connections; // ptr to prevent destruction on copy
	connections.reserve((argc-1)/2);
	for(int i=1; i<argc; i+=2){
		cout<<"channel "<<i/2<<": connecting to "
			<<argv[i]<<" on port "<<argv[i+1]<<"..."<<flush;
		try{connections.push_back(new Connection( argv[i], atoi(argv[i+1]) ));}
		catch(const exception &e){cout<<"FAILED: "<<e.what()<<endl; continue;}
		cout<<"connected!"<<endl;
	}

	// create jobs
	vector<Batch> shots ={
		interpolate(Job(  0,1920,1080,-1,-1, 0, 0),
					Job(  9,1920,1080, 0, 0, 1, 1) ),
		interpolate(Job( 10,1920,1080, 0, 0, 1, 1),
					Job( 19,1920,1080, 1, 1, 1, 1) ),
		interpolate(Job( 20,1920,1080, 0, 0, 1, 1),
					Job( 29,1920,1080, 1, 1, 1, 1) )
	};
	Batch batch = flatten(shots);
	vector<Batch> batches = interleave(batch, connections.size());

	// launch threads
	vector<thread*> threads(connections.size());
	for(unsigned i=0;i<connections.size();i++){
		threads[i] = new thread(client_thread,connections[i],batches[i]);
	}

	// wait for threads to finish
	for(thread* t:threads){
		t->join();
		delete t;
	}

	// close connections
	for(auto cptr:connections) delete cptr;
}

void server_test(int port){
	cout<<"server "<<port<<": connecting..."<<flush;
	Connection connection(port);
	cout<<"connected!"<<endl;

	Batch batch;
	try{batch=deserialize(connection.recieve());}
	catch(const exception &e){cout<<e.what()<<endl;return;}
	for(Job job:batch){
		connection.send(mandelbrot(job));
	}
}

int main(int argc,char* argv[]){
	if(argc==2) server_test(atoi(argv[1]));
	else if(((argc-1)%2==0) && (argc!=1)) client_test(argc,argv);
	else cout<<"invalid arguments"<<endl;
}
