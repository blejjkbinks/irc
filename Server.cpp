#include "Server.hpp"

static volatile bool g_running = true;

static void handle_sigint(int) { g_running = false; }

Server::Server(int port, std::string password) : _listen_port(port), _password(password), _server_name("ircserv"), _clients_n(0), _channels_n(0)
{
	for (int i = 0; i < Client::MAX_CLIENTS; i++)
		_clients[i].setFd(-1);
	for (int i = 0; i < Client::MAX_CLIENTS + 2; i++)
		_pfds[i].fd = -1;
}

Server::Server(void) : _listen_port(0), _password(""), _server_name("ircserv"), _clients_n(0), _channels_n(0)
{}

Server::Server(const Server &other) { *this = other; }

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		_listen_port = other._listen_port;
		_password = other._password;
		_server_name = other._server_name;
		_clients_n = other._clients_n;
		for (int i = 0; i < Client::MAX_CLIENTS; i++)
			_clients[i] = other._clients[i];
		_channels_n = other._channels_n;
		for (int i = 0; i < Channel::MAX_CHANNELS; i++)
			_channels[i] = other._channels[i];
	}
	return (*this);
}

Server::~Server(void)
{}

bool Server::addChannel(Channel c)
{
	if (this->_channels_n >= Channel::MAX_CHANNELS)
		return (false);
	this->_channels[this->_channels_n] = c;
	this->_channels_n++;
	return (true);
}

Channel *Server::getChannel(std::string name) {
	for (int i = 0; i < this->_channels_n; i++)
	{
		if (this->_channels[i].getName() == name)
			return (&this->_channels[i]);
	}
	return (NULL);
}

void Server::removeChannel(std::string name)
{
	for (int i = 0; i < this->_channels_n; i++)
	{
		if (this->_channels[i].getName() == name)
		{
			for (int j = i; j < this->_channels_n - 1; j++)
				this->_channels[j] = this->_channels[j + 1];
			this->_channels_n--;
			return;
		}
	}
}

int Server::_makeListenSocket(int port)
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		return (-1);
	int yes = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	//int flags = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, O_NONBLOCK);

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(listen_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
	{
		close(listen_fd);
		return (-1);
	}
	if (listen(listen_fd, 16) < 0)
	{
		close(listen_fd);
		return (-1);
	}
	return (listen_fd);
}

void Server::_acceptNewClients(int listen_fd)
{
	for (;;)
	{
		sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &len);
		if (client_fd < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			std::cerr << "accept error: " << std::strerror(errno) << std::endl;
			break;
		}
		//int flags = fcntl(client_fd, F_GETFL, 0);
		fcntl(client_fd, F_SETFL, O_NONBLOCK);

		if (_clients_n < Client::MAX_CLIENTS)
		{
			_pfds[_clients_n + 2].fd = client_fd;
			_pfds[_clients_n + 2].events = POLLIN;
			_pfds[_clients_n + 2].revents = 0;
			_clients[_clients_n].reset();
			_clients[_clients_n].setFd(client_fd);
			_clients[_clients_n].buffer = "";
			_clients[_clients_n].setIndex(_clients_n + 1);
			std::cout << _clients_n + 1 << " connected" << std::endl;
			_clients_n++;
		}
		else
		{
			std::cerr << "max_client limit reached" << std::endl;
			close(client_fd);
		}
	}
}

std::string Server::getPassword(void) { return (this->_password); }

std::string Server::getServerPrefix(void) { return (":" + _server_name); }

Client *Server::getClientByNick(std::string nick)
{
	for (int i = 0; i < _clients_n; i++)
	{
		if (_clients[i].getNick() == nick)
			return (&_clients[i]);
	}
	return (NULL);
}

