#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include <sys/socket.h>

Client::Client(void) : _name(""), _nick(""), _fd(0), _is_authenticated(false), _user(""),
	_is_registered(false), _waiting_for_cap_end(false), _index(0)
{}

Client::Client(const Client &other) { *this = other; }

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		_name = other._name;
		_nick = other._nick;
		_fd = other._fd;
		buffer = other.buffer;
		_is_authenticated = other._is_authenticated;
		_user = other._user;
		_is_registered = other._is_registered;
		_waiting_for_cap_end = other._waiting_for_cap_end;
		_index = other._index;
	}
	return (*this);
}

Client::~Client(void) {}

std::string Client::getName(void) { return (this->_name); }

std::string Client::getNick(void) { return (this->_nick); }

int Client::getFd(void) { return (this->_fd); }

void Client::setName(std::string name) { this->_name = name; }

void Client::setNick(std::string nick) { this->_nick = nick; }

void Client::setFd(int fd) { _fd = fd; }

void Client::setIndex(int index) { _index = index; }

int Client::getIndex(void) { return (_index); }

void Client::ft_send(std::string message)
{
	std::string log_message = message;
	if (!log_message.empty() && log_message[log_message.size() - 1] == '\n')
		log_message.erase(log_message.size() - 1);
	if (!log_message.empty() && log_message[log_message.size() - 1] == '\r')
		log_message.erase(log_message.size() - 1);
	std::cout << _index << "->[" << log_message << "]" << std::endl;
	send(_fd, message.c_str(), message.size(), 0);
}

bool Client::isAuthenticated(void) { return (_is_authenticated); }

void Client::setAuthenticated(bool v) { _is_authenticated = v; }

void Client::reset(void)
{
	_name = "";
	_nick = "";
	_fd = -1;
	_is_authenticated = false;
	_user = "";
	_is_registered = false;
	_waiting_for_cap_end = false;
	buffer = "";
	_index = 0;
}

std::ostream &operator<<(std::ostream &o, Client &c)
{
	o << "Client(name=" << c.getName() << ", nick=" << c.getNick() << ", fd=" << c.getFd() << ")";
	return (o);
}

void Client::_pass(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		std::string reply = "Error: PASS requires a password\r\n";
		ft_send(reply);
		return;
	}
	std::string password = line.substr(first_space + 1);
	if (!password.empty() && password[password.size() - 1] == '\r')
		password.erase(password.size() - 1);

	if (password == server.getPassword())
		_is_authenticated = true;
	else
		server.disconnect(getFd(), "Password incorrect");
}

void Client::_nickCmd(std::string line, Server &server)
{
	(void)server;
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		std::string reply = "Error: NICK requires a nickname\r\n";
		ft_send(reply);
		return;
	}
	std::string nick = line.substr(first_space + 1);
	if (!nick.empty() && nick[nick.size() - 1] == '\r')
		nick.erase(nick.size() - 1);
	_nick = nick;
	_welcome(server);
}

void Client::_userCmd(std::string line, Server &server)
{
	(void)server;
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		std::string reply = "Error: USER requires a username\r\n";
		ft_send(reply);
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t second_space = params.find(' ');
	if (second_space == std::string::npos)
		_user = params;
	else
		_user = params.substr(0, second_space);
	if (!_user.empty() && _user[_user.size() - 1] == '\r')
		_user.erase(_user.size() - 1);

	size_t colon_pos = line.find(':');
	if (colon_pos != std::string::npos)
	{
		std::string real_name = line.substr(colon_pos + 1);
		if (!real_name.empty() && real_name[real_name.size() - 1] == '\r')
			real_name.erase(real_name.size() - 1);
		_name = real_name;
	}
	_welcome(server);
}

