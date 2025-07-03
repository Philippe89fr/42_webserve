/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverSetUp.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prambaud <prambaud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 15:26:07 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:26:22 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//Ce programme check la lecture il faudrait aussi checker l ecriture.

/* Dans ce code il n'y a pas de demande de connection cote client (on utilise pas connect / getaddrinfo), il faut le simuler.
Dans d’autres terminaux, nous pouvons utiliser ' nc localhost 4243' pour tenter de se connecter en TCP au port 4243 sur ta machine locale.
pour nous connecter à l’adresse et au port du serveur (ici, localhost port 4242) et
y envoyer des messages qui devront apparaître chez les autres clients connectés :*/

// creation du socket serveur

int	g_sig_errno = 0;

int server_socket_creation(t_serveur *data_config) {
    struct sockaddr_in	sa;
    int					socket_fd;
    int					status;

     // Préparaton de l'adresse et du port pour la socket de notre serveur
     memset(&sa, 0, sizeof sa); // initialisation a zero de la structure
     sa.sin_family = AF_INET; // IPv4
     sa.sin_addr.s_addr = INADDR_ANY; //htonl(std::atoi(data_config->adressIP.c_str())); // Adress IP: 127.0.0.1, localhost
     sa.sin_port = htons(std::atoi(data_config->port.c_str()));
     data_config->sockaddr_in = sa;
     // Convertir string IP vers format binaire
    if (inet_pton(AF_INET, data_config->adressIP.c_str(), &sa.sin_addr) <= 0) {
        perror("inet_pton error");
        return -1;
        }

	// creation de la socket
	socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
	serveurNonBlocking(socket_fd);
	std::cout << "[Server] Created server socket " << socket_fd << std::endl;

    // Liaison de la socket a l'adresse et au PORT
//    std::cout << "tameeeeeeeeeeeeeeeeeeeeeeeeeeeeere" << std::endl;
    status = bind(socket_fd, (struct sockaddr *)&sa, sizeof sa);
    if (status != 0) {
		std::cerr << "[Server] Bind error: " << strerror(errno) << std::endl;
		close (socket_fd);
        return (-1);
    }
//	std::cout << "[server] bound socket to localhost port " << data_config->port << std::endl;
    return (socket_fd);
}

bool processReadyRequests(std::map<int, t_client> &clients, t_globalData &dataStruct)
{
    bool i(0);
    std::map<int, t_client>::iterator it;
    for(it = clients.begin();  it != clients.end(); it++)
    {
        if (it->second.statut == PROCESSING)
        {
            i = 1;
            executeRequest(it->second.serversPort, it->second, dataStruct);
            if (it->second.responseReady)
            {
                FD_SET(it->first, &dataStruct.d_all_write_sockets); // Ajoute la socket client à l'ensemble
                it->second.statut = WRITING;
            }
        }
    }
    return (i);
}

int boucleConnectionStart(t_globalData &dataStruct, bool inProcess)
{
    int status;
    dataStruct.d_read_fds = dataStruct.d_all_read_sockets; // on fait une copie car select va la modifier
    dataStruct.d_write_fds = dataStruct.d_all_write_sockets;
    if (inProcess)
        dataStruct.d_timer.tv_sec = 0;
    else
        dataStruct.d_timer.tv_sec = 1; // Timeout de 2 secondes pour select()
    dataStruct.d_timer.tv_usec = 0;
    status = select(dataStruct.d_fdMax + 1, &dataStruct.d_read_fds, &dataStruct.d_write_fds, NULL, &dataStruct.d_timer);
    if (status == -1) {
		std::cerr << "[Server] Select error: " << strerror(errno) << std::endl;
        exit(1);
    }
    return status;
}

void waiting_connection(std::vector<t_serveur>& servers)
{
	std::cout << "---- SERVER LAUNCHED ----" << std::endl << std::endl;

    // Pour surveiller les sockets clients :
    t_globalData dataStruct;
    globalDataStructInit(dataStruct, servers);
    bool	inProcess(0);
	short	loop_idx = 0;
    //creation des clients/
	try
	{
		while (!g_sig_errno)
		{
			if (!boucleConnectionStart(dataStruct, inProcess))
			{
				// == select vaut 0,
				switch (loop_idx)
				{
					case 0:
						std::cout << "[Server] Waiting.  \r" << std::flush;
						break;
					case 1:
						std::cout << "[Server] Waiting.. \r" << std::flush;
						break;
					case 2:
						std::cout << "[Server] Waiting...\r" << std::flush;
				}
				++loop_idx;
				if (loop_idx == 3)
					loop_idx = 0;
			}
			else
				loop_idx = 0;
			// Boucle sur les sockets pour voir si elles ont ete modifie par select
			for(int j = 0; j <= dataStruct.d_fdMax; j++)
			{
				if (FD_ISSET(j, &dataStruct.d_read_fds) != 1 && FD_ISSET(j, &dataStruct.d_write_fds) != 1)
					continue; //  la socket i n’est pas prête pour la lecture, select n a pas signale d acti dessus
				//std::cout << "dataStruct.d_fdMax :" << dataStruct.d_fdMax << "\n";
				int nb_serv = checkNewConnection(servers, j); //trouve si j est une socket server, sinon, nb_serv vaut -1
				if (FD_ISSET(j, &dataStruct.d_read_fds) == 1 && nb_serv != -1)
				{
					std::cout << "[Server] Receiving request for socket " << j << std::endl;
					accept_new_connection(servers, servers[nb_serv].socket, &dataStruct.d_all_read_sockets, &dataStruct.d_fdMax, dataStruct.d_clients); // va accepter notre socket client
				}
				else if (FD_ISSET(j, &dataStruct.d_read_fds) == 1 && dataStruct.d_clients[j].statut == READING)
				{
					std::cout << "[Server] Reading request from socket " << j << std::endl;
					readRequest(dataStruct, dataStruct.d_clients[j]);
				}
				else if (FD_ISSET(j, &dataStruct.d_write_fds) == 1 && dataStruct.d_clients[j].statut == WRITING)
				{
					std::cout << "[Server] Sending response to socket " << j << std::endl;
					writeRequest(dataStruct.d_clients[j], dataStruct);
				}
			}
			// std::map<int, t_client>::iterator it;
			// for(it = dataStruct.d_clients.begin();  it != dataStruct.d_clients.end(); it++)
			// {
			//     if (it->second.statut == PROCESSING)
			//     {
			//         //std::cout << "Client en process : " << it->second.fd << std::endl;
			//     }
			// }
			check_client_timeout(dataStruct);
			inProcess = processReadyRequests(dataStruct.d_clients, dataStruct);
			cleanDoneClient(dataStruct.d_clients);
		}
	}
	catch(const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
	freeall(dataStruct);
}
