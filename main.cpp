#include <iostream>
#include <vector>
#include <exception>
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
	vector<Connection> connections;
	connections.resize(argc/2);

	for(int i=1, a=0; i<argc; i+=2){
		cout<<"connecting to "<<argv[i]<<" on port "<<argv[i+1]<<"...";
		try{connections[a++].client( argv[i], atoi(argv[i+1]) );}
		catch(const exception &e){cout<<"FAILED: "<<e.what()<<endl; continue;}
		cout<<"DONE!"<<endl;
	}

	for(auto &c:connections){
		cout<<"sending "<<c.send(toSerial("ohai"))<<" bytes"<<endl;
	}

	cout<<"press <RETURN> to close connection"<<endl;
	cin.get();
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

int main(){
	//server_test(50100);
	char* argv[] = {"client","localhost","50100","localhost","50101","localhost","50102"};
	int   argc = 7;
	client_test(argc,argv);
}
