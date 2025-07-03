/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 11:24:52 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/03 15:35:43 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>    // pour std::string
#include <map>
#include <stdexcept>
#include "CGIProcess.hpp"
#include <vector>

// Forward declarations au lieu d'include
struct s_serveur;
struct s_location;
typedef struct s_serveur t_serveur;
typedef struct s_location t_location;

class HTTPRequest {
private:
    std::string									method;
    std::string									uri;
    std::string									uriPath;
    std::string									version;
    std::map<std::string, std::string>			headers;
    std::string									body;
	t_serveur const								*serveur;
	t_location const							*location;
	CGIProcess									*CGI_process;
    std::vector<char>							bodyVec;

public:
    HTTPRequest();
    ~HTTPRequest();
    void										setMethod(std::string input);
    void										setBody(std::string input);
    void										setUri(std::string input);
    void										setUriPath(std::string input);
    void										setVersion(std::string input);
    void										setbodyVec(std::vector<char> input);
    void										setHeader(std::string input, std::string input2);
	void										setServeur(t_serveur const &found_serveur);
	void										setLocation(t_location const &found_location);
	void										makeCGIProcess(pid_t pid, int fd_in, int fd_out);
    void										resetRequest();
    const std::map<std::string, std::string>&	getHeader() const;
    const std::string							getUriPath() const;
    const std::vector<char>						getbodyVec() const;
    const std::string							getMethod() const;
    std::string							        getUri() const;
    const std::string							getVersion() const;
    const std::string							getBody() const;
	const t_serveur								*getServeur() const;
	const t_location							*getLocation() const;
	CGIProcess									*getCGIProcess() const;
};
