
#include "Socket.hpp"
#include <string>
#include <unistd.h>
#include <thread>
#include <iostream>
#include "tokenizer.hpp"
#include "SocketException.hpp"
#include <iomanip>
#include <map>

using namespace std;

int port=6200;
string ip = "127.0.0.1";
string name;

void receive_thread(Socket *sock,map<string,int> *Clientlist){
    while(1){
        std::string ressy;
        try{
        ressy = sock->recv();
        }catch(SocketException &error){
            cout << "[system Server disconnected]" << endl;
            cout << "[system reconnection...]" << endl;
            sock->close(); // lösche aktuellen Socket
            Clientlist->clear(); // lösche Inhalt Clientlist
            port++;
            //in while Schleife mehrere "Connection Tests" möglich
            sock = new Socket(ip,port); // connect to new Port
            cout << "[system reconnection successfull]" << endl;
            try{
                ressy=sock->recv();
                sock->send(name);
            }catch(SocketException &error){
                cout << error.show() << endl;
                cout << "Next Server down :D " << endl;
                continue;
            }
            
        }
        
        Tokenizer tok(ressy,":");
        
        string typ = tok.nextToken();
        
        if(typ == "CL_List"){
                for(int i=1; i < tok.countTokens();i++){
                string name =  tok.nextToken();
                    if(name == "ZERO"){
                        cout << "[system]" << "[" << "0"  << " online] "<< endl;
                        continue;
                    }
                cout << "[system]" << "[" << name  << " online] "<< endl;
                Clientlist->insert(pair<string,int>(name,1));
                }
        }
        else if(typ == "NW_Client"){
            string name = tok.nextToken();
            cout << "[system]" << "[" <<  name  << " connected to Server]" << endl;
            map<string,int>::iterator iter = Clientlist->find(name);
            if(iter == Clientlist->end()){
                //Add new Client to Clienlist
                Clientlist->insert(pair<string,int>(name,1));
            }
        }
        else if(typ == "DEL_Client"){
            string name = tok.nextToken();
            if(name == ""){
            }
            else{
                cout << "[system]" << "[" << name << " disconnected from Server]" << endl;
                map<string,int>::iterator iter = Clientlist->find(name);
                Clientlist->erase(iter);
            }
        }
        else if(typ == "USR_BROADCAST"){
            cout << "[ALLE]"<< "["<< tok.nextToken()<< "]" <<  tok.nextToken()<< endl; //USRNAME:CONTENT
        }
        else if(typ == "USR_UNICAST"){
            cout <<"[PRIVAT]" << "[" << tok.nextToken() << "]" << tok.nextToken()<< endl; //USRNAME:CONTENT
        }
        else if(typ == "ERR_NO_USR") {
            cout << "[sytsem][something went wrong, call Apple for Fix]" << endl;
        }
    }
}


