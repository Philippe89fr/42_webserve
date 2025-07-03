/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 13:31:19 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/06 11:56:35 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void	check_valid_serv(std::vector<t_serveur>& servers)
{
	std::vector<t_serveur>::iterator	it = servers.begin();
	while (it != servers.end())
	{
		if (it->socket != -1)
			return;
		++it;
	}
	throw std::runtime_error("Error: no valid server socket");
}

void	launch_servers(std::vector<t_serveur>& servers)
{
	std::cout << "---- SERVER INIT ----" << std::endl << std::endl;
    int status;

	for (size_t i = 0; i < servers.size(); ++i)
    {
		// creation du socket serveur
		servers[i].socket = server_socket_creation(&servers[i]); // bind + socket_serveur
		if (servers[i].socket == -1) {
			std::cerr << "[Server] Creation error: " << strerror(errno) << std::endl << std::endl;
			return ;
		}
//		std::cout << "socket server[" << i << "] :" << servers[i].socket << std::endl;

		// Listen du PORT via la socket
		std::cout << "[Server] Listening on port " << servers[i].port << std::endl << std::endl;
		status = listen(servers[i].socket, BACKLOG);
		if(status != 0) {
			std::cerr << "[Server] Listen error: " << strerror(errno) << std::endl << std::endl;
			return ;
		}
	}
}

int		main(int ac, char **av)
{
    if (ac > 2) {
		std::cerr << "Error: Please enter just one config file" << std::endl;
		return 1;
	}
	std::string	conf_file_path;
	if (ac == 1)
		conf_file_path = "conf_files/ConfigBase.conf";
	else
		conf_file_path = av[1];
	std::ifstream	configFile(conf_file_path.c_str());
	if (!configFile.is_open()) {
		std::cerr << "Error: Cannot open config file" << std::endl;
		return 1;
	}
	sighandler_init();
    std::vector<t_serveur> data_config;
	try
	{
		ft_configFileparsing(configFile, data_config);
		configFile.close();
		//printDataConfig(data_config);
		launch_servers(data_config);
		check_valid_serv(data_config);
		waiting_connection(data_config);
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
	std::vector<t_serveur>::iterator	it = data_config.begin();
	while (it != data_config.end())
	{
		if (it->socket != -1)
			close(it->socket);
		++it;
	}
    return (0);
}
