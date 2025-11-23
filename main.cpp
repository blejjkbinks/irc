#include "Server.hpp"
#include <stdexcept>

void parsePortPassword(char *argv[], int &port, std::string &password)
{
	port = atoi(argv[1]);
	password = argv[2];
	if (port <= 0 || port >= 65535)
		throw std::invalid_argument("invalid port");
	if (password.empty())
		throw std::invalid_argument("invalid password");
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << " usage : ./ircserv [port] [password]\n";
		return (1);
	}

	int port;
	std::string password;
	try
	{
		parsePortPassword(argv, port, password);
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl; 
		return (1);
	}

	Server srv(port, password);
	srv.start();
	return (0);
}