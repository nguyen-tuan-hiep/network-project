#include "ServerSocket.h"

ServerSocket::ServerSocket(){
  WSADATA wsa;
  struct sockaddr_in server;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      error("Failed WSAStartup.\n");

  std::cout << "Winsock Initialised.\n";

  if ((server_so = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      error("Failed socket.\n");
      
  std::cout << "Socket created.\n";

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons(5500);

  if (bind(server_so, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
      error("Bind failed.\n");

  if (listen(server_so, MAX_CLIENT) < 0)
      error("Listen failed.\n");

  initializeAccountList();

  std::cout << "Waiting for incoming connections..\n";
};

void ServerSocket::close()
{
    closesocket(this->server_so);
    WSACleanup();
    return;
}

void ServerSocket::error(std::string message)
  {
      std::cerr << message << ": " << WSAGetLastError() << "\n";
      exit(1);
      return;
  }

void ServerSocket::error(std::string message, bool isThread)
  {
      std::cerr << message << ": " << WSAGetLastError() << "\n";
      if (!isThread)
        exit(1);
      return;
  }

void ServerSocket::handleClient(int client_socket)
{
  char buffer[RECEIVE_BUFFER_SIZE];
  std::string message;
  std::string code;
  int roomIndex = -1;
  int bytes_received;
  std::thread::id threadId = std::this_thread::get_id();

  u_long mode = 1; // Enable non-blocking mode
  if (ioctlsocket(client_socket, FIONBIO, &mode) != 0)
    ServerSocket::error("Failed to set the socket to non-blocking mode", true);

  while (1)
  {
    bytes_received = recv(client_socket, buffer, RECEIVE_BUFFER_SIZE, 0);

    if (bytes_received > 0)
    {
      std::stringstream ss(buffer);
      std::string token;
      std::getline(ss, token, '\n');
      std::cout << "Token: " << token << '\n';

      if (!token.compare(CREATE_ROOM_CODE))
      {
        handleCreateRoomSignal(client_socket, ss, roomIndex, threadId);
      }
      else if (!token.compare(LEAVE_ROOM_CODE))
      {
        handleLeaveRoomSignal(client_socket, ss, roomIndex, threadId);
      }
      else if (!token.compare(JOIN_ROOM_CODE))
      {
        handleJoinRoomSignal(client_socket, ss, roomIndex, threadId);
      }
      else if (!token.compare(START_GAME_CODE)){
        handleStartGameSignal(client_socket, ss, roomIndex);
      }
      else if (!token.compare(MOVE_CODE)){
        handleMoveSignal(client_socket, ss, roomIndex, threadId);
      }
      else if (!token.compare(READY_CODE)){
        handleReadySignal(client_socket, ss, roomIndex);
      }
      else if (!token.compare(RESIGN_CODE)){
        handleResignSignal(client_socket, ss, roomIndex, threadId);
      }
      else if (!token.compare(LOGIN_CODE)){
        handleLoginSignal(client_socket, ss);
      }  
    }
    else if (bytes_received == 0)
    {
      std::cout << "Connection closed by the client." << std::endl;
      break;
    }
    else
    {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK)
      {
        std::cerr << "Failed to receive data. Error code: " << error << std::endl;
        break;
      }
      // No data available in the receive buffer
      // Do other work or sleep for a while before calling recv() again
    }

    if (roomIndex >= 0)
    {
      handleRoomStatus(client_socket, roomIndex, threadId);
    }
  }

  std::cout << "Closing client socket..";
  closesocket(client_socket);
}

void ServerSocket::handleCreateRoomSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId){
  std::string token, code, message;

  std::getline(ss, token, '\n');
  std::cout << "Create room with owner : " << token << '\n';
  std::cout << "Creating room \n";

  std::string roomCode = gen_random(6);
  for (int i = 0; i < MAX_CLIENT; i++)
  {
    if (!roomList[i].isRoomOccupied())
    {
      roomIndex = i;
      roomList[i].setRoomCode(roomCode);
      roomList[i].setRoomOccupied(true);
      roomList[i].setOwner(threadId, token);
      code.assign(CREATE_ROOM_CODE);
      message = code + '\n' + roomCode + '\n';
      send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
      std::cout << "Room " << i << " is occupied with code " << roomCode << '\n';
      break;
    }
  }
}

void ServerSocket::handleJoinRoomSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId){
  std::string token, code, message;

  bool foundRoom = false;
  bool roomFull = false;
  std::getline(ss, token, '\n');
  std::cout << "Find room with code : " << token << '\n';

  for (int i = 0; i < MAX_CLIENT; i++)
  {
    if (roomList[i].isRoomOccupied())
    {
      if (roomList[i].getRoomCode() == token)
      {
        foundRoom = true;
        if (roomList[i].isRoomFull())
        {
          roomFull = true;
          break;
        }
        std::getline(ss, token, '\n');
        std::cout << "Player : " << token << " is joing room\n";
        roomIndex = i;
        roomList[i].setGuest(threadId, token);
        code.assign(JOIN_ROOM_CODE);
        message = code + '\n' + "1\n" + roomList[i].getOwnerName() + '\n';
        send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
        std::cout << "Room " << i << " with code " << roomList[i].getRoomCode() << " has guest joined\n";
        break;
      }
    }
  }
  if (!foundRoom)
  {
    code.assign(JOIN_ROOM_CODE);
    message = code + '\n' + "0\n";
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    std::cout << "Room with code " << token << " can not be found\n";
  }
  else if (roomFull)
  {
    code.assign(JOIN_ROOM_CODE);
    message = code + '\n' + "2\n";
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    std::cout << "Room with code " << token << " is full\n";
  }
}

void ServerSocket::handleLeaveRoomSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId){
  std::string token, code, message;
  if (roomIndex < 0){
    std::cout << "Player not in a room but send leave room request. Ignoring...\n";
    return;
  }

  if (roomList[roomIndex].isOwner(threadId))
  {
    std::cout << "Room " << roomIndex << " with code " << roomList[roomIndex].getRoomCode() << " has lost its owner\n";
    roomList[roomIndex].removeOwner();
    roomIndex = -1;
    code.assign(LEAVE_ROOM_CODE);
    message = code + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }
  else if (roomList[roomIndex].isGuest(std::this_thread::get_id()))
  {
    std::cout << "Room " << roomIndex << " with code " << roomList[roomIndex].getRoomCode() << "has lost its guest\n";
    roomList[roomIndex].removeGuest();
    roomIndex = -1;
    code.assign(LEAVE_ROOM_CODE);
    message = code + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }
  else
  {
    std::cout << "Something is wrong cant find room to leave";
  }
}

