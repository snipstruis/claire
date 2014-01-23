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

#include "job.hpp"
void client_thread(Connection* c,Batch batch){
	static mutex iolock;

    c->send(serialize(batch));

	for(const Job &job:batch){
		SerialData image;
		try{image = c->recieve();}
		catch(const exception &e){
			cout<<e.what()<<endl;
			break;
		}

		stringstream ss;
		if(job.id < 1000000) ss<<"0";
		if(job.id < 100000)  ss<<"0";
		if(job.id < 10000)   ss<<"0";
		if(job.id < 1000)    ss<<"0";
		if(job.id < 100)     ss<<"0";
		if(job.id < 10)		 ss<<"0";
        ss<<job.id<<".tga";
		string filename = ss.str();
        cout << "creating " << filename << endl;

		iolock.lock();
		ofstream file (filename, ios::out | ios::binary);
		file.write(reinterpret_cast<const char*>(image.data()), image.size());
		file.flush();
        file.close();
		iolock.unlock();
	}
}

void client_main(int argc, char* argv[]){
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
	int shotwidth = 1920, shotheight = 1080;
	vector<Batch> shots ={
		interpolate(Job( 0,shotwidth,shotheight, -0.7439072-0.000000401834,0.1347169-0.000000401834, -0.7439072+0.000000401834,0.1347169+0.000000401834),
					Job( 29,shotwidth,shotheight, -0.7439072-0.000000401834,0.1347169-0.000000401834, -0.7439072+0.000000401834,0.1347169+0.000000401834) )
	};
	Batch masterBatch = flatten(shots);
	vector<Batch> batches = interleave(masterBatch, connections.size());


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

void server_main(int port){
	cout<<"server "<<port<<": connecting..."<<flush;
	Connection connection(port);
	cout<<"connected!"<<endl;

    Batch batch;
    try{batch=deserialize(connection.recieve());}
    catch(const exception &e){cout<<e.what()<<endl;return;}

	for(Job job:batch){
		cout<<job.id<<" "<<job.x1<<" "<<job.y1<<" "<<job.x2<<" "<<job.y2<<" "<<job.pixelsHigh<<" "<<job.pixelsWide<<endl;
		connection.send(mandelbrot(job));
	}
}

int main(int argc,char* argv[]){
	if(argc==2) server_main(atoi(argv[1]));
	else if(((argc-1)%2==0) && (argc!=1)) client_main(argc,argv);
	else cout<<"invalid arguments"<<endl;
}
