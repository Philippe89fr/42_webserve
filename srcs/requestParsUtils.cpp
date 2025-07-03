/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestParsUtils.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 13:57:21 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 15:46:55 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

std::string getBodyContentLength(char buffer[], int contentLength)
{
    std::istringstream messageHttp(buffer);
    std::string line;
    char* bodyBuffer = new char[contentLength + 1]; // new char de pile la taille que l'on souhaite
    while(std::getline(messageHttp, line)) { // On va jusqu au body
    // Supprimer le \r à la fin si présent
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        // Si la ligne est vide, c'est la fin des en-têtes
        if(line.empty()) {
            break;
        }
    }
    messageHttp.read(bodyBuffer, contentLength);
    bodyBuffer[contentLength] = '\0';
    std::string body(bodyBuffer, contentLength); // commence au pointeur boddyBuffer et prends contentLength bytes
    delete[] bodyBuffer;
    return body;
}

int hex_to_int(const std::string& hex_str) {
    int result;
    std::stringstream ss;

    ss << std::hex << hex_str;
    ss >> result;

    //if (ss.fail())
    //    throw std::invalid_argument("Invalid hexadecimal number: " + hex_str);

    return result;
}

//void printHttpRequest(HTTPRequest requestHTTP)
//{
//    std::cout << std::endl << std::endl << "----------------------- REQUEST START --------------------------" << std::endl;
//    std::cout << "Method = " << requestHTTP.getMethod() << std::endl << "URI = " << requestHTTP.getUri() << std::endl << "Version = " << requestHTTP.getVersion() << std::endl;
//    std::map<std::string, std::string>::const_iterator it;
//    for (it = requestHTTP.getHeader().begin(); it != requestHTTP.getHeader().end(); ++it) {
//        std::cout << it->first << ": " << it->second << std::endl;
//    }
//    std::cout << "Body = " << requestHTTP.getBody().substr(0, 200) << std::endl;
//    std::cout << std::endl << "----------------------------- REQUEST END -----------------------------" << std::endl << std::endl;
//}

// check si format premiere ligne est OK
void checkHttpFirstLine(HTTPRequest requestHTTP) {
    if(requestHTTP.getMethod() != "GET" && requestHTTP.getMethod() != "DELETE" && requestHTTP.getMethod() != "POST" )
	{
		std::string	err_msg("Error 501 : Not Implemented");
        throw HttpError(err_msg);
	}
    if(requestHTTP.getUri()[0] != '/')
	{
		std::string	err_msg("Error 400 : Request line issue: Wrong URI, no '/'");
        throw HttpError(err_msg);
	}
    if(requestHTTP.getUri().length() > 2048)
	{
		std::string	err_msg("Error 414 : URI Too Long");
        throw HttpError(err_msg);
	}
    if(requestHTTP.getVersion() != "HTTP/1.0" && requestHTTP.getVersion() != "HTTP/1.1")
	{
		std::string	err_msg("Error 505 : HTTP Version Not Supported");
        throw HttpError(err_msg);
	}
}

bool checkValideRequestHost(HTTPRequest requestHTTP, t_client &client)
{
    std::map<std::string, std::string>::const_iterator it;
    it = requestHTTP.getHeader().find("Host");
    size_t i(0);
    while (i < client.serversPort.size())
    {
        std::string hostName;
        hostName = it->second.substr(0, it->second.find(":"));
//        std::cout << "Host request : " <<  hostName << std::endl;
//        std::cout << "client.serversPort[i].serverName : " <<  client.serversPort[i].serverName << std::endl;
        if(hostName == client.serversPort[i].serverName)
            return (true);
        if(hostName == "localhost")
            return (true);
        if(hostName == client.serversPort[i].adressIP)
            return (true);
        i++;
    }
    return (false);
}

//check si Header Host exist car obligatoir
bool checkHeadersHostExist(HTTPRequest requestHTTP) {
   std::map<std::string, std::string>::const_iterator it;
    it = requestHTTP.getHeader().find("Host");
    if(it != requestHTTP.getHeader().end()) {
            return true;
    }
    return false;
}

std::string getBodyTransferEncoding(char buffer[])
{
    std::istringstream messageHttp(buffer);
    std::string line;
    std::string hex_line;
    int nb_octet;
    size_t fin_hex_line;
    while(std::getline(messageHttp, line)) { // On va jusqu au body
    // Supprimer le \r à la fin si présent
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        // Si la ligne est vide, c'est la fin des en-têtes
        if(line.empty()) {
            break;
        }
    }
    std::string body;
    while(std::getline(messageHttp, hex_line))
    {
        fin_hex_line = hex_line.find_first_of(';');//fin_hex_line = on recupere la partie qui renseigne, en hexadecimal
        if ((int)fin_hex_line == -1)//le nombre d'octet a lire. Cette partie se finit soit par ';' soit par "\r\n" et donc '\r'.
            fin_hex_line = hex_line.find_first_of('\r');
        hex_line = hex_line.substr(0, fin_hex_line);//on recupere la partie de la string voulue (la partie en hexa)
        nb_octet = hex_to_int(hex_line);//on converti en int
        char *buf = new char[nb_octet];//on cree une string de la bonne taille (sans le /0, car on va ensuite concatener les chaines).
        messageHttp.read(buf, nb_octet);//on lit notre bon nombre d'octet
        std::string body1(buf, nb_octet);//on met ce qu'on vient de lire dans body1
        body = body + body1;//on ajoute ce qu'on vient de lire au reste du body
        delete[] buf;
        std::getline(messageHttp, line);//on finit la ligne pour mettre le curseur au debut de la ligne suivante.
    }
    body = body + '\0';//ajout du /0
    return body;
}

void printRequestPreParsing(char buffer[])
{
   std::cout << "\n---- print_REQUEST_PreParsing ----\n" << std::endl;
   std::istringstream messageHttp(buffer);
   std::string line;
   while (std::getline(messageHttp, line)) {
       std::cout << line << std::endl;
   }
   std::cout << "---------------END printRequestPreParsing-----------------" << std::endl;
}
