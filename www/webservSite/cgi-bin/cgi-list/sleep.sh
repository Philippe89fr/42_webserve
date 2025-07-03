# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    sleep.sh                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: prambaud <prambaud@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/23 18:35:49 by mgalvez           #+#    #+#              #
#    Updated: 2025/06/02 17:23:29 by mgalvez          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/bin/sh

sleep 10

echo "<!DOCTYPE html>
<html lang=\"fr\">
<head>
<meta charset=\"UTF-8\">
<meta name =\"viewport\" content=\"width=device-width, initial-scale=1.0\">
<title>SLEEP CGI</title>
</head>
<body>
<h1>SLEEP CGI</h1>
<p>Congratulation! You've waited 10 seconds!</p>
<br><br>
<a href=\"../CGI.php\" class=\"back-link\">Retour à l'index des CGIs</a>
<br><br>
<a href=\"../../index.html\" class=\"back-link\">Retour à la page principale</a>
</body>
</html>"
