#include "utils.hpp"
#include <ctime>
#include <iostream>
#include <cstdlib>

std::string irc::currentTime()
{
	time_t t = std::time(0);
	struct tm *now = std::localtime(&t);
	return (asctime(now));
}

void irc::error(std::string message)
{
	std::cerr << "error: " << message << std::endl;
	exit(EXIT_FAILURE);
}
std::vector<std::string> irc::split(std::string str, std::string delimiter)
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
bool irc::isLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
bool irc::isSpecial(char c) { return (c >= '[' && c <= '`') || (c >= '{' && c <= '}'); }
bool irc::isDigit(char c) { return (c >= '0' && c <= '9'); }