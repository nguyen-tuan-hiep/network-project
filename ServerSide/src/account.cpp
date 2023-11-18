#include "account.h"
// #include <mysql_driver.h>
// #include <mysql_connection.h>

// sql::mysql::MySQL_Driver *driver;
// sql::Connection *con;
// // connect to server
// driver = sql::mysql::get_mysql_driver_instance();
// con = driver->connect("tcp://127.0.0.1:3306", "username", "password");
// con->setSchema("database");

// // query
// sql::Statement *stmt;
// stmt = con->createStatement();
// stmt->execute("CREATE TABLE example (id INT, name VARCHAR(50))");
// delete stmt;

// // handle result
// sql::ResultSet *res;
// res = stmt->executeQuery("SELECT * FROM example");
// while (res->next()) {
//     std::cout << "ID: " << res->getInt("id") << ", Name: " << res->getString("name") << std::endl;
// }
// delete res;

// delete con;


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

Account::Status Account::attempSignup(std::string username, std::string password){
  

}

Account::Status Account::loginFailed(){
  failedAttemp++;
  if (failedAttemp >= max_failed_attemp){
    status = blocked;
    return blocked;
  }
  return wrong_password;
}

