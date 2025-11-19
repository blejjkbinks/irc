#include "Client.hpp"

Client::Client(void)
	: _name(NULL), _nick(NULL), _fd(0)
{
	return;
}

Client::~Client(void)
{
	return;
}

std::string	Client::getName(void)
{
	return (this->_name);
}

std::string	Client::getNick(void)
{
	return (this->_nick);
}

int	Client::getFd(void)
{
	return (this->_fd);
}

void	Client::setName(std::string name)
{
	this->_name = name;
}

void	Client::setNick(std::string nick)
{
	this->_nick = nick;
}

void	Client::setFd(int fd)
{
	this->_fd = fd;
}