
//
//  socket.hpp
//  ParallelerServerC++
//
//  Created by Dennis Halter on 09.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#ifndef socket_hpp
#define socket_hpp

#include <iostream>
#include <cerrno>
#include <string>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

class Socket{
private:
    sockaddr_in _address;
    int _socket;
public:
    Socket(std::string ip , int port);
    Socket(int Socket);
    
    void send (std::string msg);
    std::string recv(void);
    void close(void);
    int getSocket();
};



#endif /* socket_hpp */
