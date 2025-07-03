/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 17:38:13 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:36:59 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

/*
Format de la réponse HTTP

Réponse en cas de succès (200 OK)::
"HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 30
Date: Wed, 21 May 2025 12:34:56 GMT
Connection: keep-alive

Fichier supprimé avec succès"

Réponse en cas d'erreur (404 Not Found):
"HTTP/1.1 404 Not Found
Content-Type: text/plain
Content-Length: 19
Date: Wed, 21 May 2025 12:34:56 GMT
Connection: keep-alive

Fichier non trouvé"

Réponse en cas d'erreur serveur (500 Internal Server Error) //
"HTTP/1.1 500 Internal Server Error
Content-Type: text/plain
Content-Length: 41
Date: Wed, 21 May 2025 12:34:56 GMT
Connection: keep-alive

Erreur lors de la suppression du fichier"
*/

void delete_requested_file(HTTPRequest &requestHTTP) {
	std::string	path = urlencoded_parsing_a_la_loupe(requestHTTP.getUriPath());

	if (access(path.c_str(), F_OK))
	{
		std::string	err_msg("Error 404 : Resource not found");
        throw HttpError(err_msg);
	}
	if (access(path.c_str(), W_OK) || std::remove(path.c_str()))
	{
		std::string	err_msg("Error 403 : No permission to delete this resource");
        throw HttpError(err_msg);
	}
}

std::string formatResponseBodyDelete(std::string fileName) {
    std::string body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>Supression";
    body += " réussie</title>\n</head>\n<body>\n\t<h1>Fichier supprimé avec succes";
    body += "</h1>\n\t<p>Merci pour votre supression du fichier ";
    body += urlencoded_parsing_a_la_loupe(fileName.substr(9,fileName.size()));
    body += "</p>\n\t<p><a href=\"/\">Retour à l'accueil</a></p>\n</body>\n</html>";
    return(body);
}

std::string error_404_mgnt(std::string path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
    std::cerr << "ERREUR: Fichier non trouvé: " << path << std::endl;
    return "";}
    std::string body;
    std::string line;
    while(std::getline(file, line))
    {
            body += line;
            body += "\n";
    }
    file.close();
    return(body);
}

HTTPResponse sendErrorResponseDelete(const char *str)
{
    HTTPResponse	response;
    std::string word;
    std::string body;

    std::istringstream flux(str);
    flux >> word;
    flux >> word;

    response.setHeader("Server", "Webserv/1.0");
    response.setHeader("Date", getHTTPDate());
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.setHeader("Connection", "keep-alive");
    if (word == "500")
    {
        response.setStatus(500, "Internal Server Error");
        body = formatErrorPage(word, "Error : Unexpected system error occured");
    }
    if (word == "403")
    {
        response.setStatus(403, "No permission");
        body = formatErrorPage(word, "Error : You do not have permission to delete this resource");
    }
    if (word == "404")
    {
        response.setStatus(404, "Resource not found");
        body = error_404_mgnt("./www/webservSite/error_404.html");
    }
    size_t value = body.length();
    std::ostringstream oss;
    oss << value;
    response.setHeader("Content-Length", oss.str());
    response.setBody(body);
    return (response);
}

HTTPResponse handleDelete(HTTPRequest &requestHTTP)
{
	HTTPResponse						response;
	std::string							fileName = requestHTTP.getUri();
    std::map<std::string, std::string>	headers = requestHTTP.getHeader();

    response.setBody(formatResponseBodyDelete(fileName));
    response.setStatus(200, "OK");
    response.setHeader("Server", "Webserv/1.0");
    response.setHeader("Date", getHTTPDate());
    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    if (headers["Connection"] == "keep-alive" || (requestHTTP.getVersion() == "HTTP/1.1" && headers["Connection"] != "close"))
        response.setHeader("Connection", "keep-alive");
    else if (headers["Connection"] == "close" || requestHTTP.getVersion() == "HTTP/1.0")
        response.setHeader("Connection", "close");
    size_t bodySize = formatResponseBodyDelete(fileName).length();
    std::ostringstream oss;
    oss << bodySize;
    response.setHeader("Content-Length", oss.str());
    try {
        delete_requested_file(requestHTTP);
    }
    catch (const HttpError& e) {
        response = sendErrorResponseDelete(e.what());
    }
	return (response);
}
