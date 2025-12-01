#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class Channel {
public:
  static const int MAX_CHANNELS = 5;

private:
  Client _clients[Client::MAX_CLIENTS];
  int _clients_n;
  std::string _name;
  std::string _topic;
  std::vector<std::string> _operators;
  std::vector<std::string> _invited;
  std::string _key;
  int _limit;
  bool _mode_i;
  bool _mode_t;
  bool _mode_k;
  bool _mode_l;

public:
  Channel(void);
  Channel(const Channel &other);
  Channel &operator=(const Channel &other);
  ~Channel(void);

  void addClient(Client client);
  void rmClient(Client client);
  void broadcast(std::string message, Client &sender);

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

  void addOperator(std::string nick);
  void removeOperator(std::string nick);
  bool isOperator(std::string nick);
  void addInvited(std::string nick);
  bool isInvited(std::string nick);
  void setKey(std::string key);
  std::string getKey(void);
  void setMode(char mode, bool value);
  bool getMode(char mode);
  bool isFull(void);
};

std::ostream &operator<<(std::ostream &o, Channel &c);

#endif