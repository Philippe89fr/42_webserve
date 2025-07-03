/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   requestParsing.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 13:57:21 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:29:56 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

/*
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
    }*/

HTTPResponse sendErrorResponseParsing(const char *str)
{
    HTTPResponse	response;
    std::string		word;
    std::string		body;

    std::istringstream flux(str);
    flux >> word;
    flux >> word;

    response.setHeader("Server", "Webserv/1.0");
    response.setHeader("Date", getHTTPDate());
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.setHeader("Connection", "keep-alive");
    if (word == "501")
    {
        response.setStatus(501, "Not Implemented");
        body = formatErrorPage(word, "Error : Method not implemented");
    }
    if (word == "400")
    {
        response.setStatus(400, "Bad Request");
        body = formatErrorPage(word, "Error : Bad Request");
    }
    if (word == "414")
    {
        response.setStatus(414, "URI Too Long");
        body = formatErrorPage(word, "Error : URI Too Long");
    }
    if (word == "505")
    {
        response.setStatus(505, "HTTP Version Not Supported");
        body = formatErrorPage(word, "Error : HTTP Version Not Supported");
    }
    if (word == "411")
    {
        response.setStatus(411, "Length required");
        body = formatErrorPage(word, "Error : Length required in Request");
    }
    if (word == "413")
    {
        response.setStatus(413, "Payload Too Large");
        body = formatErrorPage(word, "Error : Request Entity Too Large");
    }
    if (word == "408")
    {
        response.setStatus(408, "Request Timeout");
        body = formatErrorPage(word, "Error : Request Timeout");
    }
    size_t value = body.length();
    std::ostringstream oss;
    oss << value;
    response.setHeader("Content-Length", oss.str());
    response.setBody(body);
    return (response);
}

std::vector<char> checkBodyTransferEncodingValide(std::vector<char> bodyVec)
{
    std::vector<char> bodyVec2;
    size_t i(0);
    std::string number;
    size_t nb_octet;
    while (i < bodyVec.size())
    {
        number = "";
        while (i < bodyVec.size() && bodyVec[i] != '\r')
        {
            number += bodyVec[i];
            i++;
        }
        nb_octet = hex_to_int(number);
        if (nb_octet == 0)
        {
            if (i + 1 >= bodyVec.size() || bodyVec[i] != '\r' || bodyVec[i + 1] != '\n')
			{
				std::string	err_msg("Error 400 : invalid body format");
                throw HttpError(err_msg);
			}
            i += 2;
            if (i + 1 >= bodyVec.size() || bodyVec[i] != '\r' || bodyVec[i + 1] != '\n')
			{
				std::string	err_msg("Error 400 : invalid body format");
                throw HttpError(err_msg);
			}
            return (bodyVec2);
        }
        while (i < bodyVec.size() && bodyVec[i] != '\n')
            i++;
        i++;
        while (i < bodyVec.size() && nb_octet)
        {
            bodyVec2.push_back(bodyVec[i]);
            i++;
            nb_octet--;
        }
        if (i + 1 >= bodyVec.size() || bodyVec[i] != '\r' || bodyVec[i + 1] != '\n')
		{
			std::string	err_msg("Error 400 : invalid body format");
            throw HttpError(err_msg);
		}
        i++;
        i++;
    }
    return (bodyVec2);
}

bool checkHttpBodyValide(HTTPRequest& requestHTTP) {
    std::vector<char> bodyVec = requestHTTP.getbodyVec();
    std::map<std::string, std::string> headers = requestHTTP.getHeader();
    if (!bodyNeededCheck(requestHTTP)) // on verifie si un body est attendu
    {
        if (bodyVec.size() > 0)//pas besoin d'un body, mais body
            return (false);//alors c'est faux
        return (true);
    }
    if (bodyVec.size() > 0)//s'il y a un body sans content-length ni transfer-encoding ni Connection=close, body pas valide.
    {
        if (!(headers.count("Content-Length")) && !headers.count("Transfer-Encoding") && headers["Connection"] != "close")
		{
			std::string	err_msg("Error 411 : Length required");
            throw HttpError(err_msg);
		}
    }
    if (headers["Transfer-Encoding"] == "chunked")//Pour Content-Length, la verification a deja ete faites lors du check full message sent.
    {
        requestHTTP.setbodyVec(bodyVec);
            return true;
    }
    return true;
}

// Debut parsing http
void checkHTTPelements(HTTPRequest requestHTTP, t_client &client) {
    checkHttpFirstLine(requestHTTP);
    if(!checkHeadersHostExist(requestHTTP))
	{
		std::string	err_msg("Error 400 : Header issue in HTTP request: missing Host");
        throw HttpError(err_msg);
	}
    if (!checkValideRequestHost(requestHTTP, client))
	{
		std::string	err_msg("Error 400 : Request Host not matching with server name");
        throw HttpError(err_msg);
	}
}

void parsHttpBody(char buffer[], HTTPRequest& requestHTTP, int header_size, int byte_read)
{
    std::vector<char> body_vec;
    body_vec.insert(body_vec.end(), buffer + header_size, buffer + byte_read);

    requestHTTP.setbodyVec(body_vec);

    std::string body;
    int contentLength;
    std::map<std::string, std::string>::const_iterator it = requestHTTP.getHeader().find("Content-Length");

    if(it != requestHTTP.getHeader().end())
    {
        std::istringstream iss(it->second);
        iss >> contentLength; //conversion en int
        body = getBodyContentLength(buffer, contentLength);
        requestHTTP.setBody(body);
    }
    else
    {
        it = requestHTTP.getHeader().find("Transfer-Encoding");
        if(it != requestHTTP.getHeader().end() && it->second == "chunked")
        {
            body = getBodyTransferEncoding(buffer);
            requestHTTP.setBody(body);
        }
        else
        {
            requestHTTP.setBody("");
        }
    }
}

size_t parsHttpFirstLineAndHeaders(HTTPRequest &requestHTTP, char buffer[]) {
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
		{
			std::string	err_msg("Error 400 : Header issue in HTTP request");
            throw HttpError(err_msg);
		}
    }
    return header_size;
}

HTTPRequest parsHttpRequest(t_client &client) {
    HTTPRequest requestHTTP;
    int header_size(0);

    //printRequestPreParsing(client.buffer);
    header_size = parsHttpFirstLineAndHeaders(requestHTTP, client.buffer); // recupere header et body
    parsHttpBody(client.buffer, requestHTTP, header_size, client.size);
    if(!checkHttpBodyValide(requestHTTP))
	{
		std::string	err_msg("Error 400 : invalid request format");
        throw HttpError(err_msg);
	}
    //printHttpRequest(requestHTTP);
//    std::cout << "jjjjj" << std::endl;
    checkHTTPelements(requestHTTP, client); // check pour voir si elle est au bon format
    return (requestHTTP);
}