void Client::_cap(std::string line, Server &server)
{
	(void)server;
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
		return;
	std::string subcommand = line.substr(first_space + 1);
	size_t second_space = subcommand.find(' ');
	if (second_space != std::string::npos)
		subcommand = subcommand.substr(0, second_space);

	if (!subcommand.empty() && subcommand[subcommand.size() - 1] == '\r')
		subcommand.erase(subcommand.size() - 1);

	if (subcommand == "LS")
	{
		_waiting_for_cap_end = true;
		std::string reply = "CAP * LS :\r\n";
		ft_send(reply);
	}
	else if (subcommand == "REQ")
	{
		std::string reply = "CAP * NAK :\r\n";
		ft_send(reply);
	}
	else if (subcommand == "END")
	{
		_waiting_for_cap_end = false;
		_welcome(server);
	}
}

void Client::_privmsg(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		ft_send("411 :No recipient given (PRIVMSG)\r\n");
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t second_space = params.find(' ');
	if (second_space == std::string::npos)
	{
		ft_send("412 :No text to send\r\n");
		return;
	}
	std::string target = params.substr(0, second_space);
	std::string message = params.substr(second_space + 1);

	if (message.size() > 0 && message[0] == ':')
		message = message.substr(1);
	if (!message.empty() && message[message.size() - 1] == '\r')
		message.erase(message.size() - 1);

	std::string full_message = ":" + _nick + "!" + _user + "@localhost PRIVMSG " + target + " :" + message + "\r\n";

	if (target[0] == '#')
	{
		Channel *channel = server.getChannel(target);
		if (channel)
			channel->broadcast(full_message, *this);
		else
			ft_send("401 " + target + " :No such nick/channel\r\n");
	}
	else
	{
		Client *recipient = server.getClientByNick(target);
		if (recipient)
			recipient->ft_send(full_message);
		else
			ft_send("401 " + target + " :No such nick/channel\r\n");
	}
}

void Client::_quit(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	std::string message = "";
	if (first_space != std::string::npos)
	{
		message = line.substr(first_space + 1);
		if (!message.empty() && message[0] == ':')
			message = message.substr(1);
		if (!message.empty() && message[message.size() - 1] == '\r')
			message.erase(message.size() - 1);
	}
	server.disconnect(getFd(), message);
}

void Client::_ping(std::string line, Server &server)
{
	(void)server;
	size_t first_space = line.find(' ');
	std::string token = "";
	if (first_space != std::string::npos)
		token = line.substr(first_space + 1);
	if (!token.empty() && token[token.size() - 1] == '\r')
		token.erase(token.size() - 1);

	std::string reply = "PONG " + server.getServerPrefix().substr(1) + " " + token + "\r\n";
	ft_send(reply);
}

void Client::_welcome(Server &server)
{
	(void)server;
	if (!_nick.empty() && !_user.empty() && !_is_registered && !_waiting_for_cap_end)
	{
		_is_registered = true;
		std::string reply;
		reply = server.getServerPrefix() + " 001 " + _nick + " :Welcome to the Internet Relay Network " + _nick + "!" + _user + "@localhost\r\n";
		ft_send(reply);
		reply = server.getServerPrefix() + " 002 " + _nick + " :Your host is ircserv, running version 1.0\r\n";
		ft_send(reply);
		reply = server.getServerPrefix() + " 003 " + _nick + " :This server was created today\r\n";
		ft_send(reply);
		reply = server.getServerPrefix() + " 004 " + _nick + " ircserv 1.0 - -\r\n";
		ft_send(reply);
	}
}

