#include "../PacketManager.hpp"
#include <sstream>
#include <iostream>

void check_mode(std::string *mode, char option, bool is_minus, std::string options)
{
	size_t i = 0;

	while (i != options.size())
	{
		if (option == options[i])
		{
			if (mode->find(options[i]) != std::string::npos && is_minus)
				mode->erase(mode->begin() + mode->find(options[i]));
			else if (mode->find(options[i]) == std::string::npos && !is_minus)
				mode->insert(mode->end(), options[i]);
		}
		i++;
	}
}

void irc::MODE(struct irc::packetParams params)
{
	if (params.args.size() == 1)
		return;

	std::stringstream ss;
	std::string mode = params.user->getMode();
	bool is_minus = false;
	size_t i = 0;

	if (mode.size() == 0)
		mode = "+";

	if (params.args[1] != params.user->getNickname())
		return params.user->write(502);

	while (params.args.back() != params.args[1] && i != params.args[2].size())
	{
		if (params.args[2][i] == '-')
			is_minus = true;
		else if (params.args[2][i] == '+')
			is_minus = false;
		//else if (params.args[2][i] == 'o')
		else if (params.args[2][i] == 'i')
			check_mode(&mode, 'i', is_minus, params.user->getUser_modes());
		else if (params.args[2][i] == 's')
			check_mode(&mode, 's', is_minus, params.user->getUser_modes());
		else if (params.args[2][i] == 'w')
			check_mode(&mode, 'w', is_minus, params.user->getUser_modes());
		else
		{
			ss << params.args[2][i];
			params.user->write(472, ss.str());
			ss.str(std::string());
		}
		i++;
	}
	params.user->setMode(mode);
	params.user->write(221, mode);
}
