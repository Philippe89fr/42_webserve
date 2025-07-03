/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 15:06:39 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 15:35:30 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

void	HTTPRequest::setMethod(std::string input) {
    method = input;
}

void	HTTPRequest::setBody(std::string input) {
    body = input;
}

void	HTTPRequest::setUri(std::string input) {
    uri = input;
}

void	HTTPRequest::setbodyVec(std::vector<char> input) {
    bodyVec = input;
}

void	HTTPRequest::setVersion(std::string input) {
    version = input;
}

void	HTTPRequest::setUriPath(std::string input) {
    uriPath = input;
}

void	HTTPRequest::setHeader(std::string input, std::string input2) {
    headers[input] = input2;
}

void	HTTPRequest::setServeur(t_serveur const &found_serveur)
{
	this->serveur = &found_serveur;
}

void	HTTPRequest::resetRequest()
{
	if (this->CGI_process)
		delete this->CGI_process;
	this->CGI_process = NULL;
	this->serveur = NULL;
	this->location = NULL;
}

void	HTTPRequest::setLocation(t_location const &found_location)
{
	this->location = &found_location;
}

void	HTTPRequest::makeCGIProcess(pid_t pid, int fd_in, int fd_out)
{
	if (this->CGI_process)
		throw std::runtime_error("CGI_process already exist");
	this->CGI_process = new CGIProcess(pid, fd_in, fd_out);
}

const std::string	HTTPRequest::getMethod() const {
    return method;
}

const std::vector<char>	HTTPRequest::getbodyVec() const {
	return bodyVec;
}

const std::string	HTTPRequest::getUriPath() const {
    return uriPath;
}

const std::string	HTTPRequest::getBody() const {
    return body;
}

std::string	HTTPRequest::getUri() const {
    return uri;
}

const std::string	HTTPRequest::getVersion() const {
    return version;
}

const std::map<std::string, std::string>	&HTTPRequest::getHeader() const {
    return headers;
}

const t_serveur		*HTTPRequest::getServeur() const
{
	return (this->serveur);
}

const t_location	*HTTPRequest::getLocation() const
{
	return (this->location);
}

CGIProcess			*HTTPRequest::getCGIProcess() const
{
	return (this->CGI_process);
}

HTTPRequest::HTTPRequest()
{
	this->CGI_process = NULL;
	this->serveur = NULL;
	this->location = NULL;
}

HTTPRequest::~HTTPRequest()
{
	if (this->CGI_process)
		delete this->CGI_process;
}
