#include "clientSocket.h"
#include <sstream>

void ClientSocket::error(std::string message)
{
  std::cerr << message << ": " << WSAGetLastError() << "\n";
  exit(1);
  return;
}

void ClientSocket::close()
{
  closesocket(this->so);
  WSACleanup();
  return;
}


ClientSocket::ClientSocket(std::string server_ip, unsigned short server_port)
{
  struct sockaddr_in server;
  WSADATA wsa;

  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    error("Failed WSAStartup");

  std::cout << "Winsock Initialised.\n";

  if ((so = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    error("Error on created the socket");

  std::cout << "Socket created.\n";

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server.sin_port = htons(server_port);

  if (connect(so, (struct sockaddr *)&server, sizeof server) < 0)
    error("Failed to connect");

  std::cout << "Connected to " << server_ip << ":" << server_port << "\n";

  u_long mode = 1; // Enable non-blocking mode
  if (ioctlsocket(so, FIONBIO, &mode) != 0)
    error("Failed to set the socket to non-blocking mode");

  name.assign(gen_random(3)); // assign name here
  roomNotFound = false;
  roomFull = false;
  isOwner = false;
  isGuestReady = false;
}

void ClientSocket::sendLoginSignal()
{
  int bytes_received;
  std::string message;
  std::string code;
  char buffer[RECEIVE_BUFFER_SIZE];

  code.assign(LOGIN_CODE);
  message = code + '\n' + username + '\n' + password + '\n';
  if (send(so, message.c_str(), RECEIVE_BUFFER_SIZE, 0) == SOCKET_ERROR)
    error("Error sending message to server. Exiting\n");
}

void ClientSocket::handleLoginSignal(std::string reply, std::string name)
{
  loginStatus = reply;
  if (reply == "logged_in"){
    displayPtr->setMenu(Display::Menu::mainMenu);
    this->name = name;
  }
}