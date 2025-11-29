#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <iostream>
#include <string>

class Channel {
public:
  static const int MAX_CHANNELS = 5;

private:
  Client _clients[Client::MAX_CLIENTS];
  int _clients_n;
  std::string _name;
  std::string _topic;
  std::string _password;
  bool _invite_only;
  int _user_limit;

public:
  Channel(void);
  Channel(const Channel &other);
  Channel &operator=(const Channel &other);
  ~Channel(void);

  void addClient(Client c);
  void rmClient(Client c);

  std::string getName(void);
  std::string getTopic(void);
  std::string getPassword(void);
  bool getInviteOnly(void);
  int getUserLimit(void);

  void setName(std::string name);
  void setTopic(std::string topic);
  void setPassword(std::string password);
  void setInviteOnly(bool v);
  void setUserLimit(int limit);
  int getClientsNumber(void);
  Client *getClient(int i);
};

std::ostream &operator<<(std::ostream &o, Channel &c);

#endif