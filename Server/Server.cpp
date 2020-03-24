//
//  Server.cpp
//  Vsy_Projekt
//
//  Created by Dennis Halter on 18.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#include "Server.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tokenizer.hpp"


Server::Server(int serversocket,sockaddr_in addr,socklen_t len){
    _serversocket = serversocket;
     _clients = new map<int,Client*>;
     _addr= addr;
     _len = len;
    Client *serverinfo = new Client;
    serverinfo->_usrinfo = _addr;
    serverinfo->_socklen = _len;
    serverinfo->_usrname = "server";
    addClient(_serversocket,serverinfo);
    _minFD = _maxFD = _serversocket;
    }

void Server::addClient(int sock,Client *client){
    if(sock != _serversocket)
        cout << "[Server " << ntohs(_addr.sin_port) << "]" << "[Client connected:] " << inet_ntoa(client->_usrinfo.sin_addr) << " | " << client->_usrinfo.sin_port << " | " << sock << endl;
    _clients->insert(std::pair<int,Client*>(sock,client));
    
}


void Server::deleteClient(int sock,sockaddr_in addr,socklen_t len){
     cout <<"[Server " << ntohs(_addr.sin_port) << "]" << "[Client disconnected:] " << inet_ntoa(addr.sin_addr) << " | " << addr.sin_port<< " | " << sock << endl;
    _clients->erase(sock);
}



void Server::broadcastToOthers(int emitterfd, map<int,Client*>*clients,string msg,std::mutex *mtx){
    for(map<int,Client*>::iterator iter = _clients->begin();iter != _clients->end();iter++){
        if(iter->first != emitterfd && iter->first != _serversocket){
            Socket sock(iter->first);
            try{
                sock.send(msg); // Send Userlist to new Client
            }catch(SocketException &error){
                error.show();
                string msg = "DEL_Client:" + iter->second->_usrname;
                broadcastToOthers(iter->first,clients,msg,mtx);
                while(1){
                    if(mtx->try_lock()){
                        deleteClient(iter->first, iter->second->_usrinfo, iter->second->_socklen);
                        mtx->unlock();
                        break;
                    }
                    else
                        continue;
                }
                break;
                }
            
            }
        }
    }



