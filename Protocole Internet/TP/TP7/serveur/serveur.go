package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"net"
)

const listenPort = 8083
const maxPacketSize = 1024 // Taille maximale de datagramme que nous allons gérer
const quotationType = 128  // Type pour la réponse "quotation"
const errorType = 129      // Type pour la réponse "error"

var quotations = []string{
	"La seule façon de faire du bon travail est d'aimer ce que vous faites. — Steve Jobs",
	"Visez la lune. Au moins, si vous échouez, vous finirez dans les étoiles. — Oscar Wilde",
	"L'imagination est plus importante que le savoir. — Albert Einstein",
}

// NOTE: Le code initial avait une vérification inutile: if len(requestBody) < 0 { return }. Je l'ai supprimé.

// Décode et exécute la logique du protocole UDP
func handleRequest(conn *net.UDPConn, addr *net.UDPAddr, data []byte) {
	if len(data) < 7 {
		log.Printf("Paquet trop court reçu de %s", addr.String())
		return
	}

	// 1. Décoder l'en-tête
	requestID := binary.BigEndian.Uint32(data[0:4])
	requestType := data[4]
	requestLength := binary.BigEndian.Uint16(data[5:7])
	// Le corps n'est pas utilisé pour la logique de base du TP, mais c'est bien de le garder
	// requestBody := data[7:]

	// 2. Vérification de l'intégrité du paquet
	expectedLength := 7 + int(requestLength)
	if len(data) < expectedLength {
		log.Printf("Paquet corrompu (longueur incorrecte) reçu de %s", addr.String())
		return
	}

	log.Printf("Requête #%d reçue (Type: %d, Longueur: %d)", requestID, requestType, requestLength)

	// 3. Logique du Protocole (Type 0 = get-quotation)
	if requestType == 0 { // get-quotation (Body doit être vide)
		if requestLength != 0 {
			// Erreur: Body devait être vide
			sendError(conn, addr, requestID, "Le corps de la requête get-quotation doit être vide.")
			return
		}

		// 3.1. Préparer la réponse (quotationType = 128)
		index := int(requestID) % len(quotations) // Sélection simple
		responseBody := []byte(quotations[index])

		sendResponse(conn, addr, requestID, quotationType, responseBody)

	} else {
		// Requête de Type Inconnu
		sendError(conn, addr, requestID, fmt.Sprintf("Type de requête inconnu: %d", requestType))
	}
}

// Fonction utilitaire pour envoyer une réponse UDP formatée
func sendResponse(conn *net.UDPConn, addr *net.UDPAddr, id uint32, msgType byte, body []byte) {
	responseLength := len(body)
	responsePacket := make([]byte, 7+responseLength)

	// 1. Écrire l'ID (4 octets)
	binary.BigEndian.PutUint32(responsePacket[0:4], id)

	// 2. Écrire le Type (1 octet)
	responsePacket[4] = msgType

	// 3. Écrire la Longueur (2 octets)
	binary.BigEndian.PutUint16(responsePacket[5:7], uint16(responseLength))

	// 4. Écrire le Corps
	copy(responsePacket[7:], body)

	// Envoyer le datagramme
	conn.WriteToUDP(responsePacket, addr)
	log.Printf("Réponse #%d envoyée à %s (Type: %d)", id, addr.String(), msgType)
}

// Fonction utilitaire pour envoyer une réponse d'erreur (Type 129)
func sendError(conn *net.UDPConn, addr *net.UDPAddr, id uint32, errMsg string) {
	sendResponse(conn, addr, id, errorType, []byte(errMsg))
}

func main() {
	// 1. Écouter sur l'interface locale (UDP)
	udpAddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf(":%d", listenPort))
	if err != nil {
		log.Fatalf("Erreur de résolution d'adresse UDP: %v", err)
	}

	conn, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		log.Fatalf("Erreur de l'écoute UDP: %v", err)
	}
	defer conn.Close()

	log.Printf("Serveur UDP simulateur démarré sur :%d", listenPort)

	// 2. Boucle principale de réception des datagrammes
	buffer := make([]byte, maxPacketSize)

	for {
		n, addr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Erreur de lecture UDP: %v", err)
			continue
		}

		// --- CORRECTION DE CONCURRENCE ---
		// Créer une copie locale du buffer pour la goroutine
		dataCopy := make([]byte, n)
		copy(dataCopy, buffer[:n])

		// Gérer la requête dans une goroutine
		go handleRequest(conn, addr, dataCopy)
	}
}
