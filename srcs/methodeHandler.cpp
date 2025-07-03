/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   methodeHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 16:46:54 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 16:34:07 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//L'autoindex se déclenche uniquement lorsque la requête cible explicitement un répertoire:
//Autoindex dans le cas d'une requete GET : si on fait une requete GET dans un repertoire sans index, et que l'autoindex est ON
//alors on ouvre une nouvelle page avec la liste des repertoires / fichiers dans le repertoire en question.

//(c'est-à-dire quand l'URI se termine par / ou correspond à un répertoire sans fichier spécifié)

HTTPResponse sendErrorResponseGet(const char *str)
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
    if (word == "500")
    {
        response.setStatus(500, "Internal Server Error");
        body = formatErrorPage(word, "Error : Unexpected system error occured");
    }
    if (word == "403")
    {
        response.setStatus(403, "Forbidden");
        body = formatErrorPage(word, "Error : Forbidden");
    }
    if (word == "404")
    {
        response.setStatus(404, "Resource not found");
        body = error_404_mgnt("./www/webservSite/error_404.html");
    }
    if (word == "405")
    {
        response.setStatus(405, "Method not allowed");
        body = formatErrorPage(word, "Error : Method not allowed");
    }
    size_t value = body.length();
    std::ostringstream oss;
    oss << value;
    response.setHeader("Content-Length", oss.str());
    response.setBody(body);
    return (response);
}

static HTTPResponse returnResponse(HTTPRequest &requestHTTP, const t_location &current_location) {
    HTTPResponse response;

    std::map<std::string, std::string> header = requestHTTP.getHeader();
    long code = atoi(current_location.return_.first.c_str());
    if(code == 302) {
        response.setStatus(code, "Found");
    }
    if(code == 301) {
        response.setStatus(code, "Moved Permanently");
    }
    response.setHeader("Server", "Webserv/1.0");
    response.setHeader("Date", getHTTPDate());
/*    std::string redirection = current_location.return_.second;
    if (redirection.find("/") != std::string::npos && redirection.find("http") == std::string::npos)
    {
        std::vector<std::string> path;
        while (redirection.find("/") != std::string::npos)
        {
            path.push_back(redirection.substr(0, (int)redirection.find("/")));
            redirection = redirection.substr((int)redirection.find("/") + 1, redirection.size());
        }
        size_t i = 0;
        std::cout << "--------------------------------------------------" << std::endl;
        while (i < path.size())
        {
            std::cout << path[i] << std::endl;
            i++;
        }
        std::cout << "--------------------------------------------------" << std::endl;
    }*/
    response.setHeader("Location", current_location.return_.second);
    if (header["Connection"] == "keep-alive" || (requestHTTP.getVersion() == "HTTP/1.1" && header["Connection"] != "close"))
        response.setHeader("Connection", "keep-alive");
    else if (header["Connection"] == "close" || requestHTTP.getVersion() == "HTTP/1.0")
        response.setHeader("Connection", "close");
    response.setHeader("Content-Length", "0");
	return(response);
}

static void handleGet(const t_serveur &server, t_client &client, const t_location &current_location) {

    // lier parsing request et parsing config //
    std::string	path = client.request.getUriPath();

//    std::cout << "UriPath : " << path << std::endl;
    struct stat	path_stat;
	if(current_location.return_.first.size()) {
        client.response = returnResponse(client.request, current_location);
        client.responseReady = true;
    }
	else try {
        if (stat(path.c_str(), &path_stat) != 0)
        {
			std::string	err_msg("error 404 : Resource not found");
            throw HttpError(err_msg);
        }
        else if (S_ISDIR(path_stat.st_mode)) // si c est un dossier
        {
            if(!current_location.index_bool && current_location.autoindex)  // location sans index + autoIndex ON
            {
                client.response = pushAutoindex(server, current_location, path);
                client.responseReady = true;
            }
            else if (current_location.index_bool || (current_location.path == "/" || current_location.path == "./")) {
                client.request.setUriPath(client.request.getUriPath());
                GetResponse(client, client.request.getUriPath());
            } // location avec index
            else // location sans index + autoIndex OFF
			{
				std::string	err_msg("Error 403 : Forbidden");
                throw HttpError(err_msg);
			}
        }
        else // pas un dossier
            GetResponse(client, client.request.getUriPath());
    }
    catch (const HttpError& e) {
        client.response = sendErrorResponseGet(e.what());
        client.responseReady = true;
    }
    return ;
}

static HTTPResponse	makeCGIresponse(HTTPRequest &requestHTTP)
{
    HTTPResponse						response;
    std::map<std::string, std::string>	headers = requestHTTP.getHeader();
	std::ifstream						temp_file_stream(requestHTTP.getCGIProcess()->getTempFileName().c_str());
	std::string							body;

	while (!temp_file_stream.eof())
	{
		std::string	line;
		std::getline(temp_file_stream, line);
		body += line + '\n';
	}
	temp_file_stream.close();
	response.setBody(body);
	response.setStatus(200, "OK");
	response.setHeader("Server", "Webserv/1.0");
	response.setHeader("Date", getHTTPDate());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	if (headers["Connection"] == "keep-alive" || (requestHTTP.getVersion() == "HTTP/1.1" && headers["Connection"] != "close"))
	    response.setHeader("Connection", "keep-alive");
	else if (headers["Connection"] == "close" || requestHTTP.getVersion() == "HTTP/1.0")
	    response.setHeader("Connection", "close");
	size_t bodySize = body.length();
	std::ostringstream oss;
	oss << bodySize;
	response.setHeader("Content-Length", oss.str());
//	std::cout << body << std::endl;
    return (response);
}

