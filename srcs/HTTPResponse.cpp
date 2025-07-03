/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 14:18:20 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/02 11:45:59 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse()
{
    bitsRead = 0;
    bitsSent = 0;
    body_char_bool = false;
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::setStatus(int code, const std::string& message)
{
    status_code = code;
    status_message = message;
}

void HTTPResponse::setHeader(const std::string& name, const std::string& value)
{
    headers[name] = value;
}

void HTTPResponse::setHeader(const std::map<std::string, std::string>& input)
{
    headers = input;
}

void								HTTPResponse::setbody_char_bool(bool input)
{
    body_char_bool = input;
}

bool      					HTTPResponse::getbody_char_bool()
{
    return (body_char_bool);
}

void HTTPResponse::setBodyToBodyChar()
{
    size_t i = 0;
    while (i < body.size())
    {
        body_char.push_back(body[i]);
        i++;
    }
}

void HTTPResponse::setbody_char(std::vector<char> &content)
{
    body_char = content;
}

void HTTPResponse::addbody_char(std::vector<char> &content)
{
    std::vector<char>::iterator it;
    for (it = content.begin(); it != content.end(); it++)
        body_char.push_back(*it);
}

std::vector<char> &HTTPResponse::getbody_char()
{
    return (body_char);
}

void HTTPResponse::setBody(const std::string& content)
{
    body = content;
}

void    HTTPResponse::setbitsRead(size_t input)
{
    bitsRead = input;
}

void    HTTPResponse::setbitsSent(size_t input)
{
    bitsSent = input;
}
size_t HTTPResponse::getbitsSent() const
{
    return (bitsSent);
}

size_t HTTPResponse::getbitsRead() const
{
    return (bitsRead);
}

std::string						HTTPResponse::getBody() const
{
    return (body);
}

void HTTPResponse::addBody(const std::string& content)
{
    body += content;
}

void HTTPResponse::setBodyparsed(const std::map<std::string, std::string>& input)
{
    body_parsed = input;
}


std::map<std::string, std::string>	&HTTPResponse::getHeader() {
    return headers;
}

// Génère la réponse HTTP complète
std::vector<char> HTTPResponse::toVectorChar()
{
    std::ostringstream response;
    // 1. Status
    response << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
    // 2. Headers
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }
    // 3. fin des headers
    response << "\r\n";
    std::string yo = response.str();
    // 4. Body
    std::vector<char> repVecChar;
    size_t i(0);
    while (i < yo.size())
    {
        repVecChar.push_back(yo[i]);
        i++;
    }
    std::vector<char>::iterator it;
    for (it = body_char.begin(); it != body_char.end(); it++)
        repVecChar.push_back(*it);
    return repVecChar;
}
