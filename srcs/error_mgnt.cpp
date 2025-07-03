/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_mgnt.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prambaud <prambaud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 14:53:37 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 17:02:13 by prambaud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

std::string error_404_mgnt(std::string path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
    std::cerr << "ERREUR: Fichier non trouvé: " << path << std::endl;
    return "";}
    std::string body;
    std::string line;
    std::string word;
    while(std::getline(file, line))
    {
        body += line;
        body += "\n";
 
    }
    file.close();
    return(body);
}