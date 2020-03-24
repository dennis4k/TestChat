//
//  Server.hpp
//  Vsy_Projekt
//
//  Created by Dennis Halter on 18.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <set>
#include <string>
#include "ServerSocket.hpp"
#include "Socket.hpp"
#include <thread>
#include <map>
#include <vector>
#include <mutex>

using namespace std;


struct Client{
    string _usrname;
    sockaddr_in _usrinfo;
    socklen_t _socklen;
};


class Server{
private:
    int _serversocket;
    sockaddr_in _addr;
    socklen_t _len;
    map<int,Client*>*_clients;
    int _minFD ,_maxFD;
public:
    Server(int serversocket,sockaddr_in addr,socklen_t len);
    void addClient(int sock,Client *client);
    void deleteClient(int sock,sockaddr_in addr,socklen_t len);
    void broadcastToOthers(int emitterfd,map<int,Client*> *clients,string msg,std::mutex *mtx);
    void run(void);
};

#endif /* Server_hpp */
