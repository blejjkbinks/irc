#include "Server.hpp"

Server::Server(int port, std::string password)
	: _listen_port(port), _password(password), _channels_n(0), _clients_n(0)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
		_clients[i].setFd(-1);
}

Server::~Server(void)
{
	return;
}

void	Server::addChannel(Channel c)
{
	this->_channels[this->_channels_n] = c;
	this->_channels_n++;
}

void	Server::start(void)
{
	std::cout << "oe" << std::endl;
}
