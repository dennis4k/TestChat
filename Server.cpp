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
    _threads = new map<int,thread*>;
    pushmap = nullptr;
     _addr= addr;
     _len = len;
    FD_ZERO(&_fdSet);
    FD_SET(_serversocket,&_fdSet);
    Client *serverinfo = new Client;
    serverinfo->_usrinfo = _addr;
    serverinfo->_socklen = _len;
    serverinfo->_usrname = "server";
    addClient(_serversocket,serverinfo);
    _minFD = _maxFD = _serversocket;
    }

void Server::addClient(int sock,Client *client){
    if(sock < _minFD)
        _minFD = sock;
    if(sock > _maxFD)
        _maxFD = sock;
    
    if(sock != _serversocket)
        cout << "[Server " << ntohs(_addr.sin_port) << "]" << "[Client connected:] " << inet_ntoa(client->_usrinfo.sin_addr) << " | " << client->_usrinfo.sin_port << " | " << sock << endl;
    _clients->insert(std::pair<int,Client*>(sock,client));
    if(sock != _serversocket)
        FD_SET(sock,&_fdSet);
}


void Server::deleteClient(int sock,sockaddr_in addr,socklen_t len){
    
    if(sock == _minFD){
        _minFD += 1;
    }
    if(sock == _maxFD){
        _maxFD -= 1;
    }
     cout <<"[Server " << ntohs(_addr.sin_port) << "]" << "[Client disconnected:] " << inet_ntoa(addr.sin_addr) << " | " << addr.sin_port<< " | " << sock << endl;
    _clients->erase(sock);
    FD_CLR(sock,&_fdSet);
}



void Server::broadcastToOthers(int emitterfd, map<int,Client*>*clients,string msg){
    for(map<int,Client*>::iterator iter = _clients->begin();iter != _clients->end();iter++){
        if(iter->first != emitterfd && iter->first != _serversocket){
            Socket sock(iter->first);
            try{
                sock.send(msg); // Send Userlist to new Client
            }catch(SocketException &error){
                error.show();
                string msg = "DEL_Client:" + iter->second->_usrname;
                broadcastToOthers(iter->first,clients,msg);
                deleteClient(iter->first, iter->second->_usrinfo, iter->second->_socklen);
                break;
                }
            
            }
        }
    }


thread* Server::get_pushmap(){
    return pushmap;
}

void getUserInformationFunc(int accept_socket,map<int,Client*>*clients,int _serversocket,sockaddr_in _addr,socklen_t _len,Server *s,map<int,thread*>*_threads){
    sleep(1);
    cout << "[New Thread activ]" << endl;
    thread *temp = s->get_pushmap();
    _threads->insert(std::pair<int,thread*>(accept_socket,temp));

    
    Socket sock(accept_socket);
    
    //Sending Client_List. Important for client to know which username is already in use..
    string ClList; // Client List
    for(map<int,Client*>::iterator sendclientfd = clients->begin(); sendclientfd != clients->end();sendclientfd++){
        if (sendclientfd->first != _serversocket && sendclientfd->first != accept_socket ){
            if(sendclientfd->second->_usrname != "")
                ClList += (sendclientfd->second->_usrname + ":");
        }
    }
    
    
    if(ClList.size() != 0){
        string fullmsg = "CL_List:" + ClList; //MSG_TYP:CL_LIST
        try{
        sock.send(fullmsg); // Send Userlist to new Client
        }catch(SocketException &error){
            error.show();
            for(map<int,Client*>::iterator iter = clients->begin();iter != clients->end();iter++){
                if(iter->first == accept_socket){
                    string msg = "DEL_Client:" + iter->second->_usrname;
                    s->broadcastToOthers(iter->first,clients,msg);
                    s->deleteClient(accept_socket, iter->second->_usrinfo, iter->second->_socklen);
                     _threads->erase(accept_socket);
                    break;
                }
            }
            
        }
    }
    else{
        
        string fullmsg = "CL_List:ZERO";
        try{
            sock.send(fullmsg); // Send Userlist to new Client
        }catch(SocketException &error){
            error.show();
            for(map<int,Client*>::iterator iter = clients->begin();iter != clients->end();iter++){
                if(iter->first == accept_socket){
                    string msg = "DEL_Client:" + iter->second->_usrname;
                    s->broadcastToOthers(accept_socket, clients, msg);
                    s->deleteClient(accept_socket, iter->second->_usrinfo, iter->second->_socklen);
                     _threads->erase(accept_socket);
                    break;
                }
            }
            
        }
    }
    
    string name;
    //receive Name
    try{
    name = sock.recv();
    }catch(SocketException &error){
        cout << error.show() << endl;
        for(map<int,Client*>::iterator iter = clients->begin();iter != clients->end();iter++){
            if(iter->first == accept_socket){
                string msg = "DEL_Client:" + iter->second->_usrname;
                s->broadcastToOthers(accept_socket, clients, msg);
                s->deleteClient(accept_socket, iter->second->_usrinfo, iter->second->_socklen);
                 _threads->erase(accept_socket);
                break;
            }
        }
        
        
        
    }
    //find and add name to client
    for(map<int,Client*>::iterator iter = clients->begin();iter != clients->end();iter++){
        if(iter->first == accept_socket && name != "")
            iter->second->_usrname = name;
    }
    
    //informate the others(Broadcast) außer Server und Emitter
    if(name != ""){
        string msg = "NW_Client:" + name;
        s->broadcastToOthers(accept_socket, clients, msg);
    }
    
    
    //remove thread of list
    _threads->erase(accept_socket);
    
    cout << "[Thread finished]" << endl;
   
   
}


