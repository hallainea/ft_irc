#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sstream>
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>

void irc::Server::pending()
{
	struct pollfd pfd;
	pfd.fd = tcp_socket;
	pfd.events = POLLIN;
	poll(&pfd, 1, -1);
}
void irc::Server::displayUsers()
{
	std::stringstream ss;
	ss << "Users: " << users.size();
	display.setLine(1, ss.str());
}
void irc::Server::registerUsers()
{
	if (users.empty())
	{
		std::stringstream ss;
		ss << WARNING << "No user, waiting for a connection...";
		display.setLine(1, ss.str());
		pending();
	}
	while (true)
	{
		struct sockaddr_in address;
		socklen_t csin_len = sizeof(address);
		int fd = accept(tcp_socket, (struct sockaddr *)&address, &csin_len);
		if (fd == -1)
			break;
		users.push_back(new User(fd, inet_ntoa(address.sin_addr)));
		displayUsers();
	}
}

std::string irc::Server::init_time()
{
	time_t t = std::time(0);
	struct tm *now = std::localtime(&t);
	return (asctime(now));
}

irc::Server::Server(unsigned short port, std::string password)
	: users(), display(), channels(display), packet(channels, *this)
{
	if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	int opt = 1;
	if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	time = init_time();
	(void)password;
	display.setLine(0, "Welcome to our \033[1;37mIRC\n");

	if (fcntl(tcp_socket, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("fcntl");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(tcp_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (listen(tcp_socket, port) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
}
irc::Server::~Server()
{
	std::vector<User *>::iterator it = users.begin();
	std::vector<User *>::iterator ite = users.end();
	while (it != ite)
		delete *it;
	close(tcp_socket);
}

void irc::Server::run()
{
	while (true)
	{
		registerUsers();

		std::vector<User *>::iterator it = users.begin();
		std::vector<User *>::iterator ite = users.end();
		while (it != ite)
		{
			packet.request((*it)->read(), *it);
			it++;
		}
	}
}
void irc::Server::kill(User *user)
{
	std::vector<User *>::iterator it = users.begin();
	std::vector<User *>::iterator ite = users.end();
	while (it != ite)
	{
		if (*it == user)
		{
			users.erase(it);
			delete *it;
			displayUsers();
			return;
		}
		it++;
	}
}

std::string irc::Server::getServername() { return ("SHIBRC"); }
std::string irc::Server::getVersion() { return ("1.69"); }
std::string irc::Server::getTime() { return (time); }
std::vector<irc::User *> &irc::Server::getUsers() { return users; }
