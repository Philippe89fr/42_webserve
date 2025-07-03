/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 18:38:01 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/03 10:58:16 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <string>
#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <sstream>   // pour std::ostringstream

// Forward declarations au lieu d'include
struct s_serveur;
struct s_location;
typedef struct s_serveur t_serveur;
typedef struct s_location t_location;

typedef struct s_formdata
{
    std::string			name;
    std::string			filename;
    std::string			content_type;
    std::vector<char>	body;
    size_t				bodyVecBitsRead;
    size_t				lecture;
} t_formdata;

class HTTPResponse
{
private:
    int									status_code;
    bool								body_char_bool;
    size_t								bitsRead;
    size_t								bitsSent;
    std::string							status_message;
    std::map<std::string, std::string>	headers;
	std::map<std::string, std::string>	body_parsed;
    std::string							body;
    std::vector<char>					body_char;

public:
    HTTPResponse();
    ~HTTPResponse();
    std::map<std::string, std::string>&	getHeader();
    void								setStatus(int code, const std::string& message);
    void								setHeader(const std::string& name, const std::string& value);
    void								setHeader(const std::map<std::string, std::string>& input);
    void								setBody(const std::string& content);
    void								setbitsRead(size_t input);
    void								setbitsSent(size_t input);
    void								setbody_char_bool(bool input);
    void								addbody_char(std::vector<char> &content);
    size_t								getbitsRead() const;
    size_t								getbitsSent() const;
    std::string							getBody() const;
    bool								getbody_char_bool();
    void								setBodyToBodyChar();
    void								setbody_char(std::vector<char> &content);
    std::vector<char>&					getbody_char();
    void								addBody(const std::string& content);
	void								setBodyparsed(const std::map<std::string, std::string>& input);
    std::vector<char>					toVectorChar(); // Génère la réponse HTTP complete
};

class HttpError : public std::exception
{
public:
	HttpError(std::string &msg) : _msg(strdup(msg.c_str())) {}
	~HttpError() throw() {free(this->_msg);}
	const char *what() const throw()
	{
		return (_msg);
	}
private:
	char	*_msg;
};