static void	giveCGIrequestbody(t_client &client)
{
	CGIProcess			&processCGI = *client.request.getCGIProcess();
	std::string const	&request_body = client.request.getBody();
	ssize_t				bytes_read = processCGI.getBytesRead();
	int					fd_in = processCGI.getFdIn();
	int					fd_out = processCGI.getFdOut();
	int					temp_file_fd = processCGI.getTempFileFd();

//	std::cout << "Bytes_read: " << bytes_read << " | body_size: " << request_body.size() << std::endl;
	if (static_cast<size_t>(bytes_read) < request_body.size() && fd_in)
	{
		bytes_read += write(fd_in, &(request_body.c_str()[bytes_read]), CHUNKREAD);
		if (bytes_read > processCGI.getBytesRead())
			processCGI.setBytesRead(bytes_read);
	}
	else if (fd_in)
	{
//		std::cout << "body passed to child process successfully" << std::endl;
		close(fd_in);
		processCGI.setFdIn(0);
	}

	char	buffer[CHUNKREAD];
	ssize_t	signed_bytes_read = read(fd_out, &buffer, CHUNKREAD);
//	std::cout << "finished to read " << signed_bytes_read << " bytes in pipe" << std::endl;

	if (signed_bytes_read > 0)
	{
		write(temp_file_fd, buffer, signed_bytes_read);
//		std::cout << "finished to write in tempfile" << std::endl;
	}
	waitpid(processCGI.getPid(), processCGI.getStatus(), WNOHANG);
//	std::cout << "finished to waitpid" << std::endl;
	if (*processCGI.getStatus() > -1 && signed_bytes_read < CHUNKREAD)
	{
		close(temp_file_fd);
		processCGI.setTempFileFd(0);
		close(fd_out);
		processCGI.setFdOut(0);
		if (*processCGI.getStatus())
			client.response = sendErrorResponseGet("Error 500 : Internal Server Error");
		else
			client.response = makeCGIresponse(client.request);
		client.responseReady = true;
	}
	else if (*processCGI.getStatus() == -1 && (std::clock() - client.time_start) / CLOCKS_PER_SEC > 300)
	{
		if (fd_in)
		{
			close(fd_in);
			processCGI.setFdIn(0);
		}
		if (fd_out)
		{
			close(fd_out);
			processCGI.setFdOut(0);
		}
		if (temp_file_fd)
		{
			close(temp_file_fd);
			processCGI.setTempFileFd(0);
		}
		client.response = sendErrorResponseGet("Error 500 : Internal Server Error");
		client.responseReady = true;
	}
}

void executeRequest(std::vector<t_serveur> &serversPort, t_client &client,
					t_globalData &dataStruct) {

    getrequestlocation(serversPort, client.request);
    HTTPResponse response;

	t_serveur const		&current_serveur = *client.request.getServeur();
    t_location const	&current_location = *client.request.getLocation();

    // if(std::find(current_location.methods.begin(),current_location.methods.end(), requestHTTP.getMethod()) == current_location.methods.end()) {
    //     throw HttpError("405 Method Not Allowed\n");
    // }
    makepathfromURI(current_serveur, client.request, current_location);

    //check_methods_autorized
    size_t i(0);
    bool method_autorized = false;
    std::string methods = "";
    while (i < current_location.methods.size())
    {
        methods += current_location.methods[i] + " ";
        if (client.request.getMethod() == current_location.methods[i])
            method_autorized = true;
        i++;
    }
	if (!method_autorized)
	{
		client.response = sendErrorResponseGet("Error 405 : Method Not Allowed");
        client.response.setHeader("Allow", methods);
		client.responseReady = true;
		client.response.setBodyToBodyChar();
		client.response.setbody_char_bool(true);
	}
	else if (current_location.cgi_bool) //CGI
	{
		try
		{
		if (!client.request.getCGIProcess())
		{
			getCGIresponse(client.request, current_location, dataStruct);
			client.time_start = std::clock();
		}
		else
			giveCGIrequestbody(client);
		}
		catch (const HttpError &e)
		{
			client.response = sendErrorResponseGet(e.what());
			client.responseReady = true;
		}
	}
	else if (client.request.getMethod() == "GET") {
		handleGet(current_serveur, client, current_location);
	}
	else if (client.request.getMethod() == "POST") {
		handlePost(current_serveur, client, current_location);
	}
	else
    {
        client.response = handleDelete(client.request);
        client.responseReady = true;
    }
//     std::cout << std::endl << std::endl << "---------------------- RESPONSE START -------------------------" << std::endl;
//    i = 0;
//     std::vector<char> yo = client.response.toVectorChar();
//     while (i < yo.size() && i < 200)
//     {
//        std::cout << yo[i];
//        i++;
//    }
//    std::cout << "\n";
//    std::cout << std::endl << std::endl << "---------------------- RESPONSE END ---------------------------" << std::endl;
    return ;
}