void Client::processLine(std::string line, Server &server, int index)
{
	(void)index;
	if (line == "JOIN :")
		return;
	size_t first_space = line.find(' ');
	std::string command = (first_space == std::string::npos) ? line : line.substr(0, first_space);
	if (command == "CAP")
	{
		_cap(line, server);
		return;
	}
	if (!server.getPassword().empty())
	{
		if (!_is_authenticated)
		{
			if (command == "PASS")
			{
				_pass(line, server);
				return;
			}
			server.disconnect(getFd(), "Password required");
			return;
		}
		else if (command == "PASS")
		{
			ft_send("Error: You are already authenticated\r\n");
			return;
		}
	}
	else if (command == "PASS")
		return;

	if (command == "NICK")
	{
		_nickCmd(line, server);
		return;
	}

	if (command == "USER")
	{
		_userCmd(line, server);
		return;
	}

	if (!_is_registered)
	{
		std::string reply = "451 " + (command.empty() ? "*" : command) + " :You have not registered\r\n";
		ft_send(reply);
		return;
	}

	if (command == "JOIN")
		_join(line, server);
	else if (command == "PART")
		_part(line, server);
	else if (command == "KICK")
		_kick(line, server);
	else if (command == "INVITE")
		_invite(line, server);
	else if (command == "TOPIC")
		_topic(line, server);
	else if (command == "MODE")
		_mode(line, server);
	else if (command == "AUTH")
		_auth(line, server);
	else if (command == "PRIVMSG")
		_privmsg(line, server);
	else if (command == "QUIT")
		_quit(line, server);
	else if (command == "PING")
		_ping(line, server);
	else if (command == "MODE")
		_mode(line, server);
	else if (command == "KICK")
		_kick(line, server);
	else if (command == "INVITE")
		_invite(line, server);
	else if (command == "TOPIC")
		_topic(line, server);
	else if (command == "HELP")
		_help(line, server);
	else
	{
		std::string reply = line + "\r\n";
		ft_send(reply);
	}
}

void Client::_help(std::string line, Server &server)
{
	(void)line;
	(void)server;
	std::string reply = "Available commands:\r\n"
											"JOIN <channel>\r\n"
											"PART <channel>\r\n"
											"KICK <channel> <user> (placeholder)\r\n"
											"INVITE <user> <channel> (placeholder)\r\n"
											"TOPIC <channel> [<topic>] (placeholder)\r\n"
											"MODE <channel> <mode> (placeholder)\r\n"
											"AUTH <password> (placeholder)\r\n"
											"HELP\r\n";
	ft_send(reply);
}

void Client::_join(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		ft_send("461 JOIN :Not enough parameters\r\n");
		return;
	}
	std::string channel_name = line.substr(first_space + 1);
	if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\r')
		channel_name.erase(channel_name.size() - 1);

	if (channel_name[0] != '#')
	{
		ft_send("403 " + channel_name + " :No such channel\r\n");
		return;
	}

	Channel *channel = server.getChannel(channel_name);
	if (!channel)
	{
		Channel new_channel;
		new_channel.setName(channel_name);
		new_channel.addClient(*this);
		server.addChannel(new_channel);
		channel = server.getChannel(channel_name);
		ft_send(":" + _nick + "!" + _user + "@localhost JOIN " + channel_name + "\r\n");
		std::string topic = channel->getTopic();
		if (!topic.empty())
			ft_send("332 " + _nick + " " + channel_name + " :" + topic + "\r\n");
		else
			ft_send("331 " + _nick + " " + channel_name + " :No topic is set\r\n");
		// Send 353 (NAMES)
		std::string names_reply = "353 " + _nick + " = " + channel_name + " :";
		for (int i = 0; i < channel->getClientsNumber(); i++)
		{
			Client *c = channel->getClient(i);
			if (channel->isOperator(c->getNick()))
				names_reply += "@";
			names_reply += c->getNick() + " ";
		}
		names_reply += "\r\n";
		ft_send(names_reply);
		ft_send("366 " + _nick + " " + channel_name + " :End of /NAMES list\r\n");
		std::cout << "Channel " << channel_name << " created" << std::endl;
	}
	else
	{
		if (channel->getInviteOnly() && !channel->isInvited(_nick))
		{
			ft_send("473 " + channel_name + " :Cannot join channel (+i)\r\n");
			return;
		}
		if (channel->getMode('k'))
		{
			std::string key = "";
			size_t second_space = line.find(' ', first_space + 1);
			if (second_space != std::string::npos)
				key = line.substr(second_space + 1);
			if (key != channel->getKey())
			{
				ft_send("475 " + channel_name + " :Cannot join channel (+k)\r\n");
				return;
			}
		}
		if (channel->isFull())
		{
			ft_send("471 " + channel_name + " :Cannot join channel (+l)\r\n");
			return;
		}
		channel->addClient(*this);
		std::string join_msg = ":" + _nick + "!" + _user + "@localhost JOIN " + channel_name + "\r\n";
		channel->broadcast(join_msg, *this);
		ft_send(join_msg);
		std::string topic = channel->getTopic();
		if (!topic.empty())
			ft_send("332 " + _nick + " " + channel_name + " :" + topic + "\r\n");
		else
			ft_send("331 " + _nick + " " + channel_name + " :No topic is set\r\n");
		// Send 353 (NAMES)
		std::string names_reply = "353 " + _nick + " = " + channel_name + " :";
		for (int i = 0; i < channel->getClientsNumber(); i++)
		{
			Client *c = channel->getClient(i);
			if (channel->isOperator(c->getNick()))
				names_reply += "@";
			names_reply += c->getNick() + " ";
		}
		names_reply += "\r\n";
		ft_send(names_reply);
		ft_send("366 " + _nick + " " + channel_name + " :End of /NAMES list\r\n");
	}
}

