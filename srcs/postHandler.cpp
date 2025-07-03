/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   postHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:15:56 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:34:26 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

std::string urlencoded_parsing_a_la_loupe(const std::string& str)
{
    std::string		decoded;
    char			ch;
    size_t			i;
	unsigned int	ii;

    for (i = 0; i < str.length(); i++)
	{
		if (str[i] == '+')
			decoded += ' ';
		else if (str[i] == '%' && i + 2 < str.length())
		{
			sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			decoded += ch;
			i += 2;
		}
		else
			decoded += str[i];
    }
    return decoded;
}

// parse un body quand il est sous forme urlencoded
std::map<std::string, std::string> parse_urlencoded(const std::vector<char>& body)
{
    std::string bodystr;
    size_t i(0);
    while (i < body.size())
    {
        bodystr += body[i];
        i++;
    }
    std::map<std::string, std::string> result;
    std::istringstream stream(bodystr);
    std::string pair;
    while (std::getline(stream, pair, '&'))
	{
        size_t eq = pair.find('=');
        if (eq != std::string::npos)
		{
            std::string key = urlencoded_parsing_a_la_loupe(pair.substr(0, eq));
            std::string value = urlencoded_parsing_a_la_loupe(pair.substr(eq + 1));
            result[key] = value;
        }
    }
    return result;
}

std::map<std::string, std::string> parse_Json(const std::string& body)
{
    std::map<std::string, std::string> result;
    size_t i = 0;
    while ((i = body.find('"', i)) != std::string::npos)
	{
        size_t keyStart = i + 1;
        size_t keyEnd = body.find('"', keyStart);
        std::string key = body.substr(keyStart, keyEnd - keyStart);

        size_t colon = body.find(':', keyEnd);
        size_t valStart = body.find_first_not_of(" \t\n\"", colon + 1);
        size_t valEnd = body.find_first_of(",}", valStart);
        std::string value = body.substr(valStart, valEnd - valStart - 1);

        result[key] = value;
        i = valEnd;
    }
    return result;
}

std::string formatResponseBody(std::map<std::string, std::string>map) {
    std::string body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>Soumission";
    body += " réussie</title>\n</head>\n<body>\n\t<h1>Formulaire reçu avec succès";
    body += "</h1>\n\t<p>Merci pour votre soumission, ";
    body += map["name"];
    body += "!</p>\n\t<p>Votre commentaire: ";
    body += map["comment"];
    body += "</p>\n\t<p><a href=\"/\">Retour à l'accueil</a></p>\n</body>\n</html>";
    return(body);
}

int PostComment(const t_serveur &server, std::map<std::string, std::string>  bodyMap, const t_location &current_location)
{
    std::string path;
    if (current_location.root == "")
        path = "." + server.root + current_location.path;
    else
        path = "." + current_location.root + current_location.path;
    path += "/" + bodyMap["name"] + ".txt";
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::app);
//    std::cout << path << std::endl;
    if (!file.is_open()) {
		std::string	err_msg("Error 403 : No permission to open the file");
        throw HttpError(err_msg);
    }
    file << bodyMap["comment"];
    file << "\n";
    if (file.fail()) {
		std::string	err_msg("Error 500 : Cannot write in file");
        throw HttpError(err_msg);
    }
    file.close();
    return (0);
}

int PostCommentJson(const t_serveur &server, std::map<std::string, std::string>  bodyMap, const t_location &current_location)
{
    std::string path;
    if (current_location.root == "")
        path = "." + server.root + current_location.path;
    else
        path = "." + current_location.root + current_location.path;
    path += "/Uploads/CommentsJson.txt";
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::app);
//    std::cout << path << std::endl;
    if (!file.is_open()) {
		std::string	err_msg("Error 403 : No permission to open the file");
        throw HttpError(err_msg);
    }
    std::map<std::string, std::string>::iterator it;
    for (it = bodyMap.begin(); it != bodyMap.end(); it++)
    {
        file << it->first;
        file << " : ";
        file << it->second;
        file << "\n";
    }
    if (file.fail()) {
		std::string	err_msg("Error 500 : Cannot write in file");
        throw HttpError(err_msg);
    }
    file.close();
    return (0);
}

void init_formdata(t_formdata data)
{
    data.body.clear();
    data.content_type = "";
    data.name = "";
    data.filename = "";
}

