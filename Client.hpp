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
	std::string _user;
	bool _is_registered;
	bool _waiting_for_cap_end;

	void _join(std::string line, Server &server);
	void _part(std::string line, Server &server);
	void _kick(std::string line, Server &server);
	void _invite(std::string line, Server &server);
	void _topic(std::string line, Server &server);
	void _mode(std::string line, Server &server);
	void _auth(std::string line, Server &server);
	void _help(std::string line, Server &server);
	void _pass(std::string line, Server &server);
	void _nickCmd(std::string line, Server &server);
	void _userCmd(std::string line, Server &server);
	void _welcome(Server &server);
	void _cap(std::string line, Server &server);
	void _privmsg(std::string line, Server &server);
	void _quit(std::string line, Server &server);
	void _ping(std::string line, Server &server);

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

	int _index;
	void setIndex(int index);
	int getIndex(void);
	void ft_send(std::string message);

	void setName(std::string name);
	void setNick(std::string nick);
	void setFd(int fd);
	bool isAuthenticated(void);
	void setAuthenticated(bool v);
	void reset(void);

	void processLine(std::string line, Server &server, int index);
};

std::ostream &operator<<(std::ostream &o, Client &c);

#endif