void Client::_kick(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		ft_send("461 KICK :Not enough parameters\r\n");
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t second_space = params.find(' ');
	if (second_space == std::string::npos)
	{
		ft_send("461 KICK :Not enough parameters\r\n");
		return;
	}
	std::string channel_name = params.substr(0, second_space);
	std::string user_reason = params.substr(second_space + 1);
	std::string target_nick = user_reason;
	std::string reason = "";
	size_t third_space = user_reason.find(' ');
	if (third_space != std::string::npos)
	{
		target_nick = user_reason.substr(0, third_space);
		reason = user_reason.substr(third_space + 1);
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
	}

	Channel *channel = server.getChannel(channel_name);
	if (!channel)
	{
		ft_send("403 " + channel_name + " :No such channel\r\n");
		return;
	}
	if (!channel->isOperator(_nick))
	{
		ft_send("482 " + channel_name + " :You're not channel operator\r\n");
		return;
	}
	Client *target = server.getClientByNick(target_nick);
	if (!target)
	{
		ft_send("401 " + target_nick + " :No such nick/channel\r\n");
		return;
	}

	std::string kick_msg = ":" + _nick + "!" + _user + "@localhost KICK " + channel_name + " " + target_nick + " :" + reason + "\r\n";
	channel->broadcast(kick_msg, *this);
	ft_send(kick_msg);
	target->ft_send(kick_msg);
	channel->rmClient(*target);
	if (channel->getClientsNumber() == 0)
		server.removeChannel(channel_name);
}

void Client::_invite(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		ft_send("461 INVITE :Not enough parameters\r\n");
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t second_space = params.find(' ');
	if (second_space == std::string::npos)
	{
		ft_send("461 INVITE :Not enough parameters\r\n");
		return;
	}
	std::string target_nick = params.substr(0, second_space);
	std::string channel_name = params.substr(second_space + 1);
	if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\r')
		channel_name.erase(channel_name.size() - 1);

	Channel *channel = server.getChannel(channel_name);
	if (!channel)
	{
		ft_send("403 " + channel_name + " :No such channel\r\n");
		return;
	}
	// Check if user is on channel
	bool on_channel = false;
	for (int i = 0; i < channel->getClientsNumber(); i++)
	{
		if (channel->getClient(i)->getFd() == _fd)
		{
			on_channel = true;
			break;
		}
	}
	if (!on_channel)
	{
		ft_send("442 " + channel_name + " :You're not on that channel\r\n");
		return;
	}
	if (channel->getInviteOnly() && !channel->isOperator(_nick))
	{
		ft_send("482 " + channel_name + " :You're not channel operator\r\n");
		return;
	}
	Client *target = server.getClientByNick(target_nick);
	if (!target)
	{
		ft_send("401 " + target_nick + " :No such nick/channel\r\n");
		return;
	}
	channel->addInvited(target_nick);
	ft_send("341 " + _nick + " " + target_nick + " " + channel_name + "\r\n");
	target->ft_send(":" + _nick + "!" + _user + "@localhost INVITE " + target_nick + " " + channel_name + "\r\n");
}

