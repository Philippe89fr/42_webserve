<?php

echo "<!DOCTYPE html>
<html lang=\"fr\">
<head>
<meta charset=\"UTF-8\">
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
<title>Date CGI</title>
</head>
<body>
<h1>This page was generated at:</h1>";
echo '<p>' . date('l jS \of F Y H:i:s') . '</p>';
echo "<br><br>
<a href=\"../CGI.php\" class=\"back-link\">Retour à l'index des CGIs</a>
<br><br>
<a href=\"../../index.html\" class=\"back-link\">Retour à la page principale</a>
</body>
</html>";

?>
