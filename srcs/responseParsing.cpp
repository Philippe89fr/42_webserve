/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseParsing.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 13:31:19 by prambaud          #+#    #+#             */
/*   Updated: 2025/06/03 10:41:25 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

/*
1. verifier que le fichier / dossier existe (si non, envoyer res.setStatus(404, "Not Found");
        res.setHeader("Content-Type", "text/plain");
        res.setBody("404 Not Found\n");)
2. verifier que le fichier est lisible, si non envoyer res.setStatus(403, "Forbidden");
        res.setHeader("Content-Type", "text/plain");
        res.setBody("403 Forbidden\n");
3. Si c'est un repertoire, verifier que le fichier existe (path + "/index.html"), sinon 403 erreur si c'est pas ouvrable
4. Sinon, ouvrir le fichier,    if (!file) {
        res.setStatus(500, "Internal Server Error");
        res.setBody("500 Internal Server Error\n");
        return res;
		//
	    res.setStatus(200, "OK");
    res.setHeader("Content-Type", getMimeType(path));
    res.setBody(content);

*/

std::string GetMimeType(const std::string& path)
{
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream"; // Par défaut : binaire
    std::string ext = path.substr(dot + 1);
    if (ext == "html" || ext == "htm")
        return "text/html";
    else if (ext == "css")
        return "text/css";
    else if (ext == "js")
        return "application/javascript";
    else if (ext == "json")
        return "application/json";
    else if (ext == "png")
        return "image/png";
    else if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    else if (ext == "gif")
        return "image/gif";
    else if (ext == "svg")
        return "image/svg+xml";
    else if (ext == "txt")
        return "text/plain";
    else if (ext == "pdf")
        return "application/pdf";
    else if (ext == "ico")
        return "image/x-icon";
    return "application/octet-stream";
}

std::string getHTTPDate() {
    time_t now = time(NULL);                     // Obtenir l'heure actuelle (en secondes)
    struct tm *gmt = gmtime(&now);               // Convertir en UTC (GMT)

    const char* weekdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    const char* months[]   = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    char buffer[100];
    snprintf(buffer, sizeof(buffer),
             "%s, %02d %s %d %02d:%02d:%02d GMT",
             weekdays[gmt->tm_wday],
             gmt->tm_mday,
             months[gmt->tm_mon],
             gmt->tm_year + 1900,
             gmt->tm_hour,
             gmt->tm_min,
             gmt->tm_sec);

    return std::string(buffer);
}

/*void parsHttpBody(char buffer[], HTTPRequest& requestHTTP, int header_size, int byte_read)
{
    std::vector<char> body_vec;
    body_vec.insert(body_vec.end(), buffer + header_size, buffer + byte_read);

    requestHTTP.setbodyVec(body_vec);



    bool HttpBodyIsComplete(char buffer[], HTTPRequest& requestHTTP, int byte_read, int header_size) {
    int contentLength = 0;
    std::vector<char> body_vec;

    //trouver le nombre d'octet avant le buffer
    //body.insert(body.end(), buffer + header_octet, buffer + byte_size - header_octet);

    body_vec.insert(body_vec.end(), buffer + header_size, buffer + byte_read);
    requestHTTP.setbodyVec(body_vec);
    */
