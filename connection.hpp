#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <cmath>
#include "utils.hpp"
#include "job.hpp"
using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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
    static const size_t packetsize;
public:
	Connection(){}
	//CLIENT
	Connection(string host,int port){
		// create socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
	//SERVER
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
		if (sockfd < 0) throw(ConnectionRefused());
	}

	~Connection(){cout<<"closing connection"<<endl;close(sockfd);}

    void send(const SerialData& _data){
		cout<<"[send] "<<flush;

		// send size
        array<uint8_t,4> temp = serialize(_data.size());
		write(sockfd, &temp[0], 4);

        size_t packets   = ceil(_data.size()/double(packetsize));
        cout<<"sending "<<_data.size()<<" bytes in "<<packets<<" packets"<<endl;
		for(size_t i=0;i<packets;i++){
			// send payload
            size_t currentpacketsize=packetsize;
            if((_data.size()-(packetsize*i)) < packetsize) currentpacketsize = (_data.size()-(packetsize*i));
            int bytesSent = write(sockfd, &_data[(i*packetsize)], currentpacketsize); //FIXME

			cout<<"[send] ["<<i+1<<"/"<<packets<<"] sent "<<bytesSent<<" bytes"<<flush;

            // receive acknowledgement
			array<uint8_t,4> rcvbuf = {{0}};
			read(sockfd,&rcvbuf[0],4);

            if(unsigned(bytesSent)==deserialize(rcvbuf)){
				cout<<" [OK]"<<endl;
			}else{
				cout<<" [NACK]"<<endl;
			}
		}
		cout<<endl;
	}
	SerialData recieve(){
		//receiving size
		cout<<"[recv] "<<flush;
		array<uint8_t,4> sizev = {{0}};
		read(sockfd, &sizev[0], 4);
        uint32_t totalsize = deserialize(sizev);
		if(totalsize==0) throw(ConnectionClosed());

		size_t packets = ceil(totalsize/double(packetsize));
		cout<<"expecting "<<totalsize<<" bytes in "<<packets<<" packets"<<endl;

		SerialData totalbuf;
		totalbuf.reserve(totalsize);
        for(size_t i=1;i<=packets;i++){
			// receive payload

            size_t currentpacketsize=packetsize;
            if((totalsize-(packetsize*(i-1))) < packetsize) currentpacketsize = (totalsize-(packetsize*(i-1)));

            SerialData rcvbuf(packetsize<totalsize?packetsize:totalsize);
            int currentbytesRead = read(sockfd, &rcvbuf[0], rcvbuf.size());
            int bytesRead = currentbytesRead;
            totalbuf.insert(totalbuf.end(),rcvbuf.begin(),rcvbuf.begin()+bytesRead);
            cout<<"[recv] ["<<i<<"/"<<packets<<"] read "<<currentbytesRead<<" bytes out of "<<currentpacketsize<<endl;

            // try to complete if packet is incomplete
            while(bytesRead<packetsize && !(i==packets && bytesRead==currentpacketsize)){
                cout<<"[recv] incomplete packet trying to complete." << endl;
                currentbytesRead = read(sockfd, &rcvbuf[0], rcvbuf.size());
                bytesRead += currentbytesRead;
                totalbuf.insert(totalbuf.end(),rcvbuf.begin(),rcvbuf.begin()+currentbytesRead);
                cout<<"[recv] ["<<i<<"/"<<packets<<"] read "<<currentbytesRead<<" bytes out of "<<currentpacketsize<<endl;
            }

			// send acknowledgement
			array<uint8_t,4> sndbuf = serialize(bytesRead);
			write(sockfd, &sndbuf[0], 4);
		}
		cout<<endl;
		return totalbuf;
	}
};

const size_t Connection::packetsize=100000;
