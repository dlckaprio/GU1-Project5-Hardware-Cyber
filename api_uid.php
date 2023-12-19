<?php

// Paramètres de connexion à la base de données
$host = "127.0.0.1:3306";
$user = "u836478346_agagliardi";
$pass = "Adminzobmou69@";
$db = "u836478346_fichiers";

$conn = new mysqli($servername, $username, $password, $dbname);

// Vérifier la connexion
if ($conn->connect_error) {
    die("Échec de la connexion à la base de données: " . $conn->connect_error);
}

// Récupérer l'ID de la carte depuis la requête GET
$cardUid = $_GET["uid"];

// Requête SQL pour récupérer le statut de la carte
$sql = "SELECT id FROM badges_lciurezu WHERE uid = '$cardUid'";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    // La carte est dans la base de données

    // Envoyer la réponse JSON à l'Arduino
    echo json_encode(array("recognized" => 1));
} else {
    // La carte n'est pas dans la base de données
    echo json_encode(array("recognized" => 0));
}

// Fermer la connexion à la base de données
$conn->close();
?>