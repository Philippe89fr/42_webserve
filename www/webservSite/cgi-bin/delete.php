<?php
echo "<!DOCTYPE html>
<html lang=\"fr\">
<head>
<meta charset=\"UTF-8\">
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
<title>Methode DELETE</title>
<style>
body {
font-family: Arial, sans-serif;
max-width: 500px;
margin: 0 auto;
padding: 20px;
}
h1 {
color: #333;
text-align: center;
}
.file-manager {
background-color: #f5f5f5;
border-radius: 8px;
padding: 15px;
box-shadow: 0 2px 5px rgba(0,0,0,0.1);
}
select {
width: 100%;
padding: 8px;
margin-bottom: 15px;
border: 1px solid #ddd;
border-radius: 4px;
}
button {
padding: 8px 15px;
background-color: #f44336;
color: white;
border: none;
border-radius: 4px;
cursor: pointer;
}
button:hover {
background-color: #d32f2f;
}
</style>
</head>
<body>
<h1>Gestionnaire de fichiers Uploads</h1>
<div class=\"file-manager\">
<form id=\"deleteForm\" onsubmit=\"return confirmDelete()\">
<select id=\"fileSelect\" name=\"filename\" required>
<option value=\"\">-- Choisir un fichier à supprimer --</option>";
$files = glob("./www/webservSite/Uploads" . "/*", GLOB_BRACE);
foreach ($files as $file)
{
	if (is_file($file))
	{
	echo '<option value="' . "Uploads/" . basename($file) . '">' . basename($file) . '</option>';
	}
}
echo "
</select>
<button type=\"submit\">Supprimer le fichier</button>
</form>
</div>
<br><br>
<a href=\"../index.html\" class=\"back-link\">Retour à la page principale</a>
<script>
function confirmDelete() {
var fileSelect = document.getElementById(\"fileSelect\");
var filename = fileSelect.value;
if (!filename) {
alert(\"Veuillez sélectionner un fichier.\");
return false;
}
if (confirm(\"Êtes-vous sûr de vouloir supprimer \" + filename + \" ?\")) {
var xhr = new XMLHttpRequest();
xhr.open(\"DELETE\", \"/\" + filename, true);
xhr.onload = function() {
document.documentElement.innerHTML = xhr.responseText;
};
xhr.onerror = function() {
alert(\"Erreur réseau lors de la tentative de suppression.\");
};
xhr.send();
}
return false;
}
</script>
</body>
</html>";
?>
