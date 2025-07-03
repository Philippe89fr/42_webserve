/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestFullyReceivedCheck.cpp                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prambaud <prambaud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 14:29:15 by prambaud          #+#    #+#             */
/*   Updated: 2025/05/27 13:45:51 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

// check si le body a ete envoye en entier
bool checkBodyTransferEncodingComplete(std::vector<char> body_vec)
{
    // Vérifier que le body a au moins 5 caractères
    if (body_vec.size() < 5) {
        return false;
    }
    if (body_vec[body_vec.size() - 1] == '\n' && (body_vec[body_vec.size() - 2] == '\r')
        && (body_vec[body_vec.size() - 3] == '\n') && (body_vec[body_vec.size() - 4] == '\r')
        && (body_vec[body_vec.size() - 5] == '0'))
        return (true);
    return (false);
}

// check si le body a ete envoye en entier pour ContentLenght
bool checkBodyTransferContLenght(std::map<std::string, std::string>::const_iterator it, int contentLength, int BodySize )
{
    std::istringstream iss(it->second);
    iss >> contentLength; //conversion en int
    if (contentLength > BodySize)
        return (false);
    return (true);
}

/*
4 checks differents pour voir si le body est coherent:
1. si il y a un contentLength : verifier que le body fait la bonne taille
2. si il y a un Transfer-Encoding : verifier qu'il est valide
4. si rien de ces 3 headers, pas de body autorise. On verifie donc qu'il n'y ait pas de body.
*/

// il y a un body, on regarde si on l a recu entierement
bool HttpBodyIsComplete(char buffer[], HTTPRequest& requestHTTP, int byte_read, int header_size) {
    int contentLength = 0;
    std::vector<char> body_vec;

    //trouver le nombre d'octet avant le buffer
    //body.insert(body.end(), buffer + header_octet, buffer + byte_size - header_octet);
 
    body_vec.insert(body_vec.end(), buffer + header_size, buffer + byte_read);
    requestHTTP.setbodyVec(body_vec);
    std::map<std::string, std::string>::const_iterator it = requestHTTP.getHeader().find("Content-Length");
    if(it != requestHTTP.getHeader().end())//on verifie la condition 1.
    {
        if(checkBodyTransferContLenght(it, contentLength, byte_read - header_size))
            return(true);
        return(false);
    }
    it = requestHTTP.getHeader().find("Transfer-Encoding");
    if (it != requestHTTP.getHeader().end())//on verifie la condition 2.
    {
        if(checkBodyTransferEncodingComplete(body_vec))
            return(true);
        return(false);
    }
    return false; // car aucune des 4 conditions possibles n'est valide.
}

// est ce qu on a besoin d un body pour valider que la requette est complete
bool bodyNeededCheck(HTTPRequest requestHTTP) {
    std::map<std::string, std::string> headers = requestHTTP.getHeader();
    if (!(headers.count("Content-Length")) && !headers.count("Transfer-Encoding"))
            return(false);
    if ((headers["Content-Length"]) == "0")
            return(false);
    return true;
}

// Déterminer quand la requête est complète (généralement en cherchant "\r\n\r\n" pour la fin des en-têtes,
// puis en vérifiant la présence et la longueur du corps si Content-Length est spécifié)
bool checkIfHttpRequetteIsComplet(char buffer[], int byte_size) {
    HTTPRequest requestHTTP;
    std::string line;
    std::string word;
    int header_size(0);
    std::istringstream messageHttp(buffer);

    std::getline(messageHttp, line); //premiere ligne
    header_size += line.size() + 1;
    std::istringstream flux(line);
    flux >> word;
    requestHTTP.setMethod(word);
    flux >> word;
    requestHTTP.setUri(word);
    flux >> word;
    requestHTTP.setVersion(word);
    while(std::getline(messageHttp, line)) { //Headers
        size_t posDeuxPoints = line.find(":");
        size_t posfin = line.find("\r");
        header_size += line.size() + 1;

        if (line == "\r" || line.empty()) { // A REVOIR CAR SI PAS DE BODDY CA VA POSER PROBLEME
            break;
        } // si last line quitte pour eviter throw

        if (posDeuxPoints != std::string::npos && posfin != std::string::npos) {
            std::string firstWord = line.substr(0, posDeuxPoints);
            std::string secondWord = line.substr(posDeuxPoints + 1, posfin - posDeuxPoints - 1);

            size_t premierNonEspace = secondWord.find_first_not_of(" \t");
            if (premierNonEspace != std::string::npos) {
                secondWord = secondWord.substr(premierNonEspace); // Supprimer les espaces au début
            }
            requestHTTP.setHeader(firstWord, secondWord);
        }
        else
            return false;
    }
    if (!bodyNeededCheck(requestHTTP)) // on verifie si un body est attendu
        return (true);
    if (HttpBodyIsComplete(buffer, requestHTTP, byte_size, header_size))
        return true;
    else
        return false; // le body n est pas complet
}