int PostUpload(const t_serveur &server, t_formdata Upload_request, const t_location &current_location)
{
    std::string filename;

    std::string path;
    if (current_location.root == "")
        path = "." + server.root + current_location.path;
    else
        path = "." + current_location.root + current_location.path;
    if (Upload_request.filename == "")
        filename = Upload_request.name;
    else
        filename = Upload_request.filename;
    path += "/" + filename;
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
		std::string	err_msg("Error 403 : No permission to open the file");
        throw HttpError(err_msg);
    }
    size_t dataSize = Upload_request.body.size();
    ssize_t written = write(fd, Upload_request.body.data(), dataSize);
    if (written == -1) {
        close(fd);
		std::string	err_msg("Error 500 : Cannot write in file");
        throw HttpError(err_msg);
    }
    close(fd);
    return (0);
}


// fonction qui permet de savoir si les X dernier caracteres lus sont boundarryyyyyyy LUCAS HAPPPTYYYYYYYY

HTTPResponse sendErrorResponsePost(const char *str)
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
        body = formatErrorPage(word, "Error : You do not have permission to post this resource");
    }
    if (word == "404")
    {
        response.setStatus(404, "Resource not found");
        body = error_404_mgnt("./www/webservSite/error_404.html");
    }
    if (word == "415")
    {
        response.setStatus(415, "Content-Type unknown");
        body = formatErrorPage(word, "Error : Content-Type unknown");
    }
    size_t value = body.length();
    std::ostringstream oss;
    oss << value;
    response.setHeader("Content-Length", oss.str());
    response.setBody(body);
    return (response);
}

std::string formatErrorPage(std::string code, const std::string& message) {
    std::string body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>";
    body += code + " " + message;
    body += "</title>\n</head>\n<body>\n\t<h1>";
    body += code + " - " + message;
    body += "</h1>\n\t<p><a href=\"/\">Retour à l'accueil</a></p>\n</body>\n</html>";
    return body;
}

std::string formatResponseBodyUpload(std::string fileName) {
    std::string body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>Soumission";
    body += " réussie</title>\n</head>\n<body>\n\t<h1>Formulaire upload avec succès";
    body += "</h1>\n\t<p>Merci pour votre soumission!";
    body += "!</p>\n\t<p>Votre soumission: ";
    body += fileName;
    body += "</p>\n\t<p><a href=\"/\">Retour à l'accueil</a></p>\n</body>\n</html>";
    return(body);
}

void posteCommentTextPlain(const t_serveur &server, std::string body, const t_location &current_location)
{
    std::string path;
    if (current_location.root == "")
        path = "." + server.root + current_location.path;
    else
        path = "." + current_location.root + current_location.path;
    path += "/Uploads/CommentsTextPlain.txt";
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::app);
    if (!file.is_open()) {
        std::string	err_msg("Error 403 : No permission to open the file");
        throw HttpError(err_msg);
    }
    file << body;
    file << "\n";
    if (file.fail()) {
        std::string	err_msg("Error 500 : Cannot write in file");
        throw HttpError(err_msg);
    }
    file.close();
}

