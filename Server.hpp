#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Server {
public:
private:
  int _listen_port;
  std::string _password;
  Client _clients[Client::MAX_CLIENTS];
  int _clients_n;
  Channel _channels[Channel::MAX_CHANNELS];
  int _channels_n;

  int _make_listen_socket(int port);
  void _accept_new_clients(int listen_fd, pollfd *pfds);
  void _handle_client_io(pollfd *pfds, int i);
  void _compact_fds(pollfd *pfds);
  void _removeClientFromChannels(int fd);
  void _handle_stdin(void);

public:
  Server(int port, std::string password);
  Server(void);
  Server(const Server &other);
  Server &operator=(const Server &other);
  ~Server(void);

  void start(void);
  bool addChannel(Channel c);
  void removeChannel(std::string name);
  Channel *getChannel(std::string name);
  std::string getPassword(void);
};

#endif