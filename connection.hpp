#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <vector>
using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using SerialData = vector<uint8_t>;

#include <exception>
class BadSocket:public exception{
public:	virtual const char* what() const throw(){
		return "unable to open socket";
	}
};
class HostNotFound:public exception{
public:	const string host;
	HostNotFound(string _host):host(_host){}
	virtual const char* what() const throw(){
		return string("unable to find host: ").append(host).c_str();
	}
};
class ConnectionFailed:public exception{
public:	virtual const char* what() const throw(){
		return "connection failed";
	}
};
class ConnectionRefused:public exception{
public:	virtual const char* what() const throw(){
		return "connection refused";
	}
};
class BindingFailed:public exception{
public: virtual const char* what() const throw(){
		return "failed to bind socket to port";
	}
};
class ConnectionClosed:public exception{
public: virtual const char* what() const throw(){
		return "Connection closed by other party";
	}
};

class Connection{
	int sockfd;
public:
	// deffered initialisation (not recommended)
	Connection(){}
	void init(int port){
		// create socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd<0) throw(BadSocket());

		// bind
		sockaddr_in serv_addr;
		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);

		int status = bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr));
		if(status<0) throw(BindingFailed());

		// listen
		listen(sockfd,1);

		// accept
		sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		sockfd = accept(sockfd, (sockaddr *) &cli_addr, &clilen);
		if (sockfd < 0){cerr<<"accept error"<<endl; exit(6);}
	}

	void init(string host,int port){
		// create socket
		sockfd = socket(AF_INET, SOCK_STREAM,0);
		if(sockfd<0) throw(BadSocket());

		// find host
		hostent* server = gethostbyname(host.c_str());
		if(server==NULL) throw(HostNotFound(host));

		// connect
		sockaddr_in serv_addr;
		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
		serv_addr.sin_port = htons(port);

		int status = connect(sockfd,(sockaddr *) &serv_addr,sizeof(serv_addr));
		if(status<0) throw(ConnectionFailed());
	}

	// client initialisation (preferred)
	Connection(string host,int port){init(host,port);}

	// server initialisation (preferred)
	Connection(int port){init(port);}




	~Connection(){cout<<"closing connection"<<endl;close(sockfd);}
	uint16_t send(SerialData serialdata){
		uint16_t size;
		size = serialdata.size();
		serialdata.insert(serialdata.begin(),size/256);
		serialdata.insert(serialdata.begin(),size%256);
		write(sockfd,serialdata.data(),serialdata.size());
		return size;
	}
	SerialData recieve(){
		uint16_t size;
		read(sockfd, &size, 2);
		SerialData r(size);
		if(size==0) throw(ConnectionClosed());
		read(sockfd, r.data(), size);
		return r;
	}
};
