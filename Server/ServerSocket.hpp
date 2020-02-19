//
//  server.hpp
//  Socket-Wrapper
//
//  Created by Dennis Halter on 12.10.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#ifndef server_hpp
#define server_hpp

#include <iostream>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include "SocketException.hpp"


class ServerSocket{
private:
    sockaddr_in _address;
    int _socket;
    socklen_t _addrlen;
public:
    ServerSocket(int port, int queue_size);
    ~ServerSocket();
    
    int accept();
    void send(std::string msg);
    std::string recv();
    int getSockFD();
    sockaddr_in getSockAddr();
    socklen_t getAddrLen();
};

#endif /* server_hpp */
