#include "Server.h"
#include <arpa/inet.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>

Server::Server(int PORT, bool BroadcastPublically) //Port = port to broadcast on. BroadcastPublically = false if server is not open to the public (people outside of your router), true = server is open to everyone (assumes that the port is properly forwarded on router settings)
{
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (BroadcastPublically) //If server is open to public
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else //If server is only for our router
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally

    sListen = socket(AF_INET, SOCK_STREAM, 0); //Create socket to listen for new connections
    if ((::bind(sListen, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) //Bind the address to the socket, if we fail to bind the address..
	{
        string ErrorMsg = "Failed to bind the address to our listening socket.";
        std::cout << ErrorMsg << std::endl;
		exit(1);
	}
    if ((listen(sListen, SOMAXCONN)) != 0) //Places sListen socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max Connections, if we fail to listen on listening socket...
	{
        string ErrorMsg = "Failed to listen on listening socket.";
        std::cout << ErrorMsg << std::endl;
		exit(1);
	}
	serverptr = this;
    accList.clear();
    GetAllAccounts();
    // get account from File

    // get file path
    // QFile file(":/accs.txt");
    // if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     std::cout << "Error: Failed to open the file accs.txt.";
    //     exit(0);
    // }
    // QTextStream in(&file);
    // if(!in.atEnd())
    //     accsFilePath = in.readLine();
    // file.close();
    // if(accsFilePath.length() == 0) {
    //     std::cout << "Error: You don't have account file path.";
    //     exit(0);
    // }
    // file.setFileName(accsFilePath);
    // if(file.exists()) {
    //     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //         std::cout << "Error: Failed to open the file " << accsFilePath.toStdString();
    //         exit(0);
    //     }
    //     bool isID = true;
    //     Account tmpAcc;
    //     while (!in.atEnd()) {
    //         if(isID)
    //             tmpAcc.ID = in.readLine();
    //         else {
    //             tmpAcc.PassW = in.readLine();
    //             accList.push_back(tmpAcc);
    //         }
    //         isID = !isID;
    //     }
    // }
    // else {
    //     if (!file.open(QIODevice::WriteOnly)) {
    //         std::cout << "Error: Failed to create the file " << accsFilePath.toStdString() << endl;
    //         std::cout << file.error() << endl;
    //         exit(0);
    //     }
    // }
    // file.close();
}

void Server::Signup(QString username, QString password, int elo){
    if(connector.openConnection()){
        qDebug() << "Connected to the database!";
    }else{
        qDebug() << "Cannot connect to the database!";
        exit(66);
    }
    QSqlQuery query;
    QString sQuery = "INSERT INTO accounts (user_id, password, elo) VALUES (:username, :password,:elo)";
    query.prepare(sQuery);
    query.bindValue(":username",username);
    query.bindValue(":password",password);
    query.bindValue(":elo",elo);
    if (query.exec()) {
        qDebug() << "Data inserted successfully.";
    } else {
        qDebug() << "Error executing query:";
        qDebug() << query.lastError().text();
    }
    connector.closeConnection();
}

void Server::GetAllAccounts()
{
    if(connector.openConnection()){
        qDebug() << "Connected to the database!";
    }else{
        qDebug() << "Cannot connect to the database!";
        exit(66);
    }
    QSqlQuery query = connector.executeQuery("SELECT * FROM accounts");
    while (query.next()) {
        QString user_id = query.value(0).toString();
        QString password = query.value(1).toString();
        int elo = query.value(2).toInt();

        qDebug() << "USER ID: " << user_id;
        qDebug() << "PW: " << password;
        qDebug() << "ELO: " << elo;

        Account tmpAcc;
        tmpAcc.ID = user_id;
        tmpAcc.PassW = password;
        tmpAcc.elo = elo;
        accList.push_back(tmpAcc);

    }
    connector.closeConnection();
}

bool Server::SendString(int ID, string & _string)
{
    int RetnCheck = send(Connections[ID], _string.c_str(), 512, 0); //Send string buffer
    if (RetnCheck < 0) //If failed to send string buffer
		return false; //Return false: Failed to send string buffer
	return true; //Return true: string successfully sent
}

bool Server::GetString(int ID, string & _string)
{
	char buffer[512];
    int RetnCheck = recv(Connections[ID], buffer, 512, 0);
	_string = buffer;
	// TO DO:
	// when recv failed ,delete it!
    if (RetnCheck < 0)
		return false;
	cout << "get:" << endl << _string << endl;
	return true;
}

void Server::sendMessToClients(string Message)
{
	cout << "send:" << endl << Message << endl;
    unordered_map<int, int>::iterator it;
	for (it = Connections.begin(); it != Connections.end(); ++it)
	{
		if (!SendString(it->first, Message)) //Send message to connection at index i, if message fails to be sent...
			cout << "Failed to send message to client ID: " << it->first << endl;
	}
	//----------------------------old loop-----------------------------------------------------------
	//for (int i = 0; i < TotalConnections; i++)
	//{
	//	//if (i == ID) //If connection is the user who sent the message...
	//		//continue;//Skip to the next user since there is no purpose in sending the message back to the user who sent it.

	//	if (!SendString(i, Message)) //Send message to connection at index i, if message fails to be sent...
	//		cout << "Failed to send message to client ID: " << i << endl;
	//}
	//-----------------------------------------------------------------------------------------------
}

bool Server::ListenForNewConnection()
{
    int newConnection = accept(sListen, (struct sockaddr*)&servaddr, &addrlen); //Accept a new connection
    if (newConnection < 0) //If accepting the client connection failed
	{
		cout << "Failed to accept the client's connection." << endl;
		return false;
	}
	else //If client connection properly accepted
	{
		cout << "Client Connected! ID:" << allID << endl;
        Connections.insert(pair<int, int>(allID, newConnection));
        threadList[allID] = std::thread(&Server::ClientHandlerThread, this, allID);
		player newPlayer(new Player(allID));
		PlayerList.insert(pair<int, player>(allID, newPlayer));
		allID++;
		TotalConnections += 1; //Incremenent total # of clients that have connected
		return true;
	}
}

bool Server::Processinfo(int ID)
{

	string Message; //string to store our message we received
	if (!GetString(ID, Message)) //Get the chat message and store it in variable: Message
		return false; //If we do not properly get the chat message, return false
					  //Next we need to send the message out to each user
	cJSON *json, *json_type;
	json = cJSON_Parse(Message.c_str());
	json_type = cJSON_GetObjectItem(json, "Type");
	if (json_type == NULL)
	{
		sendMessToClients(Message);
		cout << "Processed chat message packet from user ID: " << ID << endl;
		cJSON_Delete(json);
		return true;
	}
	else
	{
		string type = json_type->valuestring;
		if (type == "Message")
			sendMessToClients(Message);
        else if (type == "SignUp") {
            cJSON *user_ID_Json;
            user_ID_Json = cJSON_GetObjectItem(json, "ID");
            cJSON *user_PW_Json;
            user_PW_Json = cJSON_GetObjectItem(json, "PW");
            cJSON *user_Elo_Json;
            user_Elo_Json = cJSON_GetObjectItem(json, "ELO");
            QString reg_ID = user_ID_Json->valuestring;
            QString reg_PW = user_PW_Json->valuestring;
            int reg_ELO = user_Elo_Json->valueint;
            Signup(reg_ID, reg_PW, reg_ELO);

            // bool flag = false;
            // for(int i = 0; i < accList.size(); i++)
            //     if(accList[i].ID == reg_ID ) {
            //         flag = true;
            //         break;
            //     }
            // if(!flag) {
            //     accList.push_back(Account(reg_ID, reg_PW));
            //     // QFile file(accsFilePath);
            //     // if (!file.open(QIODevice::Append | QIODevice::Text))
            //     //     std::cout << "Failed to open file for writing\n";
            //     // else {
            //     //     QTextStream out(&file);
            //     //     out << reg_ID << Qt::endl << reg_PW << Qt::endl;
            //     //     file.close();
            //     // }
            // }else{
            //     sendSystemInfo(ID, "SignUp")
            // }
        }
        else if (type == "LogIn") {
            cJSON *user_ID_Json;
            user_ID_Json = cJSON_GetObjectItem(json, "ID");
            cJSON *user_PW_Json;
            user_PW_Json = cJSON_GetObjectItem(json, "PW");
            QString user_ID = user_ID_Json->valuestring;
            QString user_PW = user_PW_Json->valuestring;
            bool flag = false, isLogin = false;
            for(int i = 0; i < accList.size(); i++) {
                if(accList[i].ID == user_ID && accList[i].PassW == user_PW) {
                    if(accList[i].login){
                        isLogin = true;
                        break;
                    }
                    flag = true;
                    accList[i].login = true;
                    break;
                }
            }
            if(flag) {
                sendSystemInfo(ID, "LogIn_SUCCESS");
                string MOTD = "MOTD: Welcome! This is the message of the day!.";
                SendString(ID, MOTD);
                sendGameList(ID);
                OnlineUserList[ID] = user_ID;
            }
            else{
                if(!isLogin) sendSystemInfo(ID, "LogIn_FAILED_ID_PW");
                else sendSystemInfo(ID,"LogIn_FAILED_login");
            }

        }
		else if (type == "CreateRoom")
		{
			if (GameList.size() >= 6)
			{
				cJSON_Delete(json);
				sendSystemInfo(ID, "List_Full");
				return true;
			}
			if (PlayerList[ID]->isFree())
			{
				cJSON *Username;
				Username = cJSON_GetObjectItem(json, "User");
				int gameID = GameNum;
				GameNum++;
				onlineGame newGame(new Game(gameID, ID, Username->valuestring));
				PlayerList[ID]->hostGame(gameID, newGame);
				newGame->hostIs(PlayerList[ID]);
				GameList.insert(pair<int, onlineGame>(gameID, newGame));
				sendSystemInfo(ID, "WaitingForSomeoneJoining");
				sendGameList(-1);
			}
			else
			{
				cout << "The player is already in game!" << endl;
				cJSON_Delete(json);
				return true;
			}
		}
		else if (type == "PlayAgain")
		{
			if (PlayerList[ID]->AreYouInGame() >= 0)
			{
				int HID = PlayerList[ID]->AreYouInGame();
				std::cout << "booltest first" << std::endl;
				GameList[HID]->booltest();
				GameList[HID]->playAgain(ID);
				int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
				if (anotherPlayer >= 0)
				{
					if (GameList[HID]->can_Play_again())
					{
						if (sendSystemInfo(ID, "PlayAgain") && sendSystemInfo(anotherPlayer, "PlayAgain"))
							GameList[HID]->reset_play_again();
					}
				}
				else
					GameList[HID]->reset_play_again();
			}
		}
		else if (type == "move")
		{
			if (PlayerList[ID]->AreYouInGame() >= 0)
			{
				cout << "yes, I am in game" << endl;
				int HID = PlayerList[ID]->AreYouInGame();
				int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
				if (anotherPlayer >= 0)
				{
					cout << "yes, I have palyer" << endl;
					SendString(anotherPlayer, Message);
				}
			}
		}
		else if (type == "JoinRoomRequest")
		{
			cJSON *Games_ID;
			Games_ID = cJSON_GetObjectItem(json, "ID");
			cJSON *Username;
			Username = cJSON_GetObjectItem(json, "User");
			int gameID = Games_ID->valueint;
			string p2name = Username->valuestring;
			cout << "got Join room request from ID: " << Games_ID->valueint << endl;
			// TO DO:
			// if game is nolonger exist
			// send message RoomClose
			// return
			if (GameList[gameID]->isFull())
			{
				sendSystemInfo(ID, "RoomFull");
				cJSON_Delete(json);
				return true;
			}
			else if (GameList[gameID]->hostsID() == ID)
			{
				cJSON_Delete(json);
				return true;
			}
			else
			{
				GameList[gameID]->Joinin(ID, PlayerList[ID], p2name);
				PlayerList[ID]->JoininGame(gameID, GameList[gameID]);
				sendGameList(-1);
				int hostID = GameList[gameID]->hostsID();
				sendSystemInfo(hostID, "HostStartPlaying");
				if(sendSystemInfo(ID, "JoinRoom"))
					sendSystemInfo(hostID, "SomeoneJoin_Successfully");
			}
		}
		else if (type == "BackToLobby")
		{
			if (PlayerList[ID]->AreYouInGame() >= 0)
			{
				int HID = PlayerList[ID]->AreYouInGame();
				int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
				if (anotherPlayer >= 0)
				{
					sendSystemInfo(anotherPlayer, "ReturnTolobby");
					PlayerList[anotherPlayer]->returnToLobby();
				}
				PlayerList[ID]->returnToLobby();
				GameList.erase(HID);
				sendGameList(-1);
			}
			else
				PlayerList[ID]->returnToLobby();
		}
		else if (type == "CancelHost")
		{
			if (PlayerList[ID]->AreYouInGame() >= 0)
			{
				int HID = PlayerList[ID]->AreYouInGame();
				int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
				if (anotherPlayer >= 0)
				{
					sendSystemInfo(anotherPlayer, "ReturnTolobby");
					PlayerList[anotherPlayer]->returnToLobby();
				}
				PlayerList[ID]->returnToLobby();
				GameList.erase(HID);
				sendGameList(-1);
			}
			else
				PlayerList[ID]->returnToLobby();
		}
        else if (type == "GetOnlineUsers")
        {
            string res = "";
            for(auto e : OnlineUserList)
                res += (e.second.toStdString() + ",");
            cJSON *json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "Type", "OnlineUsersList");
            cJSON_AddStringToObject(json, "Response", res.c_str());
            char *JsonToSend = cJSON_Print(json);
            cJSON_Delete(json);
            cout << "send:" << endl << JsonToSend << " To: " << ID << endl;
            string Send(JsonToSend);
            SendString(ID, Send);
        }
		else if (type == "Exit")
			return false;
	}
	cout << "Processed chat message packet from user ID: " << ID << endl;
	cJSON_Delete(json);
	return true;
}

bool Server::CreateGameList(string & _string)
{
	cJSON *json = cJSON_CreateObject();
	if (cJSON_AddStringToObject(json, "Type", "List_of_Rooms") == NULL)
	{
		cout << "type add failed." << endl;
		cJSON_Delete(json);
		return false;
	}
	cJSON * Lists = NULL;
	Lists = cJSON_AddArrayToObject(json, "List");
	if (Lists == NULL)
	{
		cout << "List add failed." << endl;
		cJSON_Delete(json);
		return false;
	}
	//if (GameList.size() == 0)
	//{
	//	_string = "NULL";
	//	return true;
	//}
	unordered_map<int, onlineGame>::iterator it;
	for (it = GameList.begin(); it != GameList.end(); ++it)
	{
		cJSON *GAME = cJSON_CreateObject();

		if (cJSON_AddStringToObject(GAME, "name", it->second->hostName.c_str()) == NULL)
		{
			cJSON_Delete(json);
			return false;
		}
		if (cJSON_AddNumberToObject(GAME, "id", it->second->id) == NULL)
		{
			cJSON_Delete(json);
			return false;
		}
		if (cJSON_AddNumberToObject(GAME, "isplay", it->second->isplay) == NULL)
		{
			cJSON_Delete(json);
			return false;
		}
		string p2name;
		if (it->second->isplay)
			p2name = it->second->p2Name;
		else
			p2name = "";
		if (cJSON_AddStringToObject(GAME, "p2name", p2name.c_str()) == NULL)
		{
			cJSON_Delete(json);
			return false;
		}
		cJSON_AddItemToArray(Lists, GAME);
	}

	_string = cJSON_Print(json);
	cJSON_Delete(json);
	return true;;
}

bool Server::sendSystemInfo(int ID, string InfoType)
{
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "Type", "System");
	cJSON_AddStringToObject(json, "System_Info", InfoType.c_str());
	char *JsonToSend = cJSON_Print(json);
	cJSON_Delete(json);
	cout << "send:" << endl << JsonToSend << " To: " << ID << endl;
	string Send(JsonToSend);
	return SendString(ID, Send);
}