void Server::run(){
    cout << "[Server running on Port: "  << ntohs(_addr.sin_port) << "]"<< endl;
    cout << "[waiting for incoming Request...]" << endl;
    while(1){
        sleep(1);
        fd_set fds = _fdSet; // reset fd set
        int err = select(_maxFD+1, &fds, NULL, NULL, NULL);
        if(err == EINTR)
            continue;
        if(err == -1){
            cout << "[Select error:] " << errno << endl;
            exit(errno);
        }
        
        //Server connection
        for(map<int,Client*>::iterator iter = _clients->begin(); iter != _clients->end();iter++){
        if(FD_ISSET(iter->first,&fds)){
            if(iter->first == _serversocket){
                sockaddr_in acpt;
                socklen_t acptlen;
                int acptSocket = ::accept(_serversocket,(struct sockaddr*)&acpt,&acptlen);
                
                Client *client = new Client;
                
                client->_usrinfo = acpt;;
                client->_socklen = acptlen;
                
                addClient(acptSocket,client);
                FD_SET(acptSocket,&_fdSet);
                
                pushmap = new thread(getUserInformationFunc,acptSocket,_clients,_serversocket,_addr,_len,this,_threads);
                sleep(2); // synchro
                break;
                }
                else
                    {
                        map<int,thread*>::iterator find = _threads->find(iter->first);
                        //Wenn der Socket nicht in der map gefunden wurde , so beudetet es dass der Thread fertig ist.. und nun kann ganz normal die Nachricht auf dem Socket empfangen werden.
                        if(find != _threads->end()){
                            cout << "[Thread is not finished, cant process msg]" << endl;
                            break;
                        }
                        
                        string who_what_where;
                        //Client found
                        Socket sock(iter->first);
                        try{
                            who_what_where =sock.recv();
                            cout << "[FULL MESSAGE REVEICE]: " << who_what_where << endl;
                        }catch(SocketException &error){
                            cout << error.show() << endl;
                            for(map<int,Client*>::iterator itern = _clients->begin();itern != _clients->end();itern++){
                                if(itern->first == iter->first){ // Achtung Untschiedlich , eins mit n.
                                    string msg = "DEL_Client:" + iter->second->_usrname;
                                    broadcastToOthers(iter->first, _clients, msg);
                                    deleteClient(iter->first, iter->second->_usrinfo, iter->second->_socklen);
                                    break;
                                }
                            }
                            
                            break;
                        }
                        Tokenizer tok(who_what_where,":"); //Client Anfrage würde so aussehen Username:Message:Destination
                        string who= tok.nextToken(); // who
                        string what = tok.nextToken(); // what
                        string where = tok.nextToken();// where
                        
                        cout << "[Message saperate]" << endl;
                        cout << "[Name posted:]" << who << " [Packet:] " << what << " [Destination:] " << where << endl;
                        
        
                        if(where == "ALL"){
                            //Broadcast
                            cout << "[Destination Type:]"<< "<<Broadcast>>" << endl;
                            string broadcastmsg = "USR_BROADCAST:" + iter->second->_usrname + ":" + what; // MSG_TYP:NAME:CONTENT
                            broadcastToOthers(iter->first, _clients, broadcastmsg);
                        }
                        else{
                            //search and send to private client
                            cout << "[Destination Type:]"<< "<<Unicast>>" << endl;
                            map<int,Client*>::iterator iterTres;
                            for(iterTres=_clients->begin(); iterTres != _clients->end();iterTres++){
                                if(iterTres->second->_usrname == where){
                                    Socket sendsock(iterTres->first);
                                    string privatmsg = "USR_UNICAST:" + iter->second->_usrname + ":" + what; // MSG_TYP:NAME:CONTENT
                                    try{
                                        sendsock.send(privatmsg);
                                    }catch(SocketException &error){
                                        cout << error.show() << endl;
                                        string msg = "DEL_Client:" + iter->second->_usrname;
                                         broadcastToOthers(iterTres->first, _clients, msg);
                                         deleteClient(iterTres->first, iterTres->second->_usrinfo, iterTres->second->_socklen);
                                         break;
                                    }
                                    break;
                                }
                            }
                            if(iterTres == _clients->end()){
                                cout << "[Destination Client not found!]" << endl;
                                Socket sendsock(iter->first);
                                string errmsg="ERR_NO_USR:" + iter->second->_usrname + "No User found."; // MSG_TYP:NAME:CONTENT
                                try{
                                    sendsock.send(errmsg);
                                }catch(SocketException &error){
                                    cout << error.show() << endl;
                                    string msg = "DEL_Client:" + iter->second->_usrname;
                                    broadcastToOthers(iter->first, _clients, msg);
                                    deleteClient(iter->first, iter->second->_usrinfo, iter->second->_socklen);
                                    break;
                                    }
                            }
                        }
                    }
                }
                else
                    continue;
        } // for
    } // while
} // run