#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
	private:
		std::string	_name;
		std::string	_nick;
		int	_fd;

	public:
		Client(void);
		~Client(void);

		std::string	buffer;
		const static int	MAX_CLIENTS = 10;

		std::string	getName(void);
		std::string	getNick(void);
		int	getFd(void);

		void	setName(std::string name);
		void	setNick(std::string nick);
		void	setFd(int fd);
};

#endif