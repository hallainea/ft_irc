#include "Command.hpp"
#include "../User.hpp"
#include "../../utils/utils.hpp"
#include "../../Server/Server.hpp"

void NICK(irc::Command *command)
{
	if (!command->getParameters().size())
		return command->reply(431);
	const char *nickname = command->getParameters()[0].c_str();

	//check valid nick
	if (command->getParameters()[0].length() > 9)
		return command->reply(432, nickname);
	size_t index = 0;
	if (!irc::isLetter(nickname[index]) && !irc::isSpecial(nickname[index]))
		return command->reply(432, nickname);
	++index;
	for (; index < command->getParameters()[0].length(); ++index)
		if (!irc::isLetter(nickname[index]) && !irc::isSpecial(nickname[index]) && !irc::isDigit(nickname[index]) && nickname[index] != '-')
			return command->reply(432, nickname);

	std::vector<irc::User *> users = command->getServer().getUsers();
	for (std::vector<irc::User *>::iterator it = users.begin(); it != users.end(); it++)
		if (command->getParameters()[0] == (*it)->getNickname())
			return command->reply(433, command->getParameters()[0]);

	if (command->getUser().getNickname().length())
		command->getUser().setPastnick(" " + command->getUser().getNickname() + " " + command->getUser().getPastnick());

	command->getUser().write(":" + command->getUser().getPrefix() + " NICK " + command->getParameters()[0]);
	command->getUser().setNickname(command->getParameters()[0]);
}
