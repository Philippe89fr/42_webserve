/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   callCGI.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prambaud <prambaud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 18:47:02 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/03 11:06:42 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"
#include "webserv.hpp"

static void	freetab(std::vector<char *> &env)
{
	std::vector<char *>::iterator	it = env.begin();

	while (it != env.end())
	{
		free(*it);
		++it;
	}
}

static void	launchCGI(HTTPRequest &requestHTTP, std::string const &CGI_exec, std::vector<char *> env)
{
//	std::cerr	<< std::endl << "------------------------ CGI LAUNCH ------------------------"
//				<< std::endl << std::endl;
	if (CGI_exec.size())
	{
//		std::cerr	<< "found to be script file." << std::endl
//					<< "CGI_exec: " << CGI_exec << std::endl;
		char	*argv[3];
		argv[0] = strdup(CGI_exec.c_str());
		argv[1] = strdup(requestHTTP.getUriPath().c_str());
		argv[2] = NULL;
		execvpe(CGI_exec.c_str(), argv, env.data());
	}
	else
	{
		std::string	binary_name = requestHTTP.getUriPath().substr(requestHTTP.getUri().find_last_of("/") + 1);
//		std::cerr	<< "found to be binary." << std::endl
//					<< "binary_name: " << binary_name << std::endl;
		execle(requestHTTP.getUriPath().c_str(), binary_name.c_str(), NULL, env.data());
	}
	freetab(env);
	exit(-1);
}

static char					*makeenvvar(std::string var_name, std::string value)
{
	return (strdup((var_name + value).c_str()));
}

static std::vector<char *>	makeCGIenv(HTTPRequest &requestHTTP)
{
	std::vector<char *>									env;
	std::map<std::string, std::string> const			&request_header = requestHTTP.getHeader();
	std::map<std::string, std::string>::const_iterator	header_it;
	std::string											buffer;

	if (!getenv("PATH"))
		env.push_back(makeenvvar("PATH=", ""));
	else
		env.push_back(makeenvvar("PATH=", getenv("PATH")));
	env.push_back(makeenvvar("SERVER_SOFTWARE=", ""));
	env.push_back(makeenvvar("SERVER_NAME=", requestHTTP.getServeur()->serverName));
	env.push_back(makeenvvar("GATEWAY_INTERFACE=CGI/1.1", ""));
	env.push_back(makeenvvar("SERVER_PROTOCOL=", requestHTTP.getVersion()));
	header_it = request_header.find("Host");
	if (header_it != request_header.end())
	{
		buffer = header_it->second;
		if (buffer.size())
			buffer = buffer.substr(buffer.find_last_of(":"), 1);
	}
	else
		buffer = "";
	env.push_back(makeenvvar("SERVER_PORT=", buffer));
	env.push_back(makeenvvar("REQUEST_METHOD=", requestHTTP.getMethod()));
	env.push_back(makeenvvar("PATH_INFO=", ""));
	env.push_back(makeenvvar("PATH_TRANSLATED=", requestHTTP.getUriPath()));
	env.push_back(makeenvvar("SCRIPT_NAME=", requestHTTP.getUri()));
	env.push_back(makeenvvar("QUERY_STRING=", ""));
	env.push_back(makeenvvar("REMOTE_HOST=", ""));
	env.push_back(makeenvvar("REMOTE_ADDR=", ""));
	env.push_back(makeenvvar("AUTH_TYPE=", ""));
	env.push_back(makeenvvar("REMOTE_USER=", ""));
	env.push_back(makeenvvar("REMOTE_IDENT=", ""));
	env.push_back(makeenvvar("CONTENT_TYPE=", ""));
	env.push_back(makeenvvar("CONTENT_LENGTH=", ""));
	header_it = request_header.find("Accept");
	if (header_it != request_header.end())
		buffer = header_it->second;
	else
		buffer = "";
	env.push_back(makeenvvar("HTTP_ACCEPT=", buffer));
	header_it = request_header.find("User-Agent");
	if (header_it != request_header.end())
		buffer = header_it->second;
	else
		buffer = "";
	env.push_back(makeenvvar("HTTP_USER_AGENT=", buffer));
	header_it = request_header.find("Cookie");
	if (header_it != request_header.end())
		buffer = header_it->second;
	else
		buffer = "";
	env.push_back(makeenvvar("HTTP_COOKIE=", buffer));
	header_it = request_header.find("Referer");
	if (header_it != request_header.end())
		buffer = header_it->second;
	else
		buffer = "";
	env.push_back(makeenvvar("HTTP_REFERER=", buffer));
	env.push_back(NULL);

//	std::cout << "CGI env: " << std::endl << std::endl;
//	std::vector<char *>::iterator	v_it = env.begin();
//	while (v_it != env.end())
//	{
//		std::cout << *v_it << std::endl;
//		++v_it;
//	}
	return (env);
}

void	getCGIresponse(HTTPRequest &requestHTTP, t_location const &current_location, t_globalData &dataStruct)
{
	std::string const									&uri = requestHTTP.getUriPath();
	std::string											ext = uri.substr(uri.find_last_of('.'));
	std::map<std::string, std::string>::const_iterator	it = current_location.cgi.find(ext);

	if (it == current_location.cgi.end())
	{
		std::string	err_msg("Error 500 : Internal Server Error");
		throw HttpError(err_msg);
	}

	std::string const	&CGI_exec = it->second;
	int					pipe_in[2];
	int					pipe_out[2];
	pid_t				pid;

//	std::cout	<< std::endl << "------------------------ CGI INFOS ------------------------"
//				<< std::endl << std::endl << "CGI ext: " << ext << std::endl
//				<< "CGI exec: " << CGI_exec << std::endl << std::endl
//				<< "-----------------------------------------------------------" << std::endl << std::endl;

	std::vector<char *>	env = makeCGIenv(requestHTTP);

	if (pipe(pipe_in) == -1)
		throw std::runtime_error("failed to open CGI pipe");
	if (pipe(pipe_out) == -1)
	{
		close(pipe_in[0]);
		close(pipe_in[1]);
		throw std::runtime_error("failed to open CGI pipe");
	}
	pid = fork();
	if (pid == -1)
		throw std::runtime_error("failed to initiate child process");
	else if (!pid)
	{
		sighandler_child();
		close(pipe_out[0]);
		close(pipe_in[1]);
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[0]);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[1]);
		freeall(dataStruct);
		launchCGI(requestHTTP, CGI_exec, env);
	}
	else
	{
		freetab(env);
		close(pipe_in[0]);
		close(pipe_out[1]);
		requestHTTP.makeCGIProcess(pid, pipe_in[1], pipe_out[0]);
	}
}
