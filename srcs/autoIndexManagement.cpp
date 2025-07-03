/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   autoIndexManagement.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 17:55:24 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 11:59:15 by lbenatar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//void    print_dir(std::vector<t_dir> dir_vec)
//{
//    size_t i = 0;
//    while (i < dir_vec.size())
//    {
//        std::cout << "NAME :" << dir_vec[i].name << "  //  TAILLE :" << dir_vec[i].size << "  //  MODIF :" << dir_vec[i].modifDate;
//        std::cout << std::endl;
//        i++;
//    }
//}

HTTPResponse createResponseAutoindex(std::vector<t_dir> dir_vec, std::string path, std::string location)
{
    HTTPResponse response;

    path = path + "";
    response.setStatus(200, "OK");
    response.setHeader("Server", "Webserv/1.0");
    response.setHeader("Content-Type", "text/html");
    std::string body;
    body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>Index of " + location + "</title>\n\t<style>\n\t\tbody { font-family: Arial, sans-serif; margin: 20px; }\n\t\th1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n\t\t";
    body = body + "table { border-collapse: collapse; width: 100%; }\n\t\tth, td { text-align: left; padding: 8px; }\n\t\ttr:nth-child(even) { background-color: #f2f2f2; }\n\t\ta { text-decoration: none; }\n\t\t";
    body = body + ".dir { font-weight: bold; }\n\t</style>\n</head>\n<body>\n\t<h1>Index of " + location + "</h1>\n\t<table>\n\t\t<tr>\n\t\t\t<th>Name</th>\n\t\t\t<th>Size</th>\n\t\t\t";
    body = body + "<th>Last Modified</th>\n\t\t</tr>";
    size_t i = 0;
    while (i < dir_vec.size())
    {
        if (dir_vec[i].name == "..")
            body += "\n\t\t<tr>\n\t\t\t<td><a href=\"" + dir_vec[i].name + "\">" + "Parent Directory" + "</a></td>";
        else
            body += "\n\t\t<tr>\n\t\t\t<td><a href=\"" + location + "/" + dir_vec[i].name + "\">" + dir_vec[i].name + "</a></td>";
        body += "\n\t\t\t<td>" + dir_vec[i].size + " KB</td>";
        body += "\n\t\t\t<td>" + dir_vec[i].modifDate.substr(0, dir_vec[i].modifDate.size() - 1) + "</td>";
        body += "\n\t\t</tr>";
        i++;
    }
    body += "\n\t</table>";
    body += "\n</body>\n</html>";
    response.setBody(body);
    size_t value = body.length();
    std::ostringstream oss;
    oss << value;
    response.setHeader("Content-Length", oss.str());
    return (response);
}

HTTPResponse pushAutoindex(const t_serveur &server, const t_location &current_location, std::string	path)
{
    std::vector<t_dir> dir_vec;

    // Ouvrir le répertoire
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        throw std::runtime_error("error 403 : Access denied\n");
    }
    // Lire les entrées une par une
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        struct stat	path_stat;
        stat((path + "/" + entry->d_name).c_str(), &path_stat);
        t_dir vec;
        vec.modifDate = (std::string)std::ctime(&path_stat.st_mtime);
        vec.name = entry->d_name;
        float size_float = path_stat.st_size / 1000.0;
        std::ostringstream oss;
        oss << size_float;
        vec.size = oss.str();
        if (vec.name != "." && !(vec.name == ".." && path == "." + server.root + "/"))
            dir_vec.push_back(vec);
    }
    closedir(dir);
//    print_dir(dir_vec);
    HTTPResponse response = createResponseAutoindex(dir_vec, path, current_location.path);
    //std::cout << response.toString() << std::endl;
    return (response);
}
