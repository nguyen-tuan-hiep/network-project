#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#include <winsock.h>
#include <sstream>
#include <fstream>
#include <thread>
#include "room.h"
#include "account.h"

class Account;

#define RECEIVE_BUFFER_SIZE 64
#define MAX_CLIENT 100
#define CREATE_ROOM_CODE "CREATE"
#define START_GAME_CODE "START"
#define JOIN_ROOM_CODE "JOIN"
#define LEAVE_ROOM_CODE "LEAVE"
#define NEW_GUEST_CODE "NEWGUEST"
#define NEW_OWNER_CODE "NEWOWNER"
#define MOVE_CODE "MOVE"
#define READY_CODE "READY"
#define RESIGN_CODE "RESIGN"
#define LOGIN_CODE "LOGIN"

class ServerSocket{
public:
  ServerSocket();
  void close();
  static void error(std::string message);
  static void error(std::string message, bool isThread);
  SOCKET getServerSocket() {return server_so;};
  void handleClient(int client_socket);

private:
  void initializeAccountList();
  void handleCreateRoomSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId);
  void handleJoinRoomSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId);
  void handleLeaveRoomSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId);
  void handleLoginSignal(int client_socket, std::stringstream& ss);
  void handleStartGameSignal(int client_socket, std::stringstream& ss, int& roomIndex);
  void handleMoveSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId);
  void handleReadySignal(int client_socket, std::stringstream& ss, int& roomIndex);
  void handleResignSignal(int client_socket, std::stringstream& ss, int& roomIndex, std::thread::id& threadId);
  void handleRoomStatus(int client_socket, int& roomIndex, std::thread::id& threadId);

  SOCKET server_so;
  Room roomList[MAX_CLIENT];
  std::map<std::string, Account> accountList;
  std::string gen_random(const int len);
  int coin_flip();
};

#endif