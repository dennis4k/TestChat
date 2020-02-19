//
//  socket.cpp
//  ParallelerServerC++
//
//  Created by Dennis Halter on 09.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#include "Socket.hpp"
#include "SocketException.hpp"
#include <arpa/inet.h>

Socket::Socket(std::string ip,int port){
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1)
        throw SocketException("cant create Socket");
    
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = inet_addr(ip.c_str());
    _address.sin_port=htons(port);
    int len = sizeof(_address);
    int rc = ::connect(_socket, (struct sockaddr*)&_address, len);
    if(rc == -1)
        throw SocketException("cant connect to ip:port");
    
}

Socket::Socket(int Socket){
    _socket = Socket;
    if(Socket == -1)
        throw SocketException("Cant create Socket!");
}

void Socket::send(std::string msg){
    size_t t;
    msg +="]]]"; //append end of message
    t = write(_socket, msg.c_str(), msg.length());
    if(t == -1)
        throw SocketException("[send error: client disconnected]");
    else
        std::cout << "[MESSAGE SENT:] " << msg << std::endl;
}

void Socket::close(){
    ::close(_socket);
}

std::string Socket::recv(){
    char block[256];
    std::string res;
    size_t len;
    
    do{ // while not end of message
        block[0] = '\0';
        len = read(_socket, block, 256);
        if(len != 0)
            res+=block;
        else{
            throw SocketException("[recv error: client disconnected]");
        }
    }while(res.find("]]]") == std::string::npos);
    
    
    //extract end of message
    std::string::size_type pos = res.find("]]]");
    res = res.substr(0,pos);
    
    return res;
    
}


int Socket::getSocket(){
    return _socket;
}