void Client::_topic(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		ft_send("461 TOPIC :Not enough parameters\r\n");
		return;
	}
	std::string params = line.substr(first_space + 1);
	std::string channel_name = params;
	std::string topic = "";
	size_t second_space = params.find(' ');
	if (second_space != std::string::npos)
	{
		channel_name = params.substr(0, second_space);
		topic = params.substr(second_space + 1);
		if (!topic.empty() && topic[0] == ':')
			topic = topic.substr(1);
		if (!topic.empty() && topic[topic.size() - 1] == '\r')
			topic.erase(topic.size() - 1);
	} 
	else
	{
		if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\r')
			channel_name.erase(channel_name.size() - 1);
	}

	Channel *channel = server.getChannel(channel_name);
	if (!channel)
	{
		ft_send("403 " + channel_name + " :No such channel\r\n");
		return;
	}
	if (second_space == std::string::npos)
	{
		// View topic
		std::string current_topic = channel->getTopic();
		if (current_topic.empty())
			ft_send("331 " + _nick + " " + channel_name + " :No topic is set\r\n");
		else
			ft_send("332 " + _nick + " " + channel_name + " :" + current_topic + "\r\n");
	}
	else
	{
		// Set topic
		if (channel->getMode('t') && !channel->isOperator(_nick))
		{
			ft_send("482 " + channel_name + " :You're not channel operator\r\n");
			return;
		}
		channel->setTopic(topic);
		std::string topic_msg = ":" + _nick + "!" + _user + "@localhost TOPIC " + channel_name + " :" + topic + "\r\n";
		channel->broadcast(topic_msg, *this);
		ft_send(topic_msg);
	}
}

void Client::_mode(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		ft_send("461 MODE :Not enough parameters\r\n");
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t second_space = params.find(' ');
	std::string channel_name = params;
	std::string modes = "";
	std::string args = "";
	if (second_space != std::string::npos)
	{
		channel_name = params.substr(0, second_space);
		std::string rest = params.substr(second_space + 1);
		size_t third_space = rest.find(' ');
		if (third_space != std::string::npos)
		{
			modes = rest.substr(0, third_space);
			args = rest.substr(third_space + 1);
		}
		else
			modes = rest;
	}
	if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\r')
		channel_name.erase(channel_name.size() - 1);

	Channel *channel = server.getChannel(channel_name);
	if (!channel)
	{
		ft_send("403 " + channel_name + " :No such channel\r\n");
		return;
	}
	if (modes.empty())
	{
		// Show modes
		std::string mode_str = "+";
		if (channel->getMode('i'))
			mode_str += "i";
		if (channel->getMode('t'))
			mode_str += "t";
		if (channel->getMode('k'))
			mode_str += "k";
		if (channel->getMode('l'))
			mode_str += "l";
		ft_send("324 " + _nick + " " + channel_name + " " + mode_str + "\r\n");
		return;
	}
	if (!channel->isOperator(_nick))
	{
		ft_send("482 " + channel_name + " :You're not channel operator\r\n");
		return;
	}

	bool add = true;
	size_t arg_pos = 0;
	for (size_t i = 0; i < modes.size(); i++)
	{
		char c = modes[i];
		if (c == '+')
			add = true;
		else if (c == '-')
			add = false;
		else if (c == 'i')
			channel->setMode('i', add);
		else if (c == 't')
			channel->setMode('t', add);
		else if (c == 'k')
		{
			if (add)
			{
				std::string key = "";
				size_t next_space = args.find(' ', arg_pos);
				if (next_space == std::string::npos)
					key = args.substr(arg_pos);
				else
				{
					key = args.substr(arg_pos, next_space - arg_pos);
					arg_pos = next_space + 1;
				}
				if (!key.empty())
				{
					channel->setKey(key);
					channel->setMode('k', true);
				}
			}
			else
			{
				channel->setKey("");
				channel->setMode('k', false);
			}
		}
		else if (c == 'o')
		{
			std::string target = "";
			size_t next_space = args.find(' ', arg_pos);
			if (next_space == std::string::npos)
				target = args.substr(arg_pos);
			else
			{
				target = args.substr(arg_pos, next_space - arg_pos);
				arg_pos = next_space + 1;
			}
			if (!target.empty())
			{
				if (add)
					channel->addOperator(target);
				else
					channel->removeOperator(target);
			}
		}
		else if (c == 'l')
		{
			if (add)
			{
				std::string limit_str = "";
				size_t next_space = args.find(' ', arg_pos);
				if (next_space == std::string::npos)
					limit_str = args.substr(arg_pos);
				else
				{
					limit_str = args.substr(arg_pos, next_space - arg_pos);
					arg_pos = next_space + 1;
				}
				if (!limit_str.empty())
				{
					channel->setUserLimit(std::atoi(limit_str.c_str()));
					channel->setMode('l', true);
				}
			}
			else
			{
				channel->setUserLimit(0);
				channel->setMode('l', false);
			}
		}
	}
	// Broadcast mode change (simplified, ideally construct the actual change string)
	std::string mode_msg = ":" + _nick + "!" + _user + "@localhost MODE " + channel_name + " " + modes + " " + args + "\r\n";
	channel->broadcast(mode_msg, *this);
	ft_send(mode_msg);
}

