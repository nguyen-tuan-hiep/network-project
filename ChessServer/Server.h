#pragma once

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#include <string>
#include <memory>
#include <iostream>
#include "Games.h"
#include "Player.h"
#include <unordered_map>
#include "cJSON/cJSON.h"
#include <thread>
#include <QVector>

//typedef std::vector<std::vector<unique_ptr>> BOARD;

using namespace std;

typedef shared_ptr<Game> onlineGame;
typedef shared_ptr<Player> player;

struct Account {
    QString ID;
    QString PassW;
    Account() {};
    Account(QString id, QString pw) {
        ID = id;
        PassW = pw;
    }
};

class Server
{
public:
	Server(int PORT, bool BroadcastPublically = false);
	bool ListenForNewConnection();

private:
	bool SendString(int ID, string & _string);
	bool GetString(int ID, string & _string);
	void sendMessToClients(string Message);
	bool Processinfo(int ID);
	bool CreateGameList(string & _string);
	bool sendSystemInfo(int ID, string InfoType);
	bool sendGameList(int ID); // if ID <0 ,means send gamelist to all clients
	void deleteGame(int ID);

    void ClientHandlerThread(int arg);

private:
	//-------------------------------------------
	// online variables:
	//-------------------------------------------
    unordered_map<int, int> Connections;
	//SOCKET Connections[512];
	int TotalConnections = 0;
	int allID = 0;
    struct sockaddr_in servaddr;              //Address that we will bind our listening socket to
    unsigned int addrlen = sizeof(servaddr);
    int sListen;
	//---------------------------------------
	//Logic variables:
	//----------------------------------------
	int GameNum = 0;
	unordered_map<int, onlineGame> GameList;
	unordered_map<int, player> PlayerList;
    std::thread threadList[512];
    QVector<Account> accList;
};

static Server * serverptr; 
