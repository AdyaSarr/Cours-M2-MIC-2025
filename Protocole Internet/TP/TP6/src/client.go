package main

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strings"
	"time"

	"github.com/gorilla/websocket"
)

const urlServer = "ws://localhost:8080/chat/ws"

type chatMessage struct {
	Id   string    `json:"id,omitempty"`
	Time time.Time `json:"time,omitempty"`
	Body string    `json:"body"`
}
type jsonMessage struct {
	Type     string        `json:"type"`
	Message  *chatMessage  `json:"message,omitempty"`
	Messages []chatMessage `json:"messages,omitempty"`
	Count    int           `json:"count,omitempty"`
	Error    string        `json:"error,omitempty"`
}

// Comme ReadJSON est blocante je vais definir un canal de lecture pour apres utiliser select
type ReadResult struct {
	Response jsonMessage // La structure décodée
	Err      error       // L'erreur de lecture, si elle existe
}
type WriteResult struct {
	MsgWrite chatMessage
	Err      error
}

//Creer un canal pour recevoir les resultats de la lecturre de websocket

func startReader(conn *websocket.Conn, resultChannel chan ReadResult) {
	var reponse jsonMessage
	err := conn.ReadJSON(&reponse)
	resultChannel <- ReadResult{
		Response: reponse,
		Err:      err,
	}
}

func keyboardMessage(resultChannel chan WriteResult) {
	reader := bufio.NewReader(os.Stdin)
	var resultRead chatMessage
	for {
		fmt.Printf("\nEntrez votre message.\n")
		textLu, err := reader.ReadString('\n')
		/* if err !=nil {
			if err == io.EOF {
				break
			}
			log.Printf("Erreur de lecture du clavier %v\n", err)
			continue
		} */
		textLu = strings.TrimSpace(textLu)
		resultRead.Body = textLu
		resultRead.Time = time.Time{}.UTC()
		if len(textLu) > 0 {
			resultChannel <- WriteResult{
				MsgWrite: resultRead,
				Err:      err,
			}
		}
	}
}

func connexionWebsocket() {
	conn, reponse, err := websocket.DefaultDialer.Dial(urlServer, nil)
	if err != nil {
		log.Fatalf("La connexion au websocket a echoué: %v\n", err)
	}

	/* bodyReponse, err := io.ReadAll(reponse.Body)
	if err != nil {
		log.Fatalf("Erreur sur la lecture du corps de la reponse: %v\n", err)
	} */
	defer reponse.Body.Close()
	if reponse.StatusCode != http.StatusSwitchingProtocols {
		log.Fatalf("Erreur : la réponse du serveur n'a pas le status 101 Switching Protocols mais %v\n", reponse.StatusCode)
	}
	defer conn.Close()
	var jsonMessageSend = jsonMessage{
		//Type:  "get",
		Type:  "subscribe",
		Count: 20,
	}
	var pingRequest = jsonMessage{
		Type: "ping",
	}
	intervalle20s := time.Duration(20) * time.Second
	intervalle35s := time.Duration(35) * time.Second
	timer20s := time.NewTimer(intervalle20s)
	timer35s := time.NewTimer(intervalle35s)
	defer timer20s.Stop()
	defer timer35s.Stop()
	err = conn.WriteJSON(jsonMessageSend)
	if err != nil {
		log.Fatalf("Erreur sur l'envoie du message a cause de: %v\n", err)
	}
	readChannel := make(chan ReadResult)
	writeChannel := make(chan WriteResult)
	go startReader(conn, readChannel)
	go keyboardMessage(writeChannel)
	for {
		select {
		case result := <-readChannel:
			if result.Err != nil {
				//Gerer la fermeture normale
				if websocket.IsCloseError(result.Err, websocket.CloseNormalClosure, websocket.CloseGoingAway) {
					log.Printf("Le serveur a fermé la connexion normalement\n")
					break
				}
				//Gerer la fermeture brutale
				if result.Err == io.EOF {
					log.Printf("Le serveur a fermé la connexion de maniere inatendu\n")
					break
				}
				log.Printf("Erreur inatendu de la part du serveur: %v", result.Err)
				break
			}
			timer20s.Reset(intervalle20s)
			timer35s.Reset(intervalle35s)
			if result.Response.Type == "messages" {
				//La reponse a la requete get
				for _, msg := range result.Response.Messages {
					fmt.Printf(" [HIST] %s: %s\n", msg.Time.Format("15:04:05"), msg.Body)
				}
			} else if result.Response.Type == "message" {
				//une notification asynchrone
				if result.Response.Message != nil {
					fmt.Printf(" [NOUVEAU] %s: %s\n", result.Response.Message.Time.Format("15:04:05"), result.Response.Message.Body)
				}
			} else if result.Response.Type == "error" {
				fmt.Printf(" [ERREUR] %s\n", result.Response.Error)
			} else if result.Response.Type == "ok" {
				log.Printf(" [OK] Requête terminée avec succès.")
			} else {
				fmt.Printf(" [INCONNU] Type de message inconnu: %s\n", result.Response.Type)
			}
			go startReader(conn, readChannel)
		case result := <-writeChannel:
			if result.Err != nil {
				if result.Err == io.EOF {
					break
				}
				log.Printf("Erreur de lecture du clavier: %v\n", result.Err)
			}
			timer20s.Reset(intervalle20s)
			timer35s.Reset(intervalle35s)
			var jsonmsg jsonMessage
			jsonmsg.Message = &result.MsgWrite
			jsonmsg.Type = "post"

			err := conn.WriteJSON(jsonmsg)
			if err != nil {
				log.Printf("Erreur lors de l'envoie du message tapé sur le clavier: %v\n", err)
				break
			}
		case <-timer20s.C:
			err := conn.WriteJSON(pingRequest)
			if err != nil {
				log.Printf("Erreur lors de l'envoie du message ping au serveur: %v\n", err)
				break
			}
			timer20s.Reset(intervalle20s)
		case <-timer35s.C:
			log.Printf("Expiration du timer deconnection\n")
			conn.Close()
		}
	}
}
func main() {
	connexionWebsocket()
}