void Server::_handleClientIO(int i)
{
	int fd = _pfds[i].fd;
	if (fd < 0)
		return;

	if (_pfds[i].revents & (POLLIN | POLLERR | POLLHUP))
	{
		char buf[4000];
		ssize_t n = recv(fd, buf, sizeof(buf), 0);
		if (n > 0)
		{
			_clients[i - 2].buffer.append(buf, n);
			std::string &b = _clients[i - 2].buffer;
			size_t pos;
			while ((pos = b.find('\n')) != std::string::npos)
			{
				std::string line = b.substr(0, pos);
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				std::cout << i - 1 << "<-[" << line << "]" << std::endl;
				_clients[i - 2].processLine(line, *this, i - 1);
				b.erase(0, pos + 1);
			}
		}
		else
		{
			if (n == 0 || (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN))
			{
				std::cout << i - 1 << " disconnected" << std::endl;
				_removeClientFromChannels(fd);
				close(fd);
				_pfds[i].fd = -1;
				_clients[i - 2].setFd(-1);
			}
		}
	}
}

void Server::_compactFds()
{
	int new_nfds = 2;
	int current_nfds = _clients_n + 2;
	for (int i = 2; i < current_nfds; ++i)
	{
		if (_pfds[i].fd != -1)
		{
			if (new_nfds != i)
			{
				_pfds[new_nfds] = _pfds[i];
				_clients[new_nfds - 2] = _clients[i - 2];
				_clients[new_nfds - 2].setIndex(new_nfds - 1);
			}
			new_nfds++;
		}
	}
	_clients_n = new_nfds - 2;
}

void Server::_removeClientFromChannels(int fd)
{
	for (int i = 0; i < _channels_n; i++)
	{
		Client c;
		c.setFd(fd);
		_channels[i].rmClient(c);
		if (_channels[i].getClientsNumber() == 0)
		{
			removeChannel(_channels[i].getName());
			i--;
		}
	}
}

void Server::_handleStdin(void)
{
	std::string line;
	if (!std::getline(std::cin, line))
		return;

	if (line == "debug")
	{
		std::cout << "--- DEBUG INFO ---" << std::endl;
		std::cout << "Clients connected: " << _clients_n << std::endl;
		for (int i = 0; i < _clients_n; ++i)
		{
			if (_clients[i].getFd() != -1)
				std::cout << "Client " << i << ": FD=" << _clients[i].getFd() << ", Nick=" << _clients[i].getNick() << ", Name=" << _clients[i].getName() << std::endl;
		}
		std::cout << "Channels: " << _channels_n << std::endl;
		for (int i = 0; i < _channels_n; ++i)
			std::cout << _channels[i] << std::endl;
		std::cout << "------------------" << std::endl;
	}
	else if (line == "quit")
		g_running = false;
	else
		std::cout << "Echo: " << line << std::endl;
}

void Server::disconnect(int fd, std::string error)
{
	for (int i = 2; i < _clients_n + 2; i++)
	{
		if (_pfds[i].fd == fd)
		{
			if (!error.empty())
			{
				std::string message = "ERROR :" + error + "\r\n";
				send(fd, message.c_str(), message.size(), 0);
			}
			std::cout << i - 1 << " disconnected" << std::endl;
			_removeClientFromChannels(fd);
			close(fd);
			_pfds[i].fd = -1;
			_clients[i - 2].setFd(-1);
			return;
		}
	}
}

void Server::start(void)
{
	signal(SIGINT, handle_sigint);
	int listen_fd = _makeListenSocket(_listen_port);
	if (listen_fd == -1)
	{
		std::cerr << "error trying to create listening socket: "
							<< std::strerror(errno) << std::endl;
		return;
	}

	_pfds[0].fd = listen_fd;
	_pfds[0].events = POLLIN;
	_pfds[1].fd = STDIN_FILENO;
	_pfds[1].events = POLLIN;
	std::cout << "server started" << std::endl;

	while (g_running)
	{
		if (poll(_pfds, _clients_n + 2, -1) < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "poll error: " << std::strerror(errno) << std::endl;
			break;
		}

		if (_pfds[0].revents & POLLIN)
			_acceptNewClients(listen_fd);
		if (_pfds[1].revents & POLLIN)
			_handleStdin();
		for (int i = 2; i < _clients_n + 2; ++i)
			_handleClientIO(i);
		_compactFds();
	}

	std::cout << "server shutting down" << std::endl;
	for (int i = 2; i < _clients_n + 2; ++i)
	{
		if (_pfds[i].fd != -1)
		{
			std::string message = "server shutting down\r\n";
			_clients[i - 2].ft_send(message);
			close(_pfds[i].fd);
		}
	}
	close(listen_fd);
}