void Client::_part(std::string line, Server &server)
{
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos)
	{
		std::string reply = "Error: PART requires a channel name\r\n";
		ft_send(reply);
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t colon_pos = params.find(':');
	std::string channels_str = (colon_pos == std::string::npos) ? params : params.substr(0, colon_pos);
	std::string reason = (colon_pos == std::string::npos) ? "" : params.substr(colon_pos + 1);

	if (colon_pos != std::string::npos && !channels_str.empty() && channels_str[channels_str.size() - 1] == ' ')
		channels_str.erase(channels_str.size() - 1);

	if (!reason.empty() && reason[reason.size() - 1] == '\r')
		reason.erase(reason.size() - 1);
	if (reason.empty() && !channels_str.empty() && channels_str[channels_str.size() - 1] == '\r')
		channels_str.erase(channels_str.size() - 1);

	size_t pos = 0;
	while ((pos = channels_str.find(',')) != std::string::npos)
	{
		std::string channel_name = channels_str.substr(0, pos);
		Channel *c = server.getChannel(channel_name);
		if (c)
		{
			std::string part_msg = ":" + _nick + "!" + _user + "@localhost PART " + channel_name + " :" + reason + "\r\n";
			c->broadcast(part_msg, *this);
			ft_send(part_msg);
			c->rmClient(*this);
			if (c->getClientsNumber() == 0)
			{
				server.removeChannel(channel_name);
				std::cout << "Channel " << channel_name << " removed" << std::endl;
			}
		}
		else
		{
			std::string reply = "Error: No such channel " + channel_name + "\r\n";
			ft_send(reply);
		}
		channels_str.erase(0, pos + 1);
	}
	if (!channels_str.empty())
	{
		std::string channel_name = channels_str;
		Channel *c = server.getChannel(channel_name);
		if (c)
		{
			std::string part_msg = ":" + _nick + "!" + _user + "@localhost PART " + channel_name + " :" + reason + "\r\n";
			c->broadcast(part_msg, *this);
			ft_send(part_msg);
			c->rmClient(*this);
			if (c->getClientsNumber() == 0)
			{
				server.removeChannel(channel_name);
				std::cout << "Channel " << channel_name << " removed" << std::endl;
			}
		} 
		else
		{
			std::string reply = "Error: No such channel " + channel_name + "\r\n";
			ft_send(reply);
		}
	}
}

void Client::_auth(std::string line, Server &server) {
	(void)line;
	(void)server;
	std::string reply = "Command AUTH not implemented yet\r\n";
	ft_send(reply);
}