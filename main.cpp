//
//  main.cpp
//  Vsy_Projekt
//
//  Created by Dennis Halter on 18.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#include <iostream>
#include "ServerSocket.hpp"
#include "Server.hpp"
#include <sys/wait.h>

int main(int argc, const char * argv[]) {
    
    pid_t pidchild1;
    pid_t pidchild2;
    int wstatus,wstatus2;
    pid_t w,k;
    
    
    if((pidchild1 = fork()) == 0){
        cout << "Server 1 starting..." << endl;
        pidchild1 = getpid();
        cout << "PID CHILD1: " << pidchild1 << endl;
        ServerSocket serversock(6200,10);
        Server server(serversock.getSockFD(),serversock.getSockAddr(),serversock.getAddrLen());
        server.run();
        
    }
    
    sleep(1);
    
    if((pidchild2 = fork()) == 0){
        cout << "Server 2 starting..." << endl;
        pidchild2 = getpid();
        cout << "PID CHILD2: " << pidchild2 << endl;
        ServerSocket serversock(6201,10);
        Server server(serversock.getSockFD(),serversock.getSockAddr(),serversock.getAddrLen());
        server.run();
    }
   
        w = waitpid(pidchild1,&wstatus,0);
        k = waitpid(pidchild2, &wstatus2,0);
        if(w == -1){
            cout << "ERROR waitpid.." << endl;
        }
        else{
        cout << "COUT PROCESS PID TERMINATED: " << w << "STATUS: " << wstatus << endl;
            //Hier kann man seine Fantasy spielen lassen. Bsp. Kann bei jedem Shutdown einfach ein neuer Server angelegt werden..
        }
    
    
        if(k == -1){
        cout << "ERROR waitpid.." << endl;
        }
        else{
            cout << "COUT PROCESS PID TERMINATED: " << k<< "STATUS: " << wstatus2 << endl;
            //Hier kann man seine Fantasy spielen lassen. Bsp. Kann bei jedem Shutdown einfach ein neuer Server angelegt werden..
        }

    
    
    sleep(10000);
    
    return 0;
}
