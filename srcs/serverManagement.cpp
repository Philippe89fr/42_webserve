/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverManagement.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/20 15:55:31 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 09:05:49 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

// trouver les servers qui ont le bon port
std::vector<t_serveur> findServerWithPort(std::vector<t_serveur>& servers, std::string port) {
    std::vector<t_serveur>::iterator it;
    std::vector<t_serveur> serversPort;
    for(it = servers.begin(); it != servers.end(); it++) {
//        std::cout << "port : " << port << std::endl;
//        std::cout << "it->port :" << it->port << std::endl;
        if(it->port == port)
        {
            serversPort.push_back(*it);
        }
    }
    return serversPort;
}

void closeClienServerConnection(int socket_client, t_globalData &dataStruct)
{
    if (FD_ISSET(socket_client, &dataStruct.d_all_read_sockets))
        FD_CLR(socket_client, &dataStruct.d_all_read_sockets);
    if (FD_ISSET(socket_client, &dataStruct.d_all_write_sockets))
        FD_CLR(socket_client, &dataStruct.d_all_write_sockets);
    removeSocketUpdateMax(socket_client, dataStruct);
    dataStruct.d_clients[socket_client].statut = DONE; // cleanDoneClient() function va tout cleaner
}

void nonBlockingRead(t_globalData &dataStruct, t_client &client) {
    if(checkIfHttpRequetteIsComplet(client.buffer, client.size))
    {
        try {
            client.request = parsHttpRequest(client);
            FD_CLR(client.fd, &dataStruct.d_all_read_sockets);
            client.statut = PROCESSING;
        }
        catch (const HttpError& e) {
            FD_CLR(client.fd, &dataStruct.d_all_read_sockets);//on ferme la socket lecture
            client.response = sendErrorResponseParsing(e.what());//on recupere la reponse erreur
            client.responseReady = true;//la reponse est prete a etre envoyer
            client.statut = WRITING;//on met directement le statut en writing pour envoyer la reponse
            FD_SET(client.fd, &dataStruct.d_all_write_sockets);//je set la socket en mode ecriture
        }
    }
}

void addSocketUpdateMax(int socket, t_globalData &dataStruct) {
    if (socket > dataStruct.d_fdMax)
        dataStruct.d_fdMax = socket;
}

// Lit le message d'une socket de maniere non bloquante (par petit bout) et repond
void readRequest(t_globalData &dataStruct, t_client &client) {
    char buffer[CHUNKLE];
    long bytes_read;

    // initialisation si pas deja faite
    t_serveur server = client.serversPort[0];

    memset(&buffer, '\0', sizeof buffer);
    if (client.buffer == NULL) {
        client.buffer = new char[server.client_max_body_size + 1]; // +1 pour le caractère nul
        client.size = 0;
        client.buffer[0] = '\0';
    }
    bytes_read = recv(client.fd, buffer, CHUNKLE, 0); //Recevoir des données en provenance de cette socket.
    if((long) client.size + bytes_read > (long) server.client_max_body_size) {
        FD_CLR(client.fd, &dataStruct.d_all_read_sockets);//on ferme la socket lecture
        client.response = sendErrorResponseParsing("Error 413 : Request Entity Too Large\n");//on recupere la reponse erreur
        client.responseReady = true;//la reponse est prete a etre envoyer
        client.statut = WRITING;//on met directement le statut en writing pour envoyer la reponse
        FD_SET(client.fd, &dataStruct.d_all_write_sockets);//je set la socket en mode ecriture
        return;
    }
    std::memcpy(client.buffer + client.size, buffer , bytes_read); // ajoute buffer dans socket.buffer
    client.size += bytes_read; // on augmente le compteur de bytes
    client.buffer[client.size] = '\0'; // on met un \0 a la fin
    // En locurence on capte ce qui est tape dans le terminal client. recv() agit comme un read() réseau : il lit les données envoyées par le client connecté.

    if (bytes_read == 0) {
		std::cout << "[Server] Closed connection with socket " << client.fd << std::endl;
        closeClienServerConnection(client.fd, dataStruct);
        return;
    }
    else if (bytes_read < 0) { //erreure de lecture
		std::cerr << "[Server] Recv error: " << std::endl;
        closeClienServerConnection(client.fd, dataStruct);
        return;
    }
    else
        nonBlockingRead(dataStruct, client);
}

