/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   methodeHandlerUtils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/20 16:13:45 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/12 12:18:44 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void	freeall(t_globalData &dataStruct)
{
		std::vector<t_serveur>::iterator	vec_it = dataStruct.d_servers->begin();
		std::map<int, t_client>::iterator	map_it = dataStruct.d_clients.begin();

		while (vec_it != dataStruct.d_servers->end())
		{
			if (vec_it->socket != -1)
				close(vec_it->socket);
			++vec_it;
		}
		while (map_it != dataStruct.d_clients.end())
		{
			close(map_it->first);
			CGIProcess	*processCGI = map_it->second.request.getCGIProcess();

			if (map_it->second.buffer)
			{
				delete[] map_it->second.buffer;
				map_it->second.buffer = NULL;
			}
			if (processCGI)
			{
				close(processCGI->getFdIn());
				close(processCGI->getFdOut());
				close(processCGI->getTempFileFd());
			}
			++map_it;
		}
}

//Dossier ?
//oui --> correspondance exacte avec l'entierete de l'Uri
//non --> on enleve le /file et avec ce qu'il reste, on cherche correspondance exacte. Pb si on a /style.css -> plus rien apres -file --> root
// cherche la bonne location au sein d'un serveur

void	getrequestlocation(std::vector<t_serveur> &server_vec, HTTPRequest &requestHTTP)
{
	std::string							buffer = requestHTTP.getUri();
	std::vector<t_serveur>::iterator	it = server_vec.begin();

	while (it != server_vec.end())
	{
		t_serveur							&server = *it;
		std::vector<t_location>::iterator	iter = server.locations.begin();

		while (iter != server.locations.end())
		{
			if (buffer.rfind(iter->path, 0) != std::string::npos)
			{
				if (!requestHTTP.getLocation() || iter->path.size() > requestHTTP.getLocation()->path.size())
				{
					requestHTTP.setLocation(*iter);
					requestHTTP.setServeur(server);
				}
			}
			++iter;
		}
		++it;
	}
	return ;
}

void	makepathfromURI(const t_serveur &server, HTTPRequest &requestHTTP, const t_location &current_location)
{
	struct stat	path_stat;
	std::string	path;

    if (current_location.root.size())
        path = current_location.root + requestHTTP.getUri().substr(current_location.path.size());
    else {
        path = server.root + requestHTTP.getUri();
    }
	if (*path.begin() != '.')
		path = '.' + path;
	int i = stat(path.c_str(), &path_stat);
	if (i != -1 && S_ISDIR(path_stat.st_mode))
	{
		if (*(--path.end()) != '/')
			path += '/';
		path += current_location.index;
	}
	//std::cout << std::endl << "found path: " << path << std::endl << std::endl;
    requestHTTP.setUriPath(path);
}
