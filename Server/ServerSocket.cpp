//
//  server.cpp
//  Socket-Wrapper
//
//  Created by Dennis Halter on 12.10.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#include "ServerSocket.hpp"


ServerSocket::ServerSocket(int port , int queue_size){
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1)
        throw SocketException("cant create Socket");
    
    
    int i=0;
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    
    //bind server port
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(port);
    

    int r = bind(_socket,(struct sockaddr *)&_address,sizeof(_address));
    
    if( r == -1)
        throw SocketException("can't bind socket!");
    
    
    //listen for incoming requests
    
    listen(_socket, queue_size);
    _addrlen = sizeof(struct sockaddr_in);
}

ServerSocket::~ServerSocket(){
    ::close(_socket);
}

int ServerSocket::accept(){
    //waiting for incoming request
    
    std::cout << "waiting for incoming request..." << std::endl;
    int conn = ::accept(_socket,(struct sockaddr *) &_address,&_addrlen);
    if(conn == -1)
        throw SocketException("can't accept Connection!");
    
    std::cout << "accept Socket: " << conn << std::endl;
    return conn;
}


int ServerSocket::getSockFD(){
    return _socket;
}

sockaddr_in ServerSocket::getSockAddr(){
    return _address;
}

socklen_t ServerSocket::getAddrLen(){
    return _addrlen;
}