bool Server::sendGameList(int ID)
{
	string Message;
	if (!CreateGameList(Message))
	{
		cout << "create game list JSON failed!" << endl;
		return false;
	}
	if (Message == "NULL")
		return true;
	if (ID < 0)
		sendMessToClients(Message);
	else
		SendString(ID, Message);
	return true;
}


void Server::ClientHandlerThread(int ID) //ID = the index in the SOCKET Connections array
{
	while (true)
	{
		if (!serverptr->Processinfo(ID))
			break;
	}
	cout << "Lost connection to client ID: " << ID << endl;
    close(serverptr->Connections[ID]);
    for(int i = 0; i < accList.size(); i++) {
        if(accList[i].ID == OnlineUserList[ID]) {
            accList[i].login = false;
            break;
        }
    }
    OnlineUserList.erase(ID);
	if (serverptr->PlayerList[ID]->AreYouInGame() >= 0)
	{
		cout << "delete its game room: " <<  endl;
		int HID = serverptr->PlayerList[ID]->AreYouInGame();
		int anotherPlayer = serverptr->GameList[HID]->anotherPlayerID(ID);
		if (anotherPlayer >= 0 && serverptr->PlayerList[anotherPlayer])
		{
			serverptr->sendSystemInfo(anotherPlayer, "LostConnection");
			serverptr->PlayerList[anotherPlayer]->returnToLobby();
		}
		serverptr->GameList.erase(HID);
		serverptr->sendGameList(-1);
	}
	serverptr->PlayerList.erase(ID);
    serverptr->Connections.erase(ID);
    serverptr->TotalConnections -= 1;
}
