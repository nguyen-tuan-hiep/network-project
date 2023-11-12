#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

class Account{
public:
  enum Status {logged_out, logged_in, blocked, wrong_password};
  Account();
  Account(std::string username, std::string password, std::string name, std::string status);

  //ACCESSOR
  std::string getName() { return name; };

  Status attempLogin(std::string username, std::string password);
  static std::string Stringify(Status status){
    switch (status)
    {
    case logged_out:
      return "logged_out";
      break;
    case logged_in:
      return "logged_in";
      break;
    case blocked:
      return "blocked";
      break;
    case wrong_password:
      return "wrong_password";
      break;
    default:
      return "";
      break;
    }
  }
private:
  bool checkUsername(std::string username) { return this->username == username; };
  bool checkPassword(std::string password) { return this->password == password; };
  Status loginFailed();
  static const int max_failed_attemp = 3;
  std::string username;
  std::string password;
  std::string name;
  Status status;
  int failedAttemp = 0;
};

#endif