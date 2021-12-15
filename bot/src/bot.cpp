#include "bot.hpp"

std::vector<std::string> Bot::split(std::string str, std::string delimiter)
{
	std::vector<std::string> values = std::vector<std::string>();

	size_t position;
	while ((position = str.find(delimiter)) != std::string::npos)
	{
		values.push_back(str.substr(0, position));
		str.erase(0, position + delimiter.length());
	}
	values.push_back(str);

	return values;
}

Bot::Bot(std::string addr, int port, std::string pass, std::string nick): addr(addr), pass(pass), nick(nick)
{
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("socket() failed");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(addr == "localhost" ? "127.0.0.1" : addr.c_str());
	std::cout << "Connecting to " << addr << ":" << port << std::endl;
	log.open("bot.log");
}

Bot::~Bot()
{
	send_msg("QUIT");
	close(sock);
	log.close();
}

void Bot::run()
{
	while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cerr << "connect() failed" << std::endl;
		std::cout << "Retrying in 5 seconds..." << std::endl;
		sleep(5);
	}
	send_pass();
	while (!connected)
	{
		try
		{
			send_nick();
		}
		catch (std::runtime_error &e)
		{
			std::cerr << e.what() << std::endl;
			nick += "_";
		}
	}
	std::cout << "Connected as " << nick << std::endl;
	send_msg("USER " + nick + " " + nick + " " + addr + " :" + nick);
	//while (recv_msg().find("376 " + nick) == std::string::npos)
	while (true)
	{
		if (messages.empty())
			recv_msg();
		if (DEBUG)
			std::cout << messages[0] << std::endl;
		std::string msg = messages[0];
		messages.erase(messages.begin());
		if (msg.find("376 " + nick) != std::string::npos)
			break;
	}
	std::cout << "Joined server " << addr << std::endl;
	//send_msg("MODE " + nick + " +B");
	send_msg("JOIN #" + nick);
	std::cout << "Joined #" << nick << std::endl;
	while (true)
	{
		if (messages.empty())
			recv_msg();
		std::string msg = messages[0];
		messages.erase(messages.begin());
		if (msg.empty())
			continue;
		if (DEBUG)
			std::cout << msg << std::endl;
		if (msg.find("PING") != std::string::npos)
			send_msg("PONG " + msg.substr(5));
		else
		{
			std::vector<std::string> values = split(msg, " ");
			if (std::find(values.begin(), values.end(), "JOIN") != values.end())
				log << C_B_YELLOW << &values[0].substr(0, values[0].find('!'))[1] << C_B_RESET << " joined" << std::endl;
			else if (std::find(values.begin(), values.end(), "PRIVMSG") != values.end())
			{
				log << "<" << C_YELLOW << &values[0].substr(0, values[0].find('!'))[1] << C_RESET << "> ";
				for (std::vector<std::string>::iterator it = values.begin() + 3; it != values.end(); ++it)
				{
					if (it != values.begin() + 3)
						log << " ";
					log << *it;
				}
				log << std::endl;
				if (values[3] == ":flip")
					send_msg("NOTICE " + values[2] + " :" + (rand() % 2 == 0 ? "heads" : "tails"), true);
			}
			else if (std::find(values.begin(), values.end(), "PART") != values.end())
				log << C_B_YELLOW << &values[0].substr(0, values[0].find('!'))[1] << C_RESET << " leave" << std::endl;
		}
	}
}

void Bot::send_msg(std::string msg, bool toLog)
{
	if (toLog)
	{
		std::vector<std::string> values = split(msg, " ");
		log << "<" << C_YELLOW << nick << C_RESET << "> ";
		for (std::vector<std::string>::iterator it = values.begin() + 2; it != values.end(); ++it)
		{
			if (it != values.begin() + 2)
				log << " ";
			log << *it;
		}
		log << std::endl;
	}
	msg += "\r\n";
	if (send(sock, msg.c_str(), msg.size(), 0) < 0)
		throw std::runtime_error("send() failed");
}

void Bot::recv_msg()
{
	char buf[4096 + 1];
	struct pollfd pfd;

	pfd.fd = sock;
	pfd.events = POLLIN;
	if (poll(&pfd, 1, 0) == -1)
		throw std::runtime_error("poll() failed");

	ssize_t len = recv(sock, &buf, 4096, 0);
	if (len < 0)
		throw std::runtime_error("recv() failed");
	if (len == 0)
		throw std::runtime_error("Connection closed");
	buf[len] = '\0';
	receive += buf;
	while (receive.find("\r\n") != std::string::npos)
	{
		std::string msg = receive.substr(0, receive.find("\r\n"));
		receive.erase(0, receive.find("\r\n") + 2);
		messages.push_back(msg);
	}
}

void Bot::send_pass()
{
	if (pass.empty())
		return;
	send_msg("PASS " + pass);
}

void Bot::send_nick()
{
	send_msg("NICK " + nick);
	recv_msg();
	std::string msg = messages[0];
	messages.erase(messages.begin());
	if (msg.find("433") != std::string::npos)
		throw std::runtime_error("Nickname already in use");
	connected = true;
}
