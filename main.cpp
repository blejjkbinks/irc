#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define BACKLOG 16
#define MAX_CLIENTS 100
#define BUFF_SIZE 4000

static volatile bool g_running = true;

struct Client
{
	int	fd;
	std::string	buffer;
};

static int	ft_make_listen_socket(int port);
static void	ft_accept_new_clients(int listen_fd, pollfd *pfds, Client *clients, int &nfds);
static void	ft_handle_client_io(pollfd *pfds, Client *clients, int i);
static void	ft_compact_fds(pollfd *pfds, Client *clients, int &nfds);
void		handle_sigint(int);


int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "usage: " << argv[0] << " <port> <password>" << std::endl;
		return (1);
	}
	int	port = std::atoi(argv[1]);
	std::string	password(argv[2]);
	signal(SIGINT, handle_sigint);

	int listen_fd = ft_make_listen_socket(port);
	if (listen_fd == -1)
	{
		std::cerr << "error trying to create listening socket: " << std::strerror(errno) << std::endl;
		return (1);
	}

	pollfd pfds[MAX_CLIENTS + 1];
	Client clients[MAX_CLIENTS];
	int	nfds = 1;
	pfds[0].fd = listen_fd;
	pfds[0].events = POLLIN;
	for (int i = 0; i < MAX_CLIENTS; ++i)
		clients[i].fd = -1;

	while (g_running)
	{
		if (poll(pfds, nfds, -1) < 0)
		{
			if (errno == EINTR) continue;
			std::cerr << "poll error: " << std::strerror(errno) << std::endl;
			break;
		}

		if (pfds[0].revents & POLLIN)
			ft_accept_new_clients(listen_fd, pfds, clients, nfds);
		
		for (int i = 1; i < nfds; ++i)
			ft_handle_client_io(pfds, clients, i);
		
		ft_compact_fds(pfds, clients, nfds);
	}

	for (int i = 1; i < nfds; ++i)
	{
		if (pfds[i].fd != -1)
		{
			std::string	message = "server shutting down\r\n";
			send(pfds[i].fd, message.c_str(), message.size(), 0);
			close(pfds[i].fd);
		}
	}

	close(listen_fd);
	std::cerr << port << ", " << password << std::endl;
	return (0);
}

static int	ft_make_listen_socket(int port)
{
	int	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		return (-1);
	int	yes = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	int flags = fcntl(listen_fd, F_GETFL, 0);
	fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

	sockaddr_in	addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(listen_fd);
		return (-1);
	}
	if (listen(listen_fd, BACKLOG) < 0)
	{
		close(listen_fd);
		return (-1);
	}
	return (listen_fd);
}

static void	ft_accept_new_clients(int listen_fd, pollfd *pfds, Client *clients, int &nfds)
{
	for (;;)
	{
		sockaddr_in	client_addr;
		socklen_t	len = sizeof(client_addr);
		int		client_fd = accept(listen_fd, (sockaddr*)&client_addr, &len);
		if (client_fd < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			std::cerr << "accept error: " << std::strerror(errno) << std::endl;
			break;
		}
		int flags = fcntl(client_fd, F_GETFL, 0);
		fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

		if (nfds < MAX_CLIENTS + 1)
		{
			pfds[nfds].fd = client_fd;
			pfds[nfds].events = POLLIN;
			pfds[nfds].revents = 0;
			clients[nfds - 1].fd = client_fd;
			clients[nfds - 1].buffer = "";
			++nfds;
			std::cout << "client connected to slot " << nfds - 1 << std::endl;
		}
		else
		{
			std::cerr << "max_client limit reached" << std::endl;
			close(client_fd);
		}
	}
}

static void	ft_handle_client_io(pollfd *pfds, Client *clients, int i)
{
	int fd = pfds[i].fd;
	if (fd < 0)
		return;
	
	if (pfds[i].revents & (POLLIN | POLLERR | POLLHUP))
	{
		char buf[BUFF_SIZE];
		ssize_t n = recv(fd, buf, sizeof(buf), 0);
		if (n > 0)
		{
			clients[i - 1].buffer.append(buf, n);
			std::string	&b = clients[i - 1].buffer;
			size_t	pos;
			while ((pos = b.find('\n')) != std::string::npos)
			{
				std::string	line = b.substr(0, pos);
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				std::cout << "receved from " << i << ": " << line << std::endl;

				std::string	reply = "sending back [" + line + "]\r\n";
				send(fd, reply.c_str(), reply.size(), 0);

				b.erase(0, pos + 1);
			}
		}
		else
		{
			if (n == 0 || (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN))
			{
				std::cout << "client disconnected from slot " << i << std::endl;
				close(fd);
				pfds[i].fd = -1;
				clients[i - 1].fd = -1;
			}
		}
	}
}

static void	ft_compact_fds(pollfd *pfds, Client *clients, int &nfds)
{
	int	new_nfds = 1;
	for (int i = 1; i < nfds; ++i)
	{
		if (pfds[i].fd != -1)
		{
			if (new_nfds != i)
			{
				pfds[new_nfds] = pfds[i];
				clients[new_nfds - 1] = clients[i - 1];
			}
			new_nfds++;
		}
	}
	nfds = new_nfds;
}

void	handle_sigint(int)
{
	g_running = false;
}