#include <ctime>
#include <iostream>
#include "ServerSocket.h"

void handle_client(int client_socket);
std::string gen_random(const int len);
std::map<std::string, Account> initializeAccountList();
void displayAccountList(const std::map<std::string, Account>& accountList);

int main()
{
  std::thread threadArray[MAX_CLIENT];
  ServerSocket serverSock;
  struct sockaddr_in client;
  int client_length;
  SOCKET client_so;

  client_length = sizeof(client);
  while (client_so = accept(serverSock.getServerSocket(), (struct sockaddr *)&client, &client_length))
  {
    std::cout << "New connection accepted " << inet_ntoa(client.sin_addr) << "\n";
    for (int i = 0; i < MAX_CLIENT; i++)
    {
      if (!threadArray[i].joinable())
      {
        threadArray[i] = std::thread([&]() { serverSock.handleClient(client_so); });
        threadArray[i].detach();
        break;
      }
    }
  }

  serverSock.close();
  return 0;
}