void ServerSocket::handleLoginSignal(int client_socket, std::stringstream& ss){
  std::string username, password, token, code, message;
  
  std::getline(ss, token, '\n');
  std::cout << token << '\n';
  username = token;
  std::getline(ss, token, '\n');
  std::cout << token << '\n';
  password = token;
  //Find account logic here
  Account::Status status;
  std::string name;
  std::string statusMessage;
  for (auto& account : accountList) {
    status = account.second.attempLogin(username, password);
    if (status != Account::Status::logged_out){
      name = account.second.getName();
      break;
    }
  }
  code.assign(LOGIN_CODE);
  message = code + '\n' + Account::Stringify(status) + '\n' + name + '\n';
  send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleReadySignal(int client_socket, std::stringstream& ss, int& roomIndex){
  std::string token, code, message;

  std::getline(ss, token, '\n');
  std::cout << token << '\n';
  roomList[roomIndex].setGuestReady(std::stoi(token));
  roomList[roomIndex].setNewGuestReady(true);

  code.assign(READY_CODE);
  message = code + '\n' + token + '\n';
  send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleStartGameSignal(int client_socket, std::stringstream& ss, int& roomIndex){
  std::string token, code, message;

  if (roomIndex < 0){
    std::cout << "Player not in a room but send start game request. Ignoring...\n";
    return;
  }
  code.assign(START_GAME_CODE);
  int newInt = coin_flip();
  std::cout << "coin: " << newInt << '\n';
  std::string playerSide = newInt ? "white" : "black";
  message = code + '\n' + playerSide + '\n';
  roomList[roomIndex].setGameStarting(true);
  roomList[roomIndex].setOwnerSide(newInt);
  send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
}

void ServerSocket::handleMoveSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId){
  std::string token, code, message;

  int moveFrom, moveTo;
  std::getline(ss, token, '\n');
  std::cout << token << '\n';
  moveFrom = std::stoi(token);
  std::getline(ss, token, '\n');
  std::cout << token << '\n';
  moveTo = std::stoi(token);

  if(roomList[roomIndex].isOwner(threadId))
    roomList[roomIndex].setOwnerMove(moveFrom, moveTo);
  else
    roomList[roomIndex].setGuestMove(moveFrom, moveTo);
}

void ServerSocket::handleResignSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId){
  if (roomList[roomIndex].isGuest(threadId))
    roomList[roomIndex].setGuestResign(true);
  else
    roomList[roomIndex].setOwnerResign(true);
}

