#include "Channel.hpp"

Channel::Channel(void)
	: _clients_n(0)
{
	return;
}

Channel::~Channel(void)
{
	return;
}

void	Channel::addClient(Client c)
{
	this->_clients[this->_clients_n] = c;
	this->_clients_n++;
}

void	Channel::rmClient(Client c)
{
	int i = 0;
	while (i < MAX_CLIENTS)
	{
		if (this->_clients[i].getName() == c.getName())	//name unique???
		{
			std::cout << "removing client at index " << i << "in channel " << this->_name << std::endl;
		}
	}

}