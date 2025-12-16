package main

import (
	"crypto/tls"
	"fmt"
	"io"
	"log"
	"net/http"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/jech/cert"
)

// Structure de données du chat
type ChatMessage struct {
	Body      string
	Timestamp time.Time
}

var (
	//Stockage des messages avec un map et chaque message sera identifié par un unique enteir sur le map (le dictionnaire)
	messages = make(map[int]ChatMessage)
	//besoin d'un verrou pour securise l'acces rendre  concurrente les requestes: mecanisme de synchronisation quand le serveur recoit
	//des reequets de plusieur clients
	mutex sync.Mutex
	//Compteur pour l'identifiant unique du prochain message
	nextID = 1
)

func main() {
	http.HandleFunc("/chat/", chatHandler) //Gestionnaire d'un handler pour tous les routes /chat/*

	certificate := cert.New("certFile", "keyFile")

	//onfiguration du serveur
	s := http.Server{
		Addr: ":8443", // definir le port HTTPS
		//Configuration de la couche TLS
		TLSConfig: &tls.Config{
			GetCertificate: func(hello *tls.ClientHelloInfo) (*tls.Certificate, error) {
				return certificate.Get()
			},
		},
	}
	// Initialiser quelques messages pour les tests
	messages[1] = ChatMessage{Body: "Bienvenue sur le chat local!", Timestamp: time.Now().Add(-5 * time.Minute).UTC()}
	nextID = 2 // Commence le compteur après les messages initiaux
	log.Println("Serveur chat local démarré sur : https://localhost:8443/chat/")
	// Pour l'Exercice 3, vous devrez utiliser l'option -k avec curl, ou configurer le client Go
	// pour ignorer la vérification TLS[cite: 117, 118, 119].
	err := s.ListenAndServeTLS("", "")
	log.Fatal("ListenAndServeTLS: ", err)
}

func chatHandler(w http.ResponseWriter, r *http.Request) {
	pathSegments := strings.Split(strings.Trim(r.URL.Path, "/"), "/") // on recuperer l'url de la requete  puis on lui ajoute "/"

	//Maintenant on va verifier si l'url contient uniquement /chat/ ou /chat/Id
	if len(pathSegments) == 1 && pathSegments[0] == "chat" {
		//Je vais recuperer tous les requetes de chat
		handleChatCollection(w, r)
	} else if len(pathSegments) == 2 && pathSegments[0] == "chat" {
		//Je lance la requete uniquement sur /chat/id
		handleChatMessage(w, r, pathSegments[1])
	} else {
		http.Error(w, "Not found", http.StatusNotFound)
	}
}

// Maintenant on va gerer les requetes GET et POST sur /chat/
func handleChatCollection(w http.ResponseWriter, r *http.Request) {
	mutex.Lock()         //demande le systeme d'avoir exclusivement acces a cette variable
	defer mutex.Unlock() // pour liberer le verrou a la fin du handle

	switch r.Method {
	case "GET":
		//a GET request to the URL /chat/ returns the list of identifiers of chat messages, one per line;
		w.Header().Set("Content-Type", "text/html; charset=utf-8")
		for id := range messages {
			fmt.Fprintln(w, id)
		}
	case "POST":
		//a POST request to the URL /chat/ creates a new message and returns its identifier ;
		body_est_message, err := io.ReadAll(r.Body)
		if err != nil {
			http.Error(w, "Cannot Read Message Body", http.StatusInternalServerError)
			return
		}
		body := string(body_est_message)

		newMessage := ChatMessage{
			Body:      body,
			Timestamp: time.Now().UTC(),
		}
		id := nextID
		messages[id] = newMessage
		nextID++

		w.Header().Set("Content-Type", "text/html; charset-8")
		fmt.Fprintf(w, "%d\n", id)
	default:
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
	}

}

func handleChatMessage(w http.ResponseWriter, r *http.Request, idString string) {
	messageID, err := strconv.Atoi(idString)

	if err != nil {
		http.Error(w, "Invalid message Id", http.StatusBadRequest)
		return
	}

	mutex.Lock()
	defer mutex.Unlock()

	message, found := messages[messageID]

	if !found {
		http.Error(w, "Message not found", http.StatusNotFound)
		return
	}
	//Generer un Etag basé sur l'horloge du message
	etagValue := message.Timestamp.Format(http.TimeFormat)
	switch r.Method {
	case "GET":
		//Verfier l'entete If-None-Match du client
		etagClient := w.Header().Get("If-None-Match")

		//Si l'Etag du client correspond a celui du serveur alors le message est inchange
		if etagClient == etagValue {
			w.Header().Set("ETag", etagValue)
			w.WriteHeader(http.StatusNotModified)
			return
		}
		w.Header().Set("Content-Type", "text/html; charset =utf-8")
		w.Header().Set("Last-Modified", etagValue)
		w.Header().Set("ETag", etagValue)
		fmt.Fprintf(w, message.Body)
	case "DELETE":
		delete(messages, messageID)
		w.WriteHeader(http.StatusNoContent)
	default:
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
	}
}
