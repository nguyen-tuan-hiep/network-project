#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <thread>

class Room{
public:
  Room();
  ~Room();

  //ACCESSOR
  std::string getRoomCode() { return roomCode; };
  bool isRoomOccupied() { return isOccupied; };
  bool isOwner(std::thread::id player) { return player == owner; };
  bool isGuest(std::thread::id player) { return player == guest; };
  bool isRoomFull() { return guest != std::thread::id(); };
  bool getNewGuestState() { return newGuest; };
  bool getNewOwnerState() { return newOwner; };
  bool getGameStarting() { return isGameStarting; };
  bool getOwnerSide() { return ownerSide; };
  std::string getOwnerName() { return ownerName; };
  std::string getGuestName() { return guestName; };
  int getOwnerMoveFrom() { return ownerMoveFrom; };
  int getOwnerMoveTo() { return ownerMoveTo; };
  int getGuestMoveFrom() { return guestMoveFrom; };
  int getGuestMoveTo() { return guestMoveTo; };
  bool availableGuestMove() { return guestMoveFrom != -1; };
  bool availableOwnerMove() { return ownerMoveFrom != -1; };
  bool getGuestReady() { return isGuestReady; };
  bool getNewGuestReady() { return newGuestReady; };
  bool getOwnerResign() { return ownerResign; };
  bool getGuestResign() { return guestResign; };

  //MUTATOR
  void setRoomOccupied(bool b) { isOccupied = b; };
  void setRoomCode(std::string roomCode) { this->roomCode = roomCode; };
  void setOwner(std::thread::id owner, std::string ownerName);
  void setGuest(std::thread::id guest, std::string guestName);
  void setNewGuestState(bool state) { newGuest = state; };
  void setNewOwnerState(bool state) { newOwner = state; };
  void setGameStarting(bool state) { isGameStarting = state; };
  void setOwnerSide(bool side) { ownerSide = side; };
  void setGuestMove(int moveFrom, int moveTo) { guestMoveFrom = moveFrom; guestMoveTo = moveTo; };
  void setOwnerMove(int moveFrom, int moveTo) { ownerMoveFrom = moveFrom; ownerMoveTo = moveTo; };
  void removeOwner();
  void removeGuest();
  void setGuestReady(bool isGuestReady) { this->isGuestReady = isGuestReady; };
  void setNewGuestReady(bool newGuestReady) { this->newGuestReady = newGuestReady; };
  void setGuestResign(bool guestResign) { this->guestResign = guestResign; };
  void setOwnerResign(bool ownerResign) { this->ownerResign = ownerResign; };
private:
  std::string roomCode;
  std::thread::id owner;
  std::string ownerName = "";
  std::thread::id guest;
  std::string guestName = "";
  int ownerMoveFrom = -1;
  int ownerMoveTo = -1;
  int guestMoveFrom = -1;
  int guestMoveTo = -1;
  bool ownerSide;
  bool isOccupied;
  bool isGameStarting;
  bool isGuestReady;
  bool ownerResign;
  bool guestResign;
  bool newGuest, newOwner, newGuestReady;
};

#endif