void handlePost(const t_serveur &server, t_client &client, const t_location &current_location)
{
	std::string fileName;
    std::map<std::string, std::string>			headers = client.request.getHeader();

    if (headers["Content-Type"] == "application/x-www-form-urlencoded") {
    //    std::cout << "\n\n\n\n\nooooooooooooooooooooooooooooooooooooooo\n\n\n\n\n\n";
        std::map<std::string, std::string> bodyMap = parse_urlencoded(client.request.getbodyVec());
    //    std::cout << "\n\n\n\n\nooooooooooooooooooooooooooooooooooooooo\n\n\n\n\n\n";
        try {
            PostComment(server, bodyMap, current_location);
        }
        catch (const HttpError& e) {
            client.response = sendErrorResponsePost(e.what());
            client.responseReady = true;
            return ;
        }
        fileName = bodyMap["name"] + ".txt";
        client.response.setBody(formatResponseBody(bodyMap));
        client.response.setStatus(201, "Created");
        client.response.setHeader("Server", "Webserv/1.0");
        client.response.setHeader("Date", getHTTPDate());
        client.response.setHeader("Content-Type", "text/html; charset=UTF-8");
        if (headers["Connection"] == "keep-alive" || (client.request.getVersion() == "HTTP/1.1" && headers["Connection"] != "close"))
            client.response.setHeader("Connection", "keep-alive");
        else if (headers["Connection"] == "close" || client.request.getVersion() == "HTTP/1.0")
            client.response.setHeader("Connection", "close");
        size_t bodySize = formatResponseBody(bodyMap).length();
        std::ostringstream oss;
        oss << bodySize;
        client.response.setHeader("Content-Length", oss.str());
        client.responseReady = true;
    }
    else if (headers["Content-Type"].find("multipart/form-data") != std::string::npos)
    {
    //    std::cout << "\n" << client.responseReady << "\n                -                   \n";
        divide_multipart(headers["Content-Type"], client);
        //std::cout << "\n" << client.responseReady << "\nooooooooooooooooooooooooooooooooooooooo\n";
        if (!client.responseReady)
            return ;
        try {
            PostUpload(server, client.datasBuffer, current_location);
        }
        catch (const HttpError& e) {
            client.response = sendErrorResponsePost(e.what());
            client.responseReady = true;
            return ;
        }
        if (client.datasBuffer.filename == "")
            fileName = client.datasBuffer.name;
        else
            fileName = client.datasBuffer.filename;
        client.response.setBody(formatResponseBodyUpload(fileName));
        client.response.setStatus(201, "Created");
        client.response.setHeader("Server", "Webserv/1.0");
        client.response.setHeader("Date", getHTTPDate());
        client.response.setHeader("Content-Type", "text/html; charset=UTF-8");
        if (headers["Connection"] == "keep-alive" || (client.request.getVersion() == "HTTP/1.1" && headers["Connection"] != "close"))
            client.response.setHeader("Connection", "keep-alive");
        else if (headers["Connection"] == "close" || client.request.getVersion() == "HTTP/1.0")
            client.response.setHeader("Connection", "close");
        size_t bodySize = formatResponseBodyUpload(fileName).length();
        std::ostringstream oss;
        oss << bodySize;
        client.response.setHeader("Content-Length", oss.str());
    }
    else if (headers["Content-Type"] == "plain/text") {
        try {
            posteCommentTextPlain(server, client.request.getBody(), current_location);
        }
        catch (const HttpError& e) {
            client.response = sendErrorResponsePost(e.what());
            client.responseReady = true;
            return ;
        }
        client.responseReady = true;
        client.response.setBody(formatResponseBodyUpload(client.request.getBody()));
        client.response.setStatus(201, "Created");
        client.response.setHeader("Server", "Webserv/1.0");
        client.response.setHeader("Date", getHTTPDate());
        client.response.setHeader("Content-Type", "text/html; charset=UTF-8");
        if (headers["Connection"] == "keep-alive" || (client.request.getVersion() == "HTTP/1.1" && headers["Connection"] != "close"))
            client.response.setHeader("Connection", "keep-alive");
        else if (headers["Connection"] == "close" || client.request.getVersion() == "HTTP/1.0")
            client.response.setHeader("Connection", "close");
        size_t bodySize = client.response.getBody().length();
        std::ostringstream oss;
        oss << bodySize;
        client.response.setHeader("Content-Length", oss.str());
    }
    else if (headers["Content-Type"] == "application/json") {
        std::map<std::string, std::string> bodyMap = parse_Json(client.request.getBody());
    //    std::cout << "\n\n\n\n\nooooooooooooooooooooooooooooooooooooooo\n\n\n\n\n\n";
        try {
            PostCommentJson(server, bodyMap, current_location);
        }
        catch (const HttpError& e) {
            client.response = sendErrorResponsePost(e.what());
            client.responseReady = true;
            return ;
        }
        client.response.setBody(formatResponseBodyUpload("un truc au format Json"));
        client.response.setStatus(201, "Created");
        client.response.setHeader("Server", "Webserv/1.0");
        client.response.setHeader("Date", getHTTPDate());
        client.response.setHeader("Content-Type", "text/html; charset=UTF-8");
        if (headers["Connection"] == "keep-alive" || (client.request.getVersion() == "HTTP/1.1" && headers["Connection"] != "close"))
            client.response.setHeader("Connection", "keep-alive");
        else if (headers["Connection"] == "close" || client.request.getVersion() == "HTTP/1.0")
            client.response.setHeader("Connection", "close");
        size_t bodySize = formatResponseBodyUpload("un truc au format Json").length();
        std::ostringstream oss;
        oss << bodySize;
        client.response.setHeader("Content-Length", oss.str());
        client.responseReady = true;
    }
    else
    {
        client.response = sendErrorResponsePost("Error 415 : Content-Type Unknown");
        client.responseReady = true;
        return ;
    }
	return ;
}
