/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   confParsing.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 14:18:20 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 15:46:45 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void give_locations_default_params(std::vector<t_serveur>& data_config)
{
    for (size_t i = 0; i < data_config.size(); ++i)
    {
        for (size_t j = 0; j < data_config[i].locations.size(); ++j)
        {
            if (!data_config[i].locations[j].client_max_body_size)
                data_config[i].locations[j].client_max_body_size = data_config[i].client_max_body_size;
            if (data_config[i].locations[j].index == "" && !data_config[i].locations[j].autoindex)
                data_config[i].locations[j].index = data_config[i].index;
        }
		if (data_config[i].adressIP == "")
			data_config[i].adressIP = "127.0.0.1";
    }
}

void ft_location_init(t_location &location) {
    location.autoindex = false;
    location.cgi_bool = false;
    location.path = "";
    location.index = "";
    location.upload_dir = "";
    location.root = "";
    location.client_max_body_size = 0;
    location.index_bool = false;
    location.cgi.clear();
    location.return_ = std::make_pair("", "");
}

void ft_server_init(t_serveur &data) {
    data.adressIP = "";
    data.port = "";
    data.serverName = "";
    data.root = "";
    data.index = "";
    data.client_max_body_size = 0;
    data.error_page.clear();
    data.locations.clear();
}

void locations_parsing(std::ifstream &configFile, t_location &location)
{
    std::string			line;
    std::string			word;

    while(std::getline(configFile, line))
    {
		if (!line.size())
			continue;
		else
		{
    		std::istringstream	flux(line);
			flux >> word;
			if (word == "}")
				return ;
			if (word == "index")
			{
				flux >> word;
				location.index_bool = true;
				location.index = word;
			} // en faire un vecteur ?
			else if (word == "autoindex")
			{
				flux >> word;
				if (word == "on")
					location.autoindex = true;
			}
			else if (word == "upload_dir")
			{
				flux >> word;
				location.upload_dir = word;
			}
			else if (word == "root")
			{
				flux >> word;
				location.root = word;
			}
			else if (word == "cgi") {
				std::string	word2;
				flux >> word;
				flux >> word2;
				location.cgi[word] = word2;
				location.cgi_bool = true;
			}
			else if (word == "allow_methods")
			{
				while (flux >> word)
					location.methods.push_back(word);
			}
			else if (word == "return")
			{
				std::string	word2;
				flux >> word;
				if (!word.size() || (word != "301" && word != "302"))
				{
					std::string	err_msg("Conf file error: no error code returned at line: '" + line + "'");
					throw std::runtime_error(err_msg);
				}
				flux >> word2;
				location.return_ = std::make_pair(word, word2);
			}
			else if (word == "client_max_body_size") {
				flux >> word;
				location.client_max_body_size = atoi(word.c_str());
			}
			else
			{
				std::string	err_msg("Conf file error: unknown location parameter: '" + line + "'");
				throw std::runtime_error(err_msg);
			}
		}
    }
	std::string	err_msg("Conf file error: unknown location parameter: '" + line + "'");
	throw std::runtime_error(err_msg);
}

void ft_configFileparsing(std::ifstream &configFile, std::vector<t_serveur> &data)
{
    int					i = -1;
    bool				in_serv = false;
    std::string			line;
    std::string			word;

	while(std::getline(configFile, line))
    {
		if (!line.size())
			continue;
		else if (!in_serv)
        {
			if (line == "server {")
			{
				in_serv = true;
				++i;
				t_serveur	new_server;
				ft_server_init(new_server);
				data.push_back(new_server);
			}
			else
			{
				std::string	err_msg("Conf file error: wrong server declaration at line: '" + line + "'");
				throw std::runtime_error(err_msg);
			}
        }
		else if (line == "}")
			in_serv = false;
		else
		{
    		std::istringstream	flux(line);
			flux >> word;
			if (word == "location")
			{
				t_location	new_location;
				ft_location_init(new_location);
				flux >> word;
				new_location.path = word;
				flux >> word;
				if (word != "{")
				{
					throw std::runtime_error("Conf file error: location declaration syntax error at line: '" + line + "'");
				}
				locations_parsing(configFile, new_location);
				data[i].locations.push_back(new_location);
			}
			else if (word == "listen") {
				flux >> word;
				data[i].port = word;
			}
			else if (word == "root") {
				flux >> word;
				data[i].root = word;
			}
			else if (word == "server_name") {
				flux >> word;
				data[i].serverName = word;
			}
			else if (word == "host") {
				flux >> word;
				data[i].adressIP = word;
			}
			else if (word == "index") {
				flux >> word;
				data[i].index = word;
				} // en faire un vecteur ?
			else if (word == "error_page") {
				std::string word2;
				flux >> word;
				flux >> word2;
				data[i].error_page[word] = word2;
			}
			else if (word == "client_max_body_size") {
				flux >> word;
				if (atoi(word.c_str()) == 0)
					data[i].client_max_body_size = 100;//pour pouvoir tester//
				else
					data[i].client_max_body_size = atoi(word.c_str()) * 1024 * 1024;
			}
			else
				throw std::runtime_error("Conf file error: unknown server parameter: '" + line + "'");
        }
    }
	if (in_serv)
		throw std::runtime_error("Conf file error: unclosed server declaration");
	give_locations_default_params(data);
}

//void printVector(const std::vector<std::string>& vec, const std::string& label)
//{
//    std::cout << label;
//    for (size_t i = 0; i < vec.size(); ++i)
//    {
//        std::cout << vec[i];
//        if (i != vec.size() - 1)
//            std::cout << ", ";
//    }
//    std::cout << std::endl;
//}
//
//void printDataConfig(const std::vector<t_serveur>& data_config)
//{
//    for (size_t i = 0; i < data_config.size(); ++i)
//    {
//        const t_serveur& srv = data_config[i];
//        std::cout << "===== Serveur " << i + 1 << " =====" << std::endl;
//        std::cout << "Port: " << srv.port << std::endl;
//        std::cout << "Server Name: " << srv.serverName << std::endl;
//        std::cout << "IP Address: " << srv.adressIP << std::endl;
//        std::cout << "Root: " << srv.root << std::endl;
//        std::cout << "Index: " << srv.index << std::endl;
//        std::cout << "Client Max Body Size: " << srv.client_max_body_size << std::endl;
//        std::map<std::string,std::string>::const_iterator it;
//        for (it = srv.error_page.begin(); it !=srv.error_page.end() ; it++)
//        {
//            std::cout << "Error_page: " << it->first << ": " << it->second << std::endl;
//        }
//        for (size_t j = 0; j < srv.locations.size(); ++j)
//        {
//            const t_location& loc = srv.locations[j];
//            std::cout << "--- Location " << j + 1 << " ---" << std::endl;
//            std::cout << "Path: " << loc.path << std::endl;
//            std::cout << "Index: " << loc.index << std::endl;
//            std::cout << "Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
//            std::cout << "Upload Dir: " << loc.upload_dir << std::endl;
//            std::cout << "Root: " << loc.root << std::endl;
//            std::cout << "Client Max Body Size: " << loc.client_max_body_size << std::endl;
//            std::cout << "cgi_bool: " << loc.cgi_bool << std::endl;
//            for (it = loc.cgi.begin(); it !=loc.cgi.end() ; it++)
//            {
//                std::cout << "cgi: " << it->first << ": " << it->second << std::endl;
//            }
//            printVector(loc.methods, "Methods: ");
//			std::cout << "return_: " << loc.return_.first << " | " << loc.return_.second << std::endl;
//        }
//        std::cout << std::endl;
//    }
//}
