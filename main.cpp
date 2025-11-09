#include <iostream>

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "usage: " << argv[0] << " <port> <password>" << std::endl;
		return (1);
	}
	int	port = std::atoi(argv[1]);
	std::string	password(argv[2]);

	return (0);
}
