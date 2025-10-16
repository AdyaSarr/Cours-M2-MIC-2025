package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"
	"time"
)

type chatMessage struct {
	Id   string    `json:"id,omitempty"`
	Time time.Time `json:"time,omitempty"`
	Body string    `json:"body"`
}

const URL_ENDPOINT = "https://jch.irif.fr:8443"

func getEndpoint(endpoint string) string {
	response, err := http.Get(URL_ENDPOINT + endpoint)

	if err != nil {
		log.Fatalf("Erreur de lecture du serveur de chat.\n%s\n", err)
	}

	if response.StatusCode > 299 {
		fmt.Printf("Code d'erreur reçu: %d\n", response.StatusCode)
	}

	body, err := io.ReadAll(response.Body)

	var values chatMessage
	err = json.Unmarshal(body, &values.Body)
	if err != nil {
		log.Fatalf("Probleme de parsing")
	}
	log.Printf("value: %#v", values)

	response.Body.Close()

	return string(values.Body)
}

func main() {
	var oldCount = 0

	for {
		// Grabs the list of messages from the response text
		var messages = strings.Split(getEndpoint("/chat"), "\n")

		for k, v := range messages {
			if v == "" || k < oldCount-1 {
				continue
			}

			fmt.Printf("%s\n", getEndpoint("/chat/"+v))
		}

		oldCount = len(messages)

		time.Sleep(10 * time.Second)
	}
}