time_t	get_time_in_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void check_client_timeout(t_globalData &dataStruct)
{
    std::map<int, t_client>& clients = dataStruct.d_clients;
    std::map<int, t_client>::iterator it;
    for(it = clients.begin();  it != clients.end(); it++)
    {
        if (it->second.statut == READING && (get_time_in_ms() - it->second.time_start) / 1000 > 300)
        {
            FD_CLR(it->second.fd, &dataStruct.d_all_read_sockets);//on ferme la socket lecture
            it->second.response = sendErrorResponseParsing("Error 408 : Request Timeout");//on recupere la reponse erreur
            it->second.responseReady = true;//la reponse est prete a etre envoyer
            it->second.statut = WRITING;//on met directement le statut en writing pour envoyer la reponse
            FD_SET(it->second.fd, &dataStruct.d_all_write_sockets);//je set la socket en mode ecriture
        }
    }
}

// On va accepter une nouvelle demande de connection entre notre serveur et une socket client et la rajouter a la all_sockets
//  l’événement "prête à lire" pour un server, signifie qu’il y a au moins une connexion entrante en attente dans la file d’écoute.
void accept_new_connection(std::vector<t_serveur>& servers, int server_socket, fd_set *all_read_sockets, int *fd_max, std::map<int, t_client>& clients) {
    int client_fd;
    t_client client;

    client_fd = accept(server_socket, NULL, NULL); // on sait qu il y a une demande client en attente donc on va recup le bon fd
    if (client_fd == -1) {
		std::cerr << "[Server] Accept error: " << strerror(errno) << std::endl;
        return ;
    }
//    std::cout << "on accepte le client " << client_fd << "\n";
    serveurNonBlocking(client_fd);
    client.fd = client_fd;
    client.time_start = get_time_in_ms();
    client.size = 0;
    client.buffer = NULL;
    client.statut = READING;
    client.responseReady = false;
    client.datasBuffer.bodyVecBitsRead = 0;
    int serverNumber = get_serveur_port_number(client_fd, servers); // choper le bon socket_serveur qui ecoute ce port
    client.serversPort = findServerWithPort(servers, servers[serverNumber].port); // on trouve tous les serveurs qui utilisent ce Port
    init_formdata(client.datasBuffer);
    clients[client_fd] = client;
    FD_SET(client_fd, all_read_sockets); // Ajoute la socket client à l'ensemble
    if (client_fd > *fd_max) {
        *fd_max = client_fd; // Met à jour la plus grande socket
    }
}

//On sent la request en chunked.
void writeRequest(t_client &client, t_globalData &dataStruct)
{
    size_t	sizeToSent;
    int		actualBytesSent;
    size_t	bitSent;

    if (!client.response.getbody_char_bool()) //Si la response est au format std::string -> on la transforme en formet std::vector<char> et on met le booleen a true.
    {
        client.response.setBodyToBodyChar();
        client.response.setbody_char_bool(true);
    }
    sizeToSent = getSizeToSend(client);
    actualBytesSent = send(client.fd, &client.response.toVectorChar()[0] + client.response.getbitsSent(), sizeToSent, 0);
    if (actualBytesSent == -1) {//cas d'erreur de send... On enleve la socket des writes, on met le client au statut DONE ce qui fermera cette socket.
        std::cerr << "[Server] Send error to client " << client.fd << "\n";
        client.statut = DONE;
        client.responseReady = false;
        FD_CLR(client.fd, &dataStruct.d_all_write_sockets);
        removeSocketUpdateMax(client.fd, dataStruct);
        return ;
    }
    else if (actualBytesSent == 0) {//cas d'erreur de send... On enleve la socket des writes, on met le client au statut DONE ce qui fermera cette socket.
        client.statut = DONE;
        client.responseReady = false;
        FD_CLR(client.fd, &dataStruct.d_all_write_sockets);
        removeSocketUpdateMax(client.fd, dataStruct);
        return ;
    }
    bitSent = client.response.getbitsSent() + actualBytesSent;//bitSent -> totalite des bits sent so far
    client.response.setbitsSent(bitSent);
    if(actualBytesSent < CHUNKSEND || client.response.getbitsSent() == client.response.toVectorChar().size()) {//si on a envoyer moins de 25000 octet, c'est qu'on a atteint la fin de la response //
        client.statut = DONE;
        client.responseReady = false;
        FD_CLR(client.fd, &dataStruct.d_all_write_sockets);
        removeSocketUpdateMax(client.fd, dataStruct);
    }
}
