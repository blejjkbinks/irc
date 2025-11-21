#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include <sys/socket.h>

Client::Client(void) : _name(""), _nick(""), _fd(0), _is_authenticated(false) {
	return;
}

Client::Client(const Client &other) { *this = other; }

Client &Client::operator=(const Client &other) {
	if (this != &other) {
		_name = other._name;
		_nick = other._nick;
		_fd = other._fd;
		buffer = other.buffer;
	}
	return *this;
}

Client::~Client(void) { return; }

std::string Client::getName(void) { return (this->_name); }

std::string Client::getNick(void) { return (this->_nick); }

int Client::getFd(void) { return (this->_fd); }

void Client::setName(std::string name) { this->_name = name; }

void Client::setNick(std::string nick) { this->_nick = nick; }

void Client::setFd(int fd) { _fd = fd; }

bool Client::isAuthenticated(void) { return _is_authenticated; }

void Client::setAuthenticated(bool v) { _is_authenticated = v; }

std::ostream &operator<<(std::ostream &o, Client &c) {
	o << "Client(name=" << c.getName() << ", nick=" << c.getNick()
		<< ", fd=" << c.getFd() << ")";
	return (o);
}

void Client::_check_password(std::string line, Server &server) {
	// Trim potential \r
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	if (line == server.getPassword()) {
		_is_authenticated = true;
		std::string reply = "Authenticated!\r\n";
		send(_fd, reply.c_str(), reply.size(), 0);
	} else {
		std::string reply = "Wrong password. Try again: ";
		send(_fd, reply.c_str(), reply.size(), 0);
	}
}

void Client::process_line(std::string line, Server &server) {
	if (!_is_authenticated) {
		_check_password(line, server);
		return;
	}

	size_t first_space = line.find(' ');
	std::string command =
			(first_space == std::string::npos) ? line : line.substr(0, first_space);

	if (command == "JOIN") {
		_join(line, server);
	} else if (command == "PART") {
		_part(line, server);
	} else if (command == "KICK") {
		_kick(line, server);
	} else if (command == "INVITE") {
		_invite(line, server);
	} else if (command == "TOPIC") {
		_topic(line, server);
	} else if (command == "MODE") {
		_mode(line, server);
	} else if (command == "AUTH") {
		_auth(line, server);
	} else if (command == "HELP") {
		_help(line, server);
	} else {
		std::string reply = "sending back [" + line + "]\r\n";
		send(_fd, reply.c_str(), reply.size(), 0);
	}
}

void Client::_help(std::string line, Server &server) {
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
	send(_fd, reply.c_str(), reply.size(), 0);
}

void Client::_join(std::string line, Server &server) {
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos) {
		std::string reply = "Error: JOIN requires a channel name\r\n";
		send(_fd, reply.c_str(), reply.size(), 0);
		return;
	}
	std::string channel_name = line.substr(first_space + 1);
	if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\r')
		channel_name.erase(channel_name.size() - 1);

	Channel *existing_channel = server.getChannel(channel_name);
	if (existing_channel) {
		existing_channel->addClient(*this);
		std::string reply = "Joined channel " + channel_name + "\r\n";
		send(_fd, reply.c_str(), reply.size(), 0);
	} else {
		Channel c;
		c.setName(channel_name);
		c.addClient(*this);
		if (server.addChannel(c)) {
			std::string reply = "Channel " + channel_name + " created\r\n";
			send(_fd, reply.c_str(), reply.size(), 0);
		} else {
			std::string reply = "Error: Max channels reached\r\n";
			send(_fd, reply.c_str(), reply.size(), 0);
		}
	}
}

void Client::_part(std::string line, Server &server) {
	size_t first_space = line.find(' ');
	if (first_space == std::string::npos) {
		std::string reply = "Error: PART requires a channel name\r\n";
		send(_fd, reply.c_str(), reply.size(), 0);
		return;
	}
	std::string params = line.substr(first_space + 1);
	size_t colon_pos = params.find(':');
	std::string channels_str =
			(colon_pos == std::string::npos) ? params : params.substr(0, colon_pos);
	std::string reason =
			(colon_pos == std::string::npos) ? "" : params.substr(colon_pos + 1);

	if (colon_pos != std::string::npos && !channels_str.empty() &&
			channels_str[channels_str.size() - 1] == ' ')
		channels_str.erase(channels_str.size() - 1);

	if (!reason.empty() && reason[reason.size() - 1] == '\r')
		reason.erase(reason.size() - 1);
	if (reason.empty() && !channels_str.empty() &&
			channels_str[channels_str.size() - 1] == '\r')
		channels_str.erase(channels_str.size() - 1);

	size_t pos = 0;
	while ((pos = channels_str.find(',')) != std::string::npos) {
		std::string channel_name = channels_str.substr(0, pos);
		Channel *c = server.getChannel(channel_name);
		if (c) {
			c->rmClient(*this);
			std::string reply = "You left channel " + channel_name + "\r\n";
			send(_fd, reply.c_str(), reply.size(), 0);
			if (c->getClientsNumber() == 0) {
				server.removeChannel(channel_name);
				std::cout << "Channel " << channel_name << " removed" << std::endl;
			}
		} else {
			std::string reply = "Error: No such channel " + channel_name + "\r\n";
			send(_fd, reply.c_str(), reply.size(), 0);
		}
		channels_str.erase(0, pos + 1);
	}
	if (!channels_str.empty()) {
		std::string channel_name = channels_str;
		Channel *c = server.getChannel(channel_name);
		if (c) {
			c->rmClient(*this);
			std::string reply = "You left channel " + channel_name + "\r\n";
			send(_fd, reply.c_str(), reply.size(), 0);
			if (c->getClientsNumber() == 0) {
				server.removeChannel(channel_name);
				std::cout << "Channel " << channel_name << " removed" << std::endl;
			}
		} else {
			std::string reply = "Error: No such channel " + channel_name + "\r\n";
			send(_fd, reply.c_str(), reply.size(), 0);
		}
	}
}

void Client::_kick(std::string line, Server &server) {
	(void)line;
	(void)server;
	std::string reply = "Command KICK not implemented yet\r\n";
	send(_fd, reply.c_str(), reply.size(), 0);
}

void Client::_invite(std::string line, Server &server) {
	(void)line;
	(void)server;
	std::string reply = "Command INVITE not implemented yet\r\n";
	send(_fd, reply.c_str(), reply.size(), 0);
}

void Client::_topic(std::string line, Server &server) {
	(void)line;
	(void)server;
	std::string reply = "Command TOPIC not implemented yet\r\n";
	send(_fd, reply.c_str(), reply.size(), 0);
}

void Client::_mode(std::string line, Server &server) {
	(void)line;
	(void)server;
	std::string reply = "Command MODE not implemented yet\r\n";
	send(_fd, reply.c_str(), reply.size(), 0);
}

void Client::_auth(std::string line, Server &server) {
	(void)line;
	(void)server;
	std::string reply = "Command AUTH not implemented yet\r\n";
	send(_fd, reply.c_str(), reply.size(), 0);
}