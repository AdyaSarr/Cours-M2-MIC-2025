package main

import (
	"fmt"
	"log"
	"net/http"
	"time"

	"github.com/gorilla/websocket"
)

const listenAddr = "localhost:8080"

var upgrader = websocket.Upgrader{} // Utiliser les paramètres par défaut

// --- Structures de Messages du TP 6 (Exercice 2) ---

type chatMessage struct {
	Id   string    `json:"id,omitempty"`
	Time time.Time `json:"time,omitempty"`
	Body string    `json:"body"`
}

type jsonMessage struct {
	Type     string        `json:"type"`
	Message  *chatMessage  `json:"message,omitempty"` // Pointeur!
	Messages []chatMessage `json:"messages,omitempty"`
	Count    int           `json:"count,omitempty"`
	Error    string        `json:"error,omitempty"`
}

// Simule des messages historiques à envoyer lors de la requête "get"
var historicalMessages = []chatMessage{
	{Id: "1", Time: time.Now().Add(-10 * time.Second), Body: "Bienvenue sur le serveur simulé."},
	{Id: "2", Time: time.Now(), Body: "Ceci est un message historique."},
	{Id: "3", Time: time.Now(), Body: "Adya SARR."},
	{Id: "4", Time: time.Now(), Body: "Adya SARR et NCG."},
}

// Handler principal pour les connexions WebSocket
func handleWS(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("Upgrade failed:", err)
		return
	}
	defer conn.Close()
	log.Println("Client connecté.")

	// --- Variables pour simuler l'abonnement ---
	// Nous utilisons un canal pour envoyer des événements asynchrones au client
	// Pour cet exercice, nous allons envoyer un événement après le 'subscribe'.

	// Boucle de lecture des messages du client
	for {
		var clientRequest jsonMessage
		err := conn.ReadJSON(&clientRequest)
		if err != nil {
			if websocket.IsCloseError(err, websocket.CloseNormalClosure, websocket.CloseGoingAway) {
				log.Println("Client déconnecté (fermeture normale)")
			} else {
				log.Println("Erreur de lecture du client:", err)
			}
			break
		}

		log.Printf("Requête reçue: Type=%s, Count=%d", clientRequest.Type, clientRequest.Count)

		// --- LOGIQUE DE SIMULATION ---

		if clientRequest.Type == "get" || clientRequest.Type == "subscribe" {
			// Réponse historique (MessagesReply)
			count := clientRequest.Count
			if count > len(historicalMessages) {
				count = len(historicalMessages)
			}
			response := jsonMessage{
				Type:     "messages",
				Messages: historicalMessages[:count],
			}

			err := conn.WriteJSON(response)
			if err != nil {
				log.Println("Erreur d'écriture de réponse:", err)
				break
			}
			log.Println("Réponse 'messages' (Historique/Abonnement) envoyée.")

			// Simuler un événement asynchrone UNIQUEMENT après l'abonnement
			if clientRequest.Type == "subscribe" {
				// Lancer une goroutine pour simuler l'arrivée d'un nouveau message
				go func() {
					time.Sleep(2 * time.Second) // Attendre 2 secondes
					eventMsg := &chatMessage{
						Id:   "100",
						Time: time.Now(),
						Body: "Ceci est un ÉVÉNEMENT ASYNCHRONE après abonnement.",
					}
					event := jsonMessage{
						Type:    "message",
						Message: eventMsg,
					}
					// Le serveur envoie l'événement non sollicité
					if err := conn.WriteJSON(event); err != nil {
						log.Println("Erreur d'envoi d'événement:", err)
					} else {
						log.Println("Événement 'message' asynchrone envoyé.")
					}
				}()
			}

		} else if clientRequest.Type == "post" && clientRequest.Message != nil {
			// Simuler l'arrivée d'un message posté par le client lui-même
			newMsg := *clientRequest.Message
			newMsg.Time = time.Now()
			newMsg.Id = fmt.Sprintf("%d", len(historicalMessages)+1)
			historicalMessages = append(historicalMessages, newMsg)

			// Simuler la réponse 'ok'
			conn.WriteJSON(jsonMessage{Type: "ok"})
			log.Printf("Message posté: %s", newMsg.Body)

		} else if clientRequest.Type == "ping" {
			conn.WriteJSON(jsonMessage{Type: "ok"})
			log.Println("Réponse 'ok' envoyée pour la requête ping.")
		} else {
			// Simuler une réponse d'erreur pour les requêtes inconnues
			errorResponse := jsonMessage{
				Type:  "error",
				Error: fmt.Sprintf("Unknown request type: %s", clientRequest.Type),
			}
			conn.WriteJSON(errorResponse)
		}
	}
}
func main() {
	// Créer un Mutex pour l'écriture simultanée si nous ajoutions plus de logique d'écriture asynchrone
	// Cependant, pour ce simulateur simple, nous le laissons de côté.

	http.HandleFunc("/chat/ws", handleWS)
	log.Printf("Serveur WebSocket démarré sur ws://%s/chat/ws", listenAddr)

	// Démarrer le serveur HTTP
	err := http.ListenAndServe(listenAddr, nil)
	if err != nil {
		log.Fatal("ListenAndServe:", err)
	}
}
