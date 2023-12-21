#include "room.h"

Room::Room(){
  isOccupied = false;
}

Room::~Room(){
}

void Room::setOwner(std::thread::id owner, std::string ownerName){
  this->owner = owner; 
  this->ownerName = ownerName; 
}

void Room::setGuest(std::thread::id guest, std::string guestName){
  this->guest = guest; 
  this->guestName = guestName; 
  newGuest = true; 
}

void Room::removeOwner(){
  if (guest != std::thread::id()){
    owner = guest;
    ownerName.assign(guestName);
    guest = std::thread::id();
    guestName.assign("");
  }else{
    owner = std::thread::id();
    ownerName.assign("");
    isOccupied = false;
    roomCode = std::string();
  }
  newOwner = true;
}

void Room::removeGuest(){
  guest = std::thread::id();
  guestName = "";
  newGuest = true;
}