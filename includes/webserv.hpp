/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 13:33:54 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:31:45 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <cstring>
#include <poll.h>
#include <sstream>   // pour std::ostringstream
#include <string>    // pour std::string
#include <iostream>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <netdb.h>
#include <stdlib.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/socket.h>
//#include <asm-generic/socket.h>
//#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h> // pour inet_pton
#include <sys/wait.h> // pour waitpid les CGIs
#include <sys/stat.h> // pour verifier si l'URI est un dossier
//#include <bits/stdc++.h>
#include "webserv_structs.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "CGIProcess.hpp"
#include <sys/time.h>

#define BACKLOG 5  // nombre max de demandes de connexion dans la file d'attente
#define CHUNKLEN 100 // nombre d'octets lus/ecrits par boucle de CGI
#define CHUNKECR 500 // nombre d'octets ecrits par boucle d upload
#define CHUNKLE 100000 // nombre d'octets lu par boucle RECV
#define CHUNKREAD 100000 // nombre d'octets lu pour GET
#define CHUNKSEND 100000 // nbr d octets SENT/Write par boucle a la fin

#define DONE 0
#define READING 1
#define PROCESSING 2
#define WRITING 3

int									get_serveur_port_number(int j, std::vector<t_serveur>& servers);
void								divide_multipart(std::string &boundary, t_client &client);
void								removeSocketUpdateMax(int socket, t_globalData &dataStruct);
void								serveurNonBlocking(int socket_fd);
std::vector<t_serveur>				findServerWithPort(std::vector<t_serveur>& servers, std::string port);
HTTPResponse						handleDelete(HTTPRequest &requestHTTP);
std::map<std::string, std::string>	parse_urlencoded(const std::vector<char>& body);
void								handlePost(const t_serveur &server, t_client &client, const t_location &current_location);
std::string							getHTTPDate();
std::string							GetMimeType(const std::string& path);
HTTPResponse						pushAutoindex(const t_serveur &server, const t_location &current_location, std::string path);
void								executeRequest(std::vector<t_serveur> &serversPort, t_client &client, t_globalData &dataStruct);
HTTPRequest 						parsHttpRequest(t_client &client);
void								makepathfromURI(const t_serveur &server, HTTPRequest &requestHTTP, const t_location &current_location);
//void        						handleGet(const t_serveur &server, t_client &client, const t_location &current_location);
void								ft_configFileparsing(std::ifstream &configFile, std::vector<t_serveur> &data);
void								readRequest(t_globalData &dataStruct, t_client &clients);
void								accept_new_connection(std::vector<t_serveur>& servers, int listener_socket, fd_set *all_sockets, int *fd_max, std::map<int, t_client> &clients);
void								parsHttpBody(char buffer[], HTTPRequest& requestHTTP, int header_size, int byte_read);
int									hex_to_int(const std::string& hex_str);
void								printDataConfig(const std::vector<t_serveur>& data_config);
bool								checkBodyNotExist(char buffer[]);
bool								checkHttpBodySize(char buffer[], HTTPRequest& requestHTTP);
bool								checkIfHttpRequetteIsComplet(char buffer[], int byteRad);
void								getrequestlocation(std::vector<t_serveur> &server_vec, HTTPRequest &requestHTTP);
int									server_socket_creation(t_serveur *data_config);
void								waiting_connection(std::vector<t_serveur>& data_config);
void        						GetResponse(t_client &client, const std::string& path);
void								getCGIresponse(HTTPRequest &requestHTTP, t_location const &current_location, t_globalData &dataStruct);
std::string							getBodyContentLength(char buffer[], int contentLength);
int									hex_to_int(const std::string& hex_str);
void								printHttpRequest(HTTPRequest requestHTTP);
void								checkHttpFirstLine(HTTPRequest requestHTTP);
bool								checkHeadersHostExist(HTTPRequest requestHTTP);
std::string							getBodyTransferEncoding(char buffer[]);
void								printRequestPreParsing(char buffer[]);
bool								bodyNeededCheck(HTTPRequest requestHTTP);
std::string							formatErrorPage(std::string code, const std::string& message);
void								init_formdata(t_formdata data);
void								globalDataStructInit(t_globalData &dataStruct, std::vector<t_serveur>& servers);
int									checkNewConnection(std::vector<t_serveur>& servers, int j);
void								cleanDoneClient(std::map<int, t_client> &clients);
size_t								getSizeToSend(t_client &client);
void								writeRequest(t_client &client, t_globalData &dataStruct);
bool								checkValideRequestHost(HTTPRequest requestHTTP, t_client &client);
HTTPResponse						sendErrorResponseDelete(const char *str);
void								sighandler_init(void);
HTTPResponse 						sendErrorResponseParsing(const char *str);
time_t								get_time_in_ms(void);
void								check_client_timeout(t_globalData &dataStruct);
void								sighandler_child(void);
void								freeall(t_globalData &dataStruct);
std::string							urlencoded_parsing_a_la_loupe(const std::string& str);
std::string error_404_mgnt(std::string path);