void GetGoodResponse(t_client &client, const std::string& path)
{
    std::ifstream file(path.c_str());
    std::vector<char> bodyPart;

    if (!file.is_open()) {
		std::string	err_msg("error 403 : Access denied");
        throw HttpError(err_msg);
    }
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(client.response.getbitsRead()); // Position de départ

    size_t remaining = fileSize - client.response.getbitsRead();
    size_t bytesToRead = std::min(static_cast<size_t>(CHUNKREAD), remaining);
    bodyPart.resize(bytesToRead);
    file.read(&bodyPart[0], bytesToRead);
    size_t actualRead = file.gcount();
    bodyPart.resize(actualRead); // Ajuster à ce qui a vraiment été lu
    client.response.addbody_char(bodyPart);
    client.response.setbitsRead(client.response.getbitsRead() + actualRead);
    if (remaining < CHUNKREAD)
        client.responseReady = true;
    file.close();

    if (!client.responseReady)
        return ;
    std::map<std::string, std::string> yo = client.request.getHeader();
    size_t value = client.response.getbody_char().size();
    std::ostringstream oss;
    oss << value;
    client.response.setbody_char_bool(true);
    client.response.setHeader("Content-Length", oss.str());
	client.response.setStatus(200, "OK");
    client.response.setHeader("Server", "Webserv/1.0");
    client.response.setHeader("Date", getHTTPDate());
    client.response.setHeader("Content-Type", GetMimeType(path));
    client.response.setHeader("Location", path);
    if (yo["Connection"] == "keep-alive" || (client.request.getVersion() == "HTTP/1.1" && yo["Connection"] != "close"))
    {
        client.response.setHeader("Connection", "keep-alive");
    }
    else if (yo["Connection"] == "close" || client.request.getVersion() == "HTTP/1.0")
    {
        client.response.setHeader("Connection", "close");
    }
    return ;
}

void GetResponse(t_client &client, const std::string& path)
{
	HTTPResponse response;
	struct stat info;

	//1.
	if (stat(path.c_str(), &info) < 0)
	{
		std::string	err_msg("error 404 : Resource not found");
		throw HttpError(err_msg);
	}
	//3.
	std::string indexPath = path;
	if (S_ISDIR(info.st_mode))//test si le path est un repertoire ou un file.
		std::string indexPath = path + "/index.html";
	std::ifstream file(indexPath.c_str());
    if (!file.is_open())
	{
		std::string	err_msg("Error 403 : Forbidden");
		throw HttpError(err_msg);
	}
    GetGoodResponse(client, indexPath);
	return ;
}

// HTTPResponse PostRequest(std::map<std::string, std::string> headers, const std::string& body)
// {
// 	HTTPResponse	response;
// 	//struct stat		info;

//     response.setBody(body);
//     response.setHeader(headers);
// 	if (headers["Content-type"] == "application/json")
// 	{
//         response.setBodyparsed(parse_Json(body));
// 	}
// 	else if (headers["Content-type"] == "application/x-www-form-urlencoded")
// 	{
//         response.setBodyparsed(parse_urlencoded(body));
// 	}
// 	else if (headers["Content-type"] == "multipart/form-data")
// 	{

// 	}
// 	else
//     {

//     }

// 	return (response); //??
// }

/*
Differents exemples des body a parser en fonction du type :

Content-Type: application/x-www-form-urlencoded
username=alice&age=23
Parser avec std::string::find('&') et std::string::find('=')
Décoder les caractères encodés (%20 → espace)

email=alice%40mail.com&username=alice&age=30&country=France
alice@mail.com
name=Jean+Dupont&subject=Demande+d%27infos&message=Bonjour%2C+je+veux+plus+d%27infos.
name = Jean Dupont
subject = Demande d'infos
message = Bonjour, je veux plus d'infos.
donc %qqch : qqch c'est en hexa et ca designe l'ascii d'un caractere special
+ = ' ';







Content-Type: application/json
{
  "username": "bob",
  "age": 30
}
  //
{
  "order_id": "A1234",
  "items": ["book", "pen"],
  "total": 19.99,
  "shipping": {
    "name": "John Doe",
    "address": "123 Street"
  }
}






Content-Type: text/plain
Rien a faire, prendre le texte tel quel.







Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryX7x7
------WebKitFormBoundaryX7x7
Content-Disposition: form-data; name="username"

alice
------WebKitFormBoundaryX7x7
Content-Disposition: form-data; name="file"; filename="img.jpg"
Content-Type: image/jpeg

(binary image data)
------WebKitFormBoundaryX7x7--

Trouver chaque part (en fonction du boundary)
Pour les fichiers : lire le filename, Content-Type et stocker les données
Il faut parfois gérer \r\n à la fin de chaque ligne
c'est mega le bordel!






Content-Type: application/octet-stream
Lire exactement Content-Length octets
Écrire tel quel dans un fichier // rien a faire
*/
