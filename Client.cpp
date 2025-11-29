#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include <sys/socket.h>

Client::Client(void)
    : _name(""), _nick(""), _fd(0), _is_authenticated(false), _user(""),
      _is_registered(false), _waiting_for_cap_end(false), _index(0) {}

Client::Client(const Client &other) { *this = other; }

Client &Client::operator=(const Client &other) {
  if (this != &other) {
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

Client::~Client(void) { return; }

std::string Client::getName(void) { return (this->_name); }

std::string Client::getNick(void) { return (this->_nick); }

int Client::getFd(void) { return (this->_fd); }

void Client::setName(std::string name) { this->_name = name; }

void Client::setNick(std::string nick) { this->_nick = nick; }

void Client::setFd(int fd) { _fd = fd; }

void Client::setIndex(int index) { _index = index; }

int Client::getIndex(void) { return (_index); }

void Client::ft_send(std::string message) {
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

std::ostream &operator<<(std::ostream &o, Client &c) {
  o << "Client(name=" << c.getName() << ", nick=" << c.getNick()
    << ", fd=" << c.getFd() << ")";
  return (o);
}

void Client::_pass(std::string line, Server &server) {
  size_t first_space = line.find(' ');
  if (first_space == std::string::npos) {
    std::string reply = "Error: PASS requires a password\r\n";
    ft_send(reply);
    return;
  }
  std::string password = line.substr(first_space + 1);
  if (!password.empty() && password[password.size() - 1] == '\r')
    password.erase(password.size() - 1);

  if (password == server.getPassword()) {
    _is_authenticated = true;
    // Silent success as per standard, or send welcome if ready (unlikely here)
  } else {
    std::string reply = "Wrong password\r\n";
    ft_send(reply);
  }
}

void Client::_nick_cmd(std::string line, Server &server) {
  (void)server;
  size_t first_space = line.find(' ');
  if (first_space == std::string::npos) {
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

void Client::_user_cmd(std::string line, Server &server) {
  (void)server;
  size_t first_space = line.find(' ');
  if (first_space == std::string::npos) {
    std::string reply = "Error: USER requires a username\r\n";
    ft_send(reply);
    return;
  }
  std::string params = line.substr(first_space + 1);
  size_t second_space = params.find(' ');
  if (second_space == std::string::npos) {
    _user = params;
  } else {
    _user = params.substr(0, second_space);
  }
  if (!_user.empty() && _user[_user.size() - 1] == '\r')
    _user.erase(_user.size() - 1);

  size_t colon_pos = line.find(':');
  if (colon_pos != std::string::npos) {
    std::string real_name = line.substr(colon_pos + 1);
    if (!real_name.empty() && real_name[real_name.size() - 1] == '\r')
      real_name.erase(real_name.size() - 1);
    _name = real_name;
  }

  _welcome(server);
}

void Client::_cap(std::string line, Server &server) {
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

  if (subcommand == "LS") {
    _waiting_for_cap_end = true;
    std::string reply = "CAP * LS :\r\n";
    ft_send(reply);
  } else if (subcommand == "REQ") {
    std::string reply = "CAP * NAK :\r\n";
    ft_send(reply);
  } else if (subcommand == "END") {
    _waiting_for_cap_end = false;
    _welcome(server);
  }
}

void Client::_welcome(Server &server) {
  (void)server;
  if (!_nick.empty() && !_user.empty() && !_is_registered &&
      !_waiting_for_cap_end) {
    _is_registered = true;

    std::string reply;
    reply = server.getServerPrefix() + " 001 " + _nick +
            " :Welcome to the Internet Relay Network " + _nick + "!" + _user +
            "@localhost\r\n";
    ft_send(reply);

    reply = server.getServerPrefix() + " 002 " + _nick +
            " :Your host is ircserv, running version 1.0\r\n";
    ft_send(reply);

    reply = server.getServerPrefix() + " 003 " + _nick +
            " :This server was created today\r\n";
    ft_send(reply);

    reply = server.getServerPrefix() + " 004 " + _nick + " ircserv 1.0 - -\r\n";
    ft_send(reply);
  }
}

void Client::process_line(std::string line, Server &server, int index) {
  (void)index;
  size_t first_space = line.find(' ');
  std::string command =
      (first_space == std::string::npos) ? line : line.substr(0, first_space);

  if (command == "CAP") {
    _cap(line, server);
    return;
  }

  if (command == "PASS") {
    if (_is_authenticated) {
      std::string reply = "Error: You are already authenticated\r\n";
      ft_send(reply);
    } else {
      _pass(line, server);
    }
    return;
  }

  if (command == "NICK") {
    _nick_cmd(line, server);
    return;
  }

  if (command == "USER") {
    _user_cmd(line, server);
    return;
  }

  /*if (!_is_registered) {
    std::string reply =
        "Error: You must register (NICK <nick>, USER <user> ...)\r\n";
    ft_send(reply);
    return;
  }*/

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
  else if (command == "HELP")
    _help(line, server);
  else {
    std::string reply = line + "\r\n";
    ft_send(reply);
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
  ft_send(reply);
}

void Client::_join(std::string line, Server &server) {
  size_t first_space = line.find(' ');
  if (first_space == std::string::npos) {
    std::string reply = "Error: JOIN requires a channel name\r\n";
    ft_send(reply);
    return;
  }
  std::string channel_name = line.substr(first_space + 1);
  if (!channel_name.empty() && channel_name[channel_name.size() - 1] == '\r')
    channel_name.erase(channel_name.size() - 1);

  Channel *existing_channel = server.getChannel(channel_name);
  if (existing_channel) {
    existing_channel->addClient(*this);
    std::string reply = "Joined channel " + channel_name + "\r\n";
    ft_send(reply);
  } else {
    Channel c;
    c.setName(channel_name);
    c.addClient(*this);
    if (server.addChannel(c)) {
      std::string reply = "Channel " + channel_name + " created\r\n";
      ft_send(reply);
    } else {
      std::string reply = "Error: Max channels reached\r\n";
      ft_send(reply);
    }
  }
}

void Client::_part(std::string line, Server &server) {
  size_t first_space = line.find(' ');
  if (first_space == std::string::npos) {
    std::string reply = "Error: PART requires a channel name\r\n";
    ft_send(reply);
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
      ft_send(reply);
      if (c->getClientsNumber() == 0) {
        server.removeChannel(channel_name);
        std::cout << "Channel " << channel_name << " removed" << std::endl;
      }
    } else {
      std::string reply = "Error: No such channel " + channel_name + "\r\n";
      ft_send(reply);
    }
    channels_str.erase(0, pos + 1);
  }
  if (!channels_str.empty()) {
    std::string channel_name = channels_str;
    Channel *c = server.getChannel(channel_name);
    if (c) {
      c->rmClient(*this);
      std::string reply = "You left channel " + channel_name + "\r\n";
      ft_send(reply);
      if (c->getClientsNumber() == 0) {
        server.removeChannel(channel_name);
        std::cout << "Channel " << channel_name << " removed" << std::endl;
      }
    } else {
      std::string reply = "Error: No such channel " + channel_name + "\r\n";
      ft_send(reply);
    }
  }
}

void Client::_kick(std::string line, Server &server) {
  (void)line;
  (void)server;
  std::string reply = "Command KICK not implemented yet\r\n";
  ft_send(reply);
}

void Client::_invite(std::string line, Server &server) {
  (void)line;
  (void)server;
  std::string reply = "Command INVITE not implemented yet\r\n";
  ft_send(reply);
}

void Client::_topic(std::string line, Server &server) {
  (void)line;
  (void)server;
  std::string reply = "Command TOPIC not implemented yet\r\n";
  ft_send(reply);
}

void Client::_mode(std::string line, Server &server) {
  (void)line;
  (void)server;
  std::string reply = "Command MODE not implemented yet\r\n";
  ft_send(reply);
}

void Client::_auth(std::string line, Server &server) {
  (void)line;
  (void)server;
  std::string reply = "Command AUTH not implemented yet\r\n";
  ft_send(reply);
}