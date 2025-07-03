/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   postMultipart.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prambaud <prambaud@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 15:40:50 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 11:21:40 by prambaud         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

// fonction qui permet de savoir si les X dernier caracteres lus sont boundarryyyyyyy LUCAS HAPPPTYYYYYYYY

// Enum pour identifier les patterns
enum PatternType {
    PATTERN_NAME = 0,
    PATTERN_FILENAME = 1, 
    PATTERN_CONTENT_TYPE = 2,
    PATTERN_BODY = 3,
    PATTERN_NONE = -1
};

// Patterns en constantes statiques (créés une seule fois)
static const std::string PATTERNS[] = {
    " name=\"",           // 0
    "filename=\"",        // 1  
    "Content-Type: ",     // 2
    "\n\r\n"           // 3 (j'ai corrigé ton "\n\r\n" qui est probablement faux)
};


// Version optimisée avec memcmp
int findPattern(const std::vector<char>& body, size_t pos) {
    // Vérification bounds une seule fois
    if (pos >= body.size()) return PATTERN_NONE;
    
    // Teste tous les patterns avec memcmp (plus rapide)
    for (size_t i = 0; i < PATTERNS->size(); ++i) {
        const std::string& pattern = PATTERNS[i];
        
        // Vérification bounds pour ce pattern
        if (pos + pattern.size() <= body.size()) {
            if (std::memcmp(&body[pos], pattern.data(), pattern.size()) == 0) {
                return i;  // Retourne l'index du pattern trouvé
            }
        }
    }
    
    return PATTERN_NONE;  // Aucun pattern trouvé
}

bool stringFinder(std::vector<char> &body, size_t pos, const std::string &boundary) {
    size_t size = boundary.size();
    size_t i(0);

    while(i < size)
    {
        if(body[pos + i] != boundary[i])
            return false;
        i++;
    }
    return true;
}

void divide_multipart(std::string &boundary, t_client &client)
{
    boundary = "--" + boundary.substr(boundary.find_last_of("=") + 1, boundary.size());
    size_t i(0);
    std::string word;
    std::vector<char> body = client.request.getbodyVec();
    size_t size = client.request.getbodyVec().size();
    size_t temp(0);

//    std::cout << "client.datasBuffer.bodyVecBitsRead  :" << client.datasBuffer.bodyVecBitsRead << std::endl;
    if(client.datasBuffer.bodyVecBitsRead == 0)
        i += boundary.size();
    else 
        i = client.datasBuffer.bodyVecBitsRead;
    temp = i;
    while (i < size && !stringFinder(body, i, boundary) && i - temp < CHUNKECR)
    {
        int pattern = findPattern(body, i);
        if(pattern == PATTERN_NAME && temp == boundary.size())
        {
            i += PATTERNS[0].size();
            while (i < body.size() && body[i] != '"')
            {
                client.datasBuffer.name += body[i];
                i++;
            }
        }
        else if(pattern == PATTERN_FILENAME && temp == boundary.size())
        {
            i += PATTERNS[1].size();
            while (i < body.size() && body[i] != '"')
            {
                client.datasBuffer.filename += body[i];
                i++;
            }
        }
        else if(pattern == PATTERN_CONTENT_TYPE && temp == boundary.size())
        {
            i += PATTERNS[2].size();
            while (i < body.size() && body[i] != '\r')
            {
                client.datasBuffer.content_type += body[i];
                i++;
            }
        }
        else if(pattern == PATTERN_BODY && temp == boundary.size())
        {
            i += PATTERNS[3].size();
            while (i < body.size() && !stringFinder(body, i, boundary) && i - temp < CHUNKECR)
            {
                client.datasBuffer.body.push_back(body[i]);
                i++;
            }
        }
        else if(temp != boundary.size())
        {
            while (i < body.size() && !stringFinder(body, i, boundary) && i - temp < CHUNKECR)
            {
                client.datasBuffer.body.push_back(body[i]);
                i++;
            }
        }
        if(stringFinder(body, i, boundary))
            break;
        i++;
    }
    if(temp == boundary.size())
        client.datasBuffer.bodyVecBitsRead += i - 1;
    else
        client.datasBuffer.bodyVecBitsRead += i - temp -1;
    if(i - temp <= CHUNKECR)
        client.responseReady = true;
}