int main(){
    map<string,int> *Clientlist = new map<string,int>;
    Socket *sock;
    try{
     sock= new Socket(ip,port);
    }catch(SocketException &error){
        //in while schleife mehrere Server "Connection Tests" möglich
        cout << error.show() << endl;
        cout << "[system Server disconnected]" << endl;
        cout << "[system reconnection...]" << endl;
        sock->close(); // lösche aktuellen Socket
        port++;
        sock = new Socket(ip,port); // connect to new Port
        cout << "[system reconnection successfull]" << endl;

    }
    thread rcv(receive_thread,sock,Clientlist);
    sleep(1); //synchro
    cout << "[Welcome to Chat *_*]" << endl;
    cout << "[HOW TO USE! Um eine Nachricht an ALLE zu senden (ALL:DEINE NACHRICHT) eingeben. Bsp ALL:Hey" << endl;
    cout << "Fuer eine PRIVATE Nachricht (BENUTZERNAME:DEINE NACHRICHT) eingeben. Bsp SEXYADMIN:Hey Sexy]" << endl;
    cout << "PRESS Q TO PRINT ALL ONLINE CLIENTS and also nice to look up names" << endl;
    cout << "[ERST Chatnamen eingeben!]" << endl;
    while(1){
    sleep(1);
    getline(cin,name);
    map<string,int>::iterator iter;
    for(iter = Clientlist->begin(); iter != Clientlist->end();iter++){
        if(iter->first == name){
            cout << "[Clientname already in use]" << endl;
            break;
        }
    }
        if(name == ""){
            cout << "[enter your name <.<]" << endl;
            continue;
        }
        if(iter == Clientlist->end() && name != ""){
            //Name ist frei
            try{
                cout << "LOL" << endl;
                sock->send(name);
                break;
            }catch(SocketException &error){
                cout << error.show() << endl;
                cout << "[system Server disconnected]" << endl;
                cout << "[system reconnection...]" << endl;
                sock->close(); // lösche aktuellen Socket
                Clientlist->clear(); // lösche Inhalt Clientlist
                port++;
                sock = new Socket(ip,port); // connect to new Port
                cout << "[system reconnection successfull]" << endl;
                try{
                sock->recv();
                sock->send(name);
                    break;
                }catch(SocketException &error){
                    cout << error.show() << endl;
                }
                sleep(1);
                continue;
            }

        }
    else
        continue; // neunen Namen eingeben
    
    }
    cout << "[system][Connection established on " << ip << " :port " << port << endl;
    cout << "[system][Sexy Admin is ready to use *_*]" << endl;
    //cin.ignore();
    while(69){
        sleep(1);
        string msg,msg_sep;
        string dest;
        getline(cin,msg);
        sleep(1);//synchro
        
        if(msg == "Q"){
            for(map<string,int>::iterator iter = Clientlist->begin(); iter != Clientlist->end();iter++){
                if(iter->first == name){
                    continue;
                }
                cout << "[system]" << "[" << iter->first << " online] "<< endl;
            }
            continue;
        }
        Tokenizer tok(msg,":");
        dest = tok.nextToken();
        msg_sep = tok.nextToken();
        
        if(dest == "ALL"){
            try{
                string fullmsg = name + ":" + msg_sep + ":" + dest;
                sock->send(fullmsg);
            }catch(SocketException &error){
                cout << "[system]" << "[" << error.show() << "]" << endl;
                cout << "[system Server disconnected]" << endl;
                cout << "[system reconnection...]" << endl;
                sock->close(); // lösche aktuellen Socket
                Clientlist->clear(); // lösche Inhalt Clientlist
                port++;
                int v = rand() % 5;
                sleep(v);
                sock = new Socket(ip,port); // connect to new Port
                cout << "[system reconnection successfull]" << endl;
                try{
                    sock->recv();
                    sock->send(name);
                    break;
                }catch(SocketException &error){
                    cout << error.show() << endl;
                    // Man könnte daraufs eine Funktion machen , die eine Serverliste durchläuft
                    //und versucht sich zu jedem der Sever zu verbinden , falls der Vorgänger nicht geklappt hat.
                    //Für uns reichen ja 2 Server usw..
                }

                continue;
            }
        }   
        else{
            
            //Suche in Clientlist ob Cl vorhanden ist
            map<string,int>::iterator iter = Clientlist->find(dest);
            if(iter == Clientlist->end()){
                cout <<"[system][Wrong Input or Client is not avaible]" << endl;
                continue;
            }
            //wenn ja sende Nachricht
            else{
                try {
                    string fullmsg = name + ":" + msg_sep + ":" + dest;
                    sock->send(fullmsg);
                } catch (SocketException &error) {
                    cout << "[system]" << "[" << error.show() << "]" << endl;
                    cout << "[system Server disconnected]" << endl;
                    cout << "[system reconnection...]" << endl;
                    sock->close(); // lösche aktuellen Socket
                    Clientlist->clear(); // lösche Inhalt Clientlist
                    port++;
                    int v = rand() % 5;
                    sleep(v);
                    sock = new Socket(ip,port); // connect to new Port
                    cout << "[system reconnection successfull]" << endl;
                    try{
                        sock->recv();
                        sock->send(name);
                        break;
                    }catch(SocketException &error){
                        cout << error.show() << endl;
                        // Man könnte daraufs eine Funktion machen , die eine Serverliste durchläuft
                        //und versucht sich zu jedem der Sever zu verbinden , falls der Vorgänger nicht geklappt hat.
                        //Für uns reichen ja 2 Server usw..
                    }
                    continue;
                }
            }
        }
    }
    
    return 0;
}
