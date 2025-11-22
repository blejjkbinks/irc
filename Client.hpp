#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>

class Server;

class Client
{
private:
	std::string _name;
	std::string _nick;
	int _fd;
	bool _is_authenticated;

	void _join(std::string line, Server &server);
	void _part(std::string line, Server &server);
	void _kick(std::string line, Server &server);
	void _invite(std::string line, Server &server);
	void _topic(std::string line, Server &server);
	void _mode(std::string line, Server &server);
	void _auth(std::string line, Server &server);
	void _help(std::string line, Server &server);
	void _check_password(std::string line, Server &server);

public:
	Client(void);
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client(void);

	std::string buffer;
	const static int MAX_CLIENTS = 10;

	std::string getName(void);
	std::string getNick(void);
	int getFd(void);

	void setName(std::string name);
	void setNick(std::string nick);
	void setFd(int fd);
	bool isAuthenticated(void);
	void setAuthenticated(bool v);

	void process_line(std::string line, Server &server);
};

std::ostream &operator<<(std::ostream &o, Client &c);

#endif