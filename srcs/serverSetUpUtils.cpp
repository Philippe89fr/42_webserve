/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverSetUpUtils.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 15:26:07 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:26:51 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//Ce programme check la lecture il faudrait aussi checker l ecriture.

/* Dans ce code il n'y a pas de demande de connection cote client (on utilise pas connect / getaddrinfo), il faut le simuler.
Dans d’autres terminaux, nous pouvons utiliser ' nc localhost 4243' pour tenter de se connecter en TCP au port 4243 sur ta machine locale.
pour nous connecter à l’adresse et au port du serveur (ici, localhost port 4242) et
y envoyer des messages qui devront apparaître chez les autres clients connectés :*/


//cette fonction permet, a partir d'une socket, de trouver a quel PORT elle est liee.

int get_serveur_port_number(int j, std::vector<t_serveur>& servers)
{
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    int Port;

    memset(&server_addr, 0, sizeof(server_addr));
    getsockname(j, (sockaddr*)&server_addr, &addr_len);
    Port = ntohs(server_addr.sin_port);
    for (size_t i = 0; i < servers.size(); ++i)
    {
        if (Port == atoi(servers[i].port.c_str()))
            return ((int)i);
    }
    return (-1);
}

void serveurNonBlocking(int socket_fd)
{
    int	reuseaddr = 1;
    int	flags = fcntl(socket_fd, F_GETFL, 0); // Retrieve the current flag of the socket
	if (flags == -1) {
		std::cerr << "Error fcntl(F_GETFL)" << std::endl;
	}
	if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) { // Add the O_NONBLOCK flag (becomes non-blocking)
		std::cerr << "Error fcntl(F_SETFL)" << std::endl;
	}
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)); //rend la socket reutilisable
    //setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuseaddr, sizeof(int)); //rend le PORT reutilisable
}

void globalDataStructInit(t_globalData &dataStruct, std::vector<t_serveur>& servers) {
    dataStruct.d_servers = &servers;
    dataStruct.d_fdMax = 0;
    FD_ZERO(&dataStruct.d_all_read_sockets); // initialisation du set
    FD_ZERO(&dataStruct.d_all_write_sockets);
    FD_ZERO(&dataStruct.d_read_fds);
    FD_ZERO(&dataStruct.d_write_fds);
    // Préparation des ensembles de sockets pour select()
    for (size_t i = 0; i < servers.size(); ++i)
    {
        FD_SET(servers[i].socket, &dataStruct.d_all_read_sockets); // Ajout de les serveurs à l'ensemble
        dataStruct.d_fdMax = std::max(dataStruct.d_fdMax, servers[i].socket) ; // Le descripteur le plus grand est forcément celui de notre seule socket
    }
//	std::cout << "[Server] Set up select fd sets" << std::endl;
}

int checkNewConnection(std::vector<t_serveur>& servers, int j)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        if (j == servers[i].socket)
            return (i);
    }
    return (-1);
}

void removeSocketUpdateMax(int socket, t_globalData &dataStruct) {
    std::map<int, t_client>::iterator	it;
    int									max_fd = 0;

    if (socket == dataStruct.d_fdMax)
    {
        for(it = dataStruct.d_clients.begin();  it != dataStruct.d_clients.end(); it++)
        {
			if (it->second.fd > max_fd)
				max_fd = it->second.fd;
        }
        dataStruct.d_fdMax = max_fd;
    }
}

//Ici on cherche le nombre d'octet a envoyer a la socket (CHUNKSEND si il reste au moins 25000 octet a envoyer, le nombre d'octet restant sinon)
size_t getSizeToSend(t_client &client)
{
    if(client.response.getbitsSent() + CHUNKSEND < client.response.toVectorChar().size())
        return CHUNKSEND;
    else
        return client.response.toVectorChar().size() - client.response.getbitsSent();
}

//On boucle sur tous les clients. Pour ceux qui ont finit leur process, on les reset, on ferme leur socket.
void cleanDoneClient(std::map<int, t_client> &clients)
{
    std::map<int, t_client>::iterator	it = clients.begin();

	while(it != clients.end())
	{
		if (it->second.statut == DONE)
        {
            it->second.size = 0;
            delete[] it->second.buffer;
            it->second.buffer = NULL;
            it->second.responseReady = false;
            it->second.request.resetRequest();
            it->second.response.setbitsRead(0);
            it->second.response.setbitsSent(0);
            it->second.response.setbody_char_bool(false);
            std::cout << "[Server] Socket " << it->first << " closed." << std::endl;
            close(it->second.fd); // Ferme la socket
			clients.erase(it);
			it = clients.begin();
        }
		else
			++it;
	}
}
