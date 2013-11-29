#include <sstream>
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
		SerialData image = c->recieve();

		stringstream ss;
		ss<<job.id<<".tga";
		string filename = ss.str();

		iolock.lock();
		 FILE* file=fopen(filename.c_str(),"w");
		  fwrite(image.data(),1,image.size(),file);
		 fclose(file);
		iolock.unlock();
	}
}

void client_test(int argc, char* argv[]){
	cout<<"client"<<endl;

	vector<Connection*> connections; // ptr to prevent destruction on copy
	connections.resize((argc-1)/2);

	for(int i=1, a=0; i<argc; i+=2){
		cout<<"channel "<<a<<": connecting to "<<argv[i]<<" on port "<<argv[i+1]<<"...";
		try{connections[a++] = new Connection( argv[i], atoi(argv[i+1]) );}
		catch(const exception &e){cout<<"FAILED: "<<e.what()<<endl; continue;}
		cout<<"DONE!"<<endl;
	}

	vector<Batch> _temp ={
		interpolate(Job(  0,1920,1080,-1,-1, 0, 0),
					Job( 59,1920,1080, 0, 0, 1, 1) ),
		interpolate(Job( 60,1920,1080, 0, 0, 1, 1),
					Job(119,1920,1080, 1, 1, 1, 1) ),
		interpolate(Job(120,1920,1080, 0, 0, 1, 1),
					Job(179,1920,1080, 1, 1, 1, 1) )
	};

	Batch batch = flatten(_temp);

	vector<Batch> batches = interleave(batch, connections.size());

	vector<thread*> t(connections.size());
	for(unsigned i=0;i<connections.size();i++){
		t[i] = new thread(client_thread,connections[i],batches[i]);
	}

	// cleanup
	for(auto cptr:connections) delete cptr;
}

void server_test(int port){
	cout<<"server "<<port<<": connecting...";
	Connection connection(port);
	cout<<"connected!"<<endl;
	while(true){
		Batch batch;
		try{batch=deserialize(connection.recieve());}
		catch(const exception &e){cout<<e.what()<<endl;return;}
		for(Job job:batch){
			connection.send(mandelbrot(job));
		}
	}
}

int main(int argc,char* argv[]){
	if(argc==2) server_test(atoi(argv[1]));
	else if(((argc-1)%2==0) && (argc!=1)) client_test(argc,argv);
	else cout<<"invalid arguments"<<endl;
}
