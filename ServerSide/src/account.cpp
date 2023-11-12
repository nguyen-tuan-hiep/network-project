#include "account.h"

Account::Account(){
}

Account::Account(std::string username, std::string password, std::string name, std::string status){
  this->username.assign(username);
  this->password.assign(password);
  this->name.assign(name);
  
  if (status == "logged_out")
    this->status = logged_out;
  else if (status == "logged_in")
    this->status = logged_in;
  else if (status == "blocked")
    this->status = blocked;
  failedAttemp = 0;
}

Account::Status Account::attempLogin(std::string username, std::string password){
  if (!checkUsername(username))
    return logged_out;
  
  if(!checkPassword(password)){
    return loginFailed();
  }

  if (status == blocked)
    return blocked;

  status = logged_in;
  return status;
}

Account::Status Account::loginFailed(){
  failedAttemp++;
  if (failedAttemp >= max_failed_attemp){
    status = blocked;
    return blocked;
  }
  return wrong_password;
}

