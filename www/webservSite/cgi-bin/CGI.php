<?php

echo "<!DOCTYPE html>
<html lang=\"fr\">
<head>
<meta charset=\"UTF-8\">
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
<title>CGI menu</title>
</head>
<body>
<h1>CGI list:</h1>";
$files = glob("./www/webservSite/cgi-bin/cgi-list" . "/*", GLOB_BRACE);
foreach ($files as $file)
{
	if (is_file($file))
	{
	echo '<p><a href="cgi-list/' . basename($file) . '">' . basename($file) . '</a></p>';
	}
}
echo "<br><br>
<a href=\"../index.html\" class=\"back-link\">Retour à la page principale</a>
</body>
</html>";

?>