void handleConnection(int acptSocket,map<int,Client*>*_clients,int _serversocket,Client *client,Server *s,std::mutex *mtx){
    std::thread::id this_id = std::this_thread::get_id();
    
    cout << "[Client Thread ID: " <<  this_id << "]" << endl;

    
    bool online=1;
    Socket sock(acptSocket);
    
    //Sending Client_List. Important for client to know which username is already in use..
    string ClList; // Client List
    for(map<int,Client*>::iterator sendclientfd = _clients->begin(); sendclientfd != _clients->end();sendclientfd++){
        if (sendclientfd->first != _serversocket && sendclientfd->first != acptSocket ){
            if(sendclientfd->second->_usrname != "")
                ClList += (sendclientfd->second->_usrname + ":");
        }
    }
    
    //sending List connected Clients
    if(ClList.size() != 0){
        string fullmsg = "CL_List:" + ClList; //MSG_TYP:CL_LIST
        try{
        sock.send(fullmsg);
        }catch(SocketException &error){
            error.show();
            for(map<int,Client*>::iterator iter = _clients->begin();iter != _clients->end();iter++){
                if(iter->first == acptSocket){
                    string msg = "DEL_Client:" + iter->second->_usrname;
                    s->broadcastToOthers(iter->first,_clients,msg,mtx);
                    while(1){
                        if(mtx->try_lock()){
                            s->deleteClient(acptSocket, iter->second->_usrinfo, iter->second->_socklen);
                            mtx->unlock();
                            break;
                        }
                        else
                            continue;
                    }
                    online=0;
                    break;
                }
            }
            
        }
    }
    //sending empthy List
    else{
        
        string fullmsg = "CL_List:ZERO";
        try{
            sock.send(fullmsg);
        }catch(SocketException &error){
            error.show();
            for(map<int,Client*>::iterator iter = _clients->begin();iter != _clients->end();iter++){
                if(iter->first == acptSocket){
                    string msg = "DEL_Client:" + iter->second->_usrname;
                    s->broadcastToOthers(acptSocket, _clients, msg,mtx);
                    while(1){
                        if(mtx->try_lock()){
                            s->deleteClient(acptSocket, iter->second->_usrinfo, iter->second->_socklen);
                            break;
                        }
                        else
                            continue;
                    }
                    online=0;
                    break;
                }
            }
            
        }
    }
    
    
    if(online == true){
        while(1){
            if(mtx->try_lock()){
                //ADD Client to Connecected List
                s->addClient(acptSocket,client);
                mtx->unlock();
                break;
            }
            else
                continue;
            }
        string name;
        //receive Name
        try{
            name = sock.recv();
        }catch(SocketException &error){
            cout << error.show() << endl;
            for(map<int,Client*>::iterator iter = _clients->begin();iter != _clients->end();iter++){
                if(iter->first == acptSocket){
                    string msg = "DEL_Client:" + iter->second->_usrname;
                    s->broadcastToOthers(acptSocket, _clients, msg,mtx);
                    while(1){
                        if(mtx->try_lock()){
                            s->deleteClient(acptSocket, iter->second->_usrinfo, iter->second->_socklen);
                            mtx->unlock();
                            break;
                        }
                        else
                            continue;
                    }
                    online=0;
                    break;
                }
            }
            
        }
        
        
        //find and add name to client
        map<int,Client*>::iterator iter;
        for(iter = _clients->begin();iter != _clients->end();iter++){
            if(iter->first == acptSocket && name != ""){
                while(1){
                    //trying to lock
                    if(mtx->try_lock()){
                        iter->second->_usrname = name;
                        mtx->unlock();
                        break;
                    }
                    else
                        continue;
                }
                string msg = "NW_Client:" + name;
                s->broadcastToOthers(acptSocket, _clients, msg,mtx);
                break;
            }
        }
        
        if(iter == _clients->end()){
            online=0;
        }
    
    }
    
//--------------------------------------------------------------------------------
    //LOOP
    while(online == true){
        //MSG
        string who_what_where;
        
        try{
            who_what_where =sock.recv();
            cout << "[FULL MESSAGE REVEICE]: " << who_what_where << endl;
        }catch(SocketException &error){
            cout << error.show() << endl;
            for(map<int,Client*>::iterator found_socket = _clients->begin();found_socket != _clients->end();found_socket++){
                if(found_socket->first == acptSocket){
                    string msg = "DEL_Client:" + found_socket->second->_usrname;
                    s->broadcastToOthers(found_socket->first, _clients, msg,mtx);
                    while(1){
                        if(mtx->try_lock()){
                            s->deleteClient(found_socket->first, found_socket->second->_usrinfo, found_socket->second->_socklen);
                            mtx->unlock();
                            break;
                        }
                    }
                    online=0;
                    break;
                }
            }
            break;
        }
        
        
        
        Tokenizer tok(who_what_where,":"); //Client Anfrage würde so aussehen Username:Message:Destination
        string who= tok.nextToken(); // who
        string what = tok.nextToken(); // what
        string where = tok.nextToken();// where
        
        
        if(where == "ALL"){
            //Broadcast
            //cout << "[Destination Type:]"<< "<<Broadcast>>" << endl;
            string broadcastmsg = "USR_BROADCAST:" + who + ":" + what; // MSG_TYP:NAME:CONTENT
            s->broadcastToOthers(acptSocket, _clients, broadcastmsg,mtx);
        }
        else{
            //search and send to private client
            //cout << "[Destination Type:]"<< "<<Unicast>>" << endl;
            map<int,Client*>::iterator search;
            for(search=_clients->begin(); search != _clients->end();search++){
                if(search->second->_usrname == where){
                    Socket sendsock(search->first);
                    string privatmsg = "USR_UNICAST:" + who + ":" + what; // MSG_TYP:NAME:CONTENT
                    try{
                        sendsock.send(privatmsg);
                    }catch(SocketException &error){
                        cout << error.show() << endl;
                        string msg = "DEL_Client:" + search->second->_usrname; // user ist nicht erreichbar, benachrichtige die Anderen
                        s->broadcastToOthers(search->first, _clients, msg,mtx);
                        while(1){
                            if(mtx->try_lock()){
                            s->deleteClient(search->first, search->second->_usrinfo, search->second->_socklen);
                            mtx->unlock();
                            break;
                            }
                            else
                                continue;
                        }
                        break;
                    }
                    break;
                }
            }
            if(search == _clients->end()){
                cout << "[Destination Client not found!]" << endl;
                Socket sendsock(acptSocket); // informiere den Client , der die Nachricht gesendet hat dass dieser User(where) nicht existiert
                string errmsg="ERR_NO_USR:" + where + "No User found."; // MSG_TYP:NAME:CONTENT
                try{
                    sendsock.send(errmsg);
                }catch(SocketException &error){ //Falls der Client in der Zwischenzeit abschmiert xD
                    cout << error.show() << endl;
                    string msg = "DEL_Client:" + who; //der sendende Clientname
                    s->broadcastToOthers(acptSocket, _clients, msg,mtx);
                    map<int,Client*>::iterator findSock; // suche sockets _userinfo und _socklen
                    for(findSock =_clients->begin(); findSock != _clients->end();findSock++){
                        if(findSock->first == acptSocket){
                            while(1){
                                if(mtx->try_lock()){
                                    s->deleteClient(findSock->first, findSock->second->_usrinfo, findSock->second->_socklen);
                                    mtx->unlock();
                                    break;
                                }
                                else
                                    continue;
                            }
                            online=0;
                            break;
                        }
                    }
                }
            }
        }
        
    } // while
    
    cout << "[Client Thread ID finished: " <<  this_id << "]" << endl;
} //Function





void Server::run(){
    cout << "[Server running on Port: "  << ntohs(_addr.sin_port) << "]"<< endl;
    cout << "[waiting for incoming Request...]" << endl;
    
    std::mutex *mtx =  new std::mutex;
    while(1){
        sockaddr_in acpt;
        socklen_t acptlen;
        int acptSocket = ::accept(_serversocket,(struct sockaddr*)&acpt,&acptlen);
        
        Client *client = new Client;
        client->_usrinfo = acpt;
        client->_socklen = acptlen;
        
        thread new_client =thread(handleConnection,acptSocket,_clients,_serversocket,client,this,mtx);
    
        new_client.detach();
         cout << "[Clients online: " << _clients->size() << "]" << endl;
        
    } // while
} // run
