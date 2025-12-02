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

class Server
{
private:
	int _listen_port;
	std::string _password;
	std::string _server_name;
	Client _clients[Client::MAX_CLIENTS];
	int _clients_n;
	Channel _channels[Channel::MAX_CHANNELS];
	int _channels_n;

	int _makeListenSocket(int port);
	void _acceptNewClients(int listen_fd);
	void _handleClientIO(int i);
	void _compactFds();
	void _removeClientFromChannels(int fd);
	void _handleStdin(void);

	pollfd _pfds[Client::MAX_CLIENTS + 2];

public:
	Server(int port, std::string password);
	Server(void);
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server(void);

	void start(void);
	void disconnect(int fd, std::string error);
	bool addChannel(Channel c);
	void removeChannel(std::string name);
	Channel *getChannel(std::string name);
	std::string getPassword(void);
	std::string getServerPrefix(void);
	Client *getClientByNick(std::string nick);
};

#endif