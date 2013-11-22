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
using PixelData  = vector<uint8_t>;

class Image{
	uint32_t id;
	vector<uint8_t> data;
public:
	Image(unsigned width, unsigned height, PixelData pixeldata);
	Image(SerialData s);
	SerialData serialize() const;
};

#include <exception>
class BadSocket:public exception{
public:
	virtual const char* what() const throw(){
		return "unable to open socket";
	}
};
class HostNotFound:public exception{
public:
	const string host;
	HostNotFound(string _host):host(_host){}
	virtual const char* what() const throw(){
		return string("unable to find host: ").append(host).c_str();
	}
};
class ConnectionFailed:public exception{
public:
	virtual const char* what() const throw(){
		return "connection failed";
	}
};
class ConnectionRefused:public exception{
public:
	virtual const char* what() const throw(){
		return "connection refused";
	}
};
class BindingFailed:public exception{
public:
	virtual const char* what() const throw(){
		return "binding failed";
	}
};

class Connection{
	int sockfd;
public:
	// client initialisation
	Connection(string hostname,int port){
		// create socket
		sockfd = socket(AF_INET, SOCK_STREAM,0);
		if(sockfd<0) throw(BadSocket());

		// find host
		hostent* server = gethostbyname(hostname.c_str());
		if(server==NULL) throw(HostNotFound(hostname));

		// connect
		sockaddr_in serv_addr;
		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
		serv_addr.sin_port = htons(port);

		int status = connect(sockfd,(sockaddr *) &serv_addr,sizeof(serv_addr));
		if(status<0) throw(ConnectionFailed());
	}
	// server initialisation
	Connection(int port){
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
	~Connection(){cout<<"closing connection"<<endl;close(sockfd);}
	void send(SerialData serialdata){
		uint16_t size;
		size = serialdata.size();
		cout<<"(sendsize: "<<size<<")\n";
		serialdata.insert(serialdata.begin(),size/256);
		serialdata.insert(serialdata.begin(),size%256);
		write(sockfd,serialdata.data(),serialdata.size());
	}
	SerialData recieve(){
		uint16_t size;
		read(sockfd, &size, 2);
		SerialData r(size);
		if(size==0) return r;
		read(sockfd, r.data(), size);
		cout<<"(recvsize: "<<size<<")\n";
		return r;
	}
};

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
