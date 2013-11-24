#include <iostream>
#include <vector>
#include <exception>
#include <thread>
#include "connection.hpp"
using namespace std;

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

void client_test(int argc, char* argv[]){
	vector<Connection*> connections; // ptr to prevent destruction on assignment
	connections.resize(argc/2);

	for(int i=1, a=0; i<argc; i+=2){
		cout<<"channel "<<a<<": connecting to "<<argv[i]<<" on port "<<argv[i+1]<<"...";
		try{connections[a++] = new Connection( argv[i], atoi(argv[i+1]) );}
		catch(const exception &e){cout<<"FAILED: "<<e.what()<<endl; continue;}
		cout<<"DONE!"<<endl;
	}

	// interactive chat client
	int channel;
	string input;
	while(cin>>channel){
		getline(cin,input);
		connections[channel]->send(toSerial(input));
		cout<<"sent "<<input.size()<<" bytes"<<endl;
	}

	// cleanup
	for(auto cptr:connections) delete cptr;
}

void server_test(int port){
	cout<<"server "<<port<<endl;
	Connection connection(port);
	cout<<"connected!"<<endl;
	while(true){
		try{cout<<toString(connection.recieve())<<endl;}
		catch(const exception &e){cout<<e.what()<<endl;return;}
	}
}

int main(int argc,char* argv[]){
	if(argc==2) server_test(atoi(argv[1]));
	else if(((argc-1)%2==0) && (argc!=1)) client_test(argc,argv);
	else cout<<"invalid arguments"<<endl;
}
