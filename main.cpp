#include <iostream>
#include <vector>
#include <exception>
#include "connection.hpp"
using namespace std;

void client_test(string hostname, int port){
	cout<<"client"<<endl;
	Connection connection(hostname,port);

	string msg = "server, this is client. over.";
	SerialData s(msg.size());
	memcpy(s.data(),msg.c_str(),msg.size());
	connection.send(s);
	cout<<"sending:  "<<msg<<endl;

	SerialData  response = connection.recieve();

	cout<<"recieved: ";
	for(auto c:response) cout<<char(c);
	cout<<endl;
}

void server_test(int port){
	cout<<"server"<<endl;

	Connection connection(port);

	while(true){
		SerialData i = connection.recieve();
		if(i.size()==0) return;

		cout<<"recieved: ";
		for(auto c:i) cout<<char(c); cout<<endl;

		string msg = "roger client, reading loud and clear.";
		SerialData s(msg.size());
		memcpy(s.data(),msg.c_str(),msg.size());
		connection.send(s);
		cout<<"sending:  "<<msg<<endl;

		for(unsigned i=0;i<0xFFFFFF; i++){
			cout<<" \r";
		}
	}
}

int main(){
	//server_test(3144);
	client_test("localhost",3144);
}