void ServerSocket::handleRoomStatus(int client_socket, int& roomIndex, std::thread::id& threadId){
  std::string token, code, message;

  if (roomList[roomIndex].getNewOwnerState())
  {
    roomList[roomIndex].setNewOwnerState(false);
    // Send onwer state to client
    code.assign(NEW_OWNER_CODE);
    message = code + '\n' + roomList[roomIndex].getOwnerName() + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }
  if (roomList[roomIndex].getNewGuestState())
  {
    roomList[roomIndex].setNewGuestState(false);
    // Send guest state to client
    code.assign(NEW_GUEST_CODE);
    message = code + '\n' + roomList[roomIndex].getGuestName() + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }
  if (roomList[roomIndex].getGameStarting() && roomList[roomIndex].isGuest(threadId)){
    roomList[roomIndex].setGameStarting(false);
    code.assign(START_GAME_CODE);
    message = code + '\n' + (roomList[roomIndex].getOwnerSide() ? "black" : "white") + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }

  if (roomList[roomIndex].availableOwnerMove() && roomList[roomIndex].isGuest(threadId)){
    code.assign(MOVE_CODE);
    message = code + '\n' + std::to_string(roomList[roomIndex].getOwnerMoveFrom()) + '\n' + std::to_string(roomList[roomIndex].getOwnerMoveTo()) + '\n';
    roomList[roomIndex].setOwnerMove(-1, -1);
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }
  if (roomList[roomIndex].availableGuestMove() && roomList[roomIndex].isOwner(threadId)){
    code.assign(MOVE_CODE);
    message = code + '\n' + std::to_string(roomList[roomIndex].getGuestMoveFrom()) + '\n' + std::to_string(roomList[roomIndex].getGuestMoveTo()) + '\n';
    roomList[roomIndex].setGuestMove(-1, -1);
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
  }
  if (roomList[roomIndex].getNewGuestReady() && roomList[roomIndex].isOwner(threadId)){
    code.assign(READY_CODE);
    message = code + '\n' + (roomList[roomIndex].getGuestReady() ? std::to_string(1) : std::to_string(0)) + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    roomList[roomIndex].setNewGuestReady(false);
  }
  if (roomList[roomIndex].getGuestResign() && roomList[roomIndex].isOwner(threadId)){
    code.assign(RESIGN_CODE);
    message = code + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    roomList[roomIndex].setGuestResign(false);
  }
  if (roomList[roomIndex].getOwnerResign() && roomList[roomIndex].isGuest(threadId)){
    code.assign(RESIGN_CODE);
    message = code + '\n';
    send(client_socket, message.c_str(), RECEIVE_BUFFER_SIZE, 0);
    roomList[roomIndex].setOwnerResign(false);
  }
}

void ServerSocket::initializeAccountList(){
  std::ifstream inputFile("../data.txt");

  if (!inputFile) {
    std::cout << "Error opening the file." << std::endl;
    exit(1);
  }

  std::string line;
  std::string username;
  std::string password;
  std::string name;
  std::string status;
  int lineCount = 0;
  while (std::getline(inputFile, line)) {
    switch (lineCount % 4) {
      case 0:
          username = line;
          break;
      case 1:
          password = line;
          break;
      case 2:
          name = line;
          break;
      case 3:
          status = line;
          accountList.emplace(username, Account(username, password, name, status));
          break;
    }
    lineCount++;
  }

}

std::string ServerSocket::gen_random(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  srand(time(0));
  for (int i = 0; i < len; ++i)
  {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

int ServerSocket::coin_flip(){
  srand(time(0));
  return rand() % 2;
}

