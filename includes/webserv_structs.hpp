/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_structs.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 10:35:30 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/03 11:32:33 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

typedef struct s_location
{
    std::string							path;
    std::string							index;
    bool								autoindex;
    std::string							upload_dir;
	bool								index_bool;
    std::string							root;
    std::vector<std::string>			methods;
    std::pair<std::string, std::string>	return_;
    size_t								client_max_body_size;
    std::map<std::string,std::string>	cgi;
    bool								cgi_bool;
} t_location;

typedef struct s_serveur
{
    int									socket;
    std::string							port;
    std::string							serverName;
    std::string							adressIP;
    std::string							index;
    std::string							root;
    struct sockaddr_in					sockaddr_in;
    std::map<std::string,std::string>	error_page;
    size_t								client_max_body_size;
    std::vector<t_location>				locations;
} t_serveur;

typedef struct s_request_buffer
{
    char	*buffer;
    size_t	size;      // Taille actuelle des datas
} t_request_buffer;

typedef struct s_dir
{
    std::string	name;
    std::string	size;
    std::string	modifDate;
} t_dir;

typedef struct s_client
{
    int						fd;
    time_t                  time_start;
    HTTPRequest				request;
    HTTPResponse			response;
    char					*buffer;
    size_t					size;
    int						statut;//0 on attend il se passe r // 1 on lit // 2 on process //3 on ecrit
    bool					responseReady;
    t_formdata				datasBuffer;
    std::vector<t_serveur>	serversPort;
}   t_client;

typedef struct s_globalData
{
    std::vector<t_serveur>	*d_servers;
    fd_set					d_all_read_sockets; // Ensemble de toutes les sockets du serveur. Ici on ajoute les descripteurs de fichier de nos nouvelles connexions, et enlever les sockets qui ont été fermées
    fd_set					d_all_write_sockets;
    fd_set					d_read_fds;    // Ensemble temporaire pour select()
    fd_set					d_write_fds;
    int						d_fdMax;
    std::map<int, t_client>	d_clients;
    struct timeval			d_timer;
} t_globalData;


