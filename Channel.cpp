#include "Channel.hpp"

Channel::Channel(void) : _clients_n(0)
{}

Channel::Channel(const Channel &other)
{
	*this = other;
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		_clients_n = other._clients_n;
		for (int i = 0; i < Client::MAX_CLIENTS; i++)
			_clients[i] = other._clients[i];
		_name = other._name;
		_topic = other._topic;
		_password = other._password;
		_invite_only = other._invite_only;
		_user_limit = other._user_limit;
	}
	return *this;
}

Channel::~Channel(void)
{}

std::string Channel::getName(void) { return _name; }

std::string Channel::getTopic(void) { return _topic; }

std::string Channel::getPassword(void) { return _password; }

bool Channel::getInviteOnly(void) { return _invite_only; }

int Channel::getUserLimit(void) { return _user_limit; }

void Channel::setName(std::string name) { _name = name; }

void Channel::setTopic(std::string topic) { _topic = topic; }

void Channel::setPassword(std::string password) { _password = password; }

void Channel::setInviteOnly(bool v) { _invite_only = v; }

void Channel::setUserLimit(int limit) { _user_limit = limit; }

void Channel::addClient(Client c)
{
	this->_clients[this->_clients_n] = c;
	this->_clients_n++;
}

void Channel::rmClient(Client c)
{
	for (int i = 0; i < _clients_n; i++)
	{
		if (_clients[i].getFd() == c.getFd())
		{
			for (int j = i; j < _clients_n - 1; j++)
				_clients[j] = _clients[j + 1];
			_clients_n--;
			std::cout << "Removed client " << c.getName() << " from channel " << _name << std::endl;
			return;
		}
	}
}

int Channel::getClientsNumber(void) { return this->_clients_n; }

Client *Channel::getClient(int i)
{
	if (i >= 0 && i < _clients_n)
		return &_clients[i];
	return NULL;
}

std::ostream &operator<<(std::ostream &o, Channel &c)
{
	o << "Channel(name=" << c.getName() << ", topic=" << c.getTopic() << ", clients=[";
	for (int i = 0; i < c.getClientsNumber(); i++)
	{
		Client *cl = c.getClient(i);
		if (cl)
		{
			o << cl->getName();
			if (i < c.getClientsNumber() - 1)
				o << " ";
		}
	}
	o << "])";
	return (o);
}