#ifndef SERVER_HPP
# define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

class Server
{
	private:
		int const	_listen_port;
		std::string const	_password;
		Client	_clients[MAX_CLIENTS];
		int		_clients_n;
		Channel	_channels[MAX_CHANNELS];
		int		_channels_n;

	public:
		Server(int port, std::string password);
		Server(void);
		~Server(void);

		const static int	MAX_CHANNELS = 5;

		void	start(void);
		void	addChannel(Channel c);
};

#endif