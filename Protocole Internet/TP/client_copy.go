package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"
	"time"
)

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
	if err != nil {
		log.Fatalf("Problème de lecture des données reçues. \n%s\n", err)
	}

	response.Body.Close()

	return string(body)
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
