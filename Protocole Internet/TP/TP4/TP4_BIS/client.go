package main

import (
	"bytes"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"time"
)

const serverURL = "https://localhost:8444/get-token"
const serverURLAuth = "https://localhost:8444/top-secret"

type Credentials struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

// Configuration du client pour ignorer la verification TLS
func createClient() *http.Client {
	transport := &*http.DefaultTransport.(*http.Transport)
	transport.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	return &http.Client{
		Transport: transport,
		Timeout:   50 * time.Second,
	}
}

func sendPostRequest(client *http.Client) {
	obejtCredental := Credentials{
		Username: "Damian",
		Password: "Rosebud",
	}
	byteObjet, err := json.Marshal(obejtCredental)
	if err != nil {
		log.Fatalf("Erreur pour sérialisation JSON : %v", err)
	}
	lecteurDonnée := bytes.NewBuffer(byteObjet)
	reponse, err := client.Post(serverURL, "application/json", lecteurDonnée)
	if err != nil {
		log.Fatalf("Erreur de l'envoi de la requête POST au serveur : %v", err)
	}
	defer reponse.Body.Close()

	if reponse.StatusCode != http.StatusOK {
		log.Fatalf("Le serveur n'a pas répondu correctement. Code d'erreur : %v", reponse.StatusCode)
	}

	jsonBody, err := io.ReadAll(reponse.Body)
	if err != nil {
		log.Fatalf("Impossible de lire le corps de la réponse JSON : %v", err)
	}
	//log.Printf("Jeton reçu (brut) : %s", jsonBody)
	var data map[string]string // Nous supposons que la réponse est {"token": "value"}
	errr := json.Unmarshal(jsonBody, &data)
	if errr != nil {
		log.Fatalf("Impossible de décoder le contenu JSON : %v", errr)
	}

	token := data["token"]
	log.Printf("Jeton extrait : %s", token)
	requeteSecrete, err := http.NewRequest("GET", serverURLAuth, nil)
	if err != nil {
		log.Fatalf("Impossible de creer la requet GET a cause du code d'erreur %v", err)
	}
	authHeaderValue := fmt.Sprintf("Bearer %s", token)
	requeteSecrete.Header.Set("Authorization", authHeaderValue)

	reponseSecret, err := client.Do(requeteSecrete)
	if err != nil {
		log.Fatalf("Erreur concernant la requete du client a cause du code d'erreur %v\n", err)
	}
	defer reponseSecret.Body.Close()
	reponseBody, err := io.ReadAll(reponseSecret.Body)

	if err != nil {
		log.Fatalf("Impossible de lire le corps de la reponse du serveur a cause de l'erreur %v\n", err)
	}

	if reponseSecret.StatusCode != http.StatusOK {
		log.Fatalf("Accés refugé: status:%v, reponse: %v", reponseSecret.StatusCode, reponseBody)
	}
	log.Println("Accés autorisé et le secret est:")
	log.Println(string(reponseBody))
}

func main() {
	client := createClient()
	sendPostRequest(client)
}
