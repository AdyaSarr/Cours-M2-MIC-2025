package main

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"math/rand/v2"
	"net"
	"net/http"
	"strconv"
	"sync"
	"time"
)

const urlSrvcount = "https://localhost:8080/peers/?count"
const SIZEMAXDATAGRAM = 1024
const AddrIPv4Client = "127.0.0.1:8081"

type Dispatcher struct {
	sync.RWMutex     // Protéger la map contre les accès concurrents
	responseChannels map[uint32]chan []byte
}

var resultReqOrRep Dispatcher

type AddrSocket struct {
	IP   string
	Port uint16
}

type StatusPeer uint8

const (
	ONLINE StatusPeer = iota
	OCCUPY
	DECONNECT
)

type InfoPerrs struct {
	Nickname  string
	Token     string
	AddrSoc   AddrSocket
	PublicKey string
	Status    StatusPeer
}

type BodyMessage struct {
	Message_Id uint32
	Message    []byte
}

//=========================================================================================================================
//											Partie client-serveu
//=========================================================================================================================

func obtainListPeersAvailable(client *http.Client, urlcount string, nbPeers int) ([]InfoPerrs, error) {
	//Prepartion de l'envoie de la requete GET
	urlSrv := fmt.Sprintf(urlcount + "=" + strconv.Itoa(nbPeers))
	preapreRequete, err := http.NewRequest("GET", urlSrv, nil)
	if err != nil {
		return nil, fmt.Errorf("Erreur de preparation de la requete GET (http.NewReques): %v", err)
	}
	sendRequest, err := client.Do(preapreRequete)
	if err != nil {
		return nil, fmt.Errorf("Erreur d'envoie de la requete au client (client.Do): %v", err)
	}
	defer sendRequest.Body.Close()
	if sendRequest.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("Erreur le serveur n'a pas repondu avec le status 200 ok mais: %v", sendRequest.StatusCode)
	}
	bodyResponse, err := io.ReadAll(sendRequest.Body)
	if err != nil {
		return nil, fmt.Errorf("Erreur sur la lecture de la reponse du serveur (io.ReadAll): %v", err)
	}
	var resultPeers []InfoPerrs
	//deserialiser la reponse
	err = json.Unmarshal(bodyResponse, &resultPeers)
	if err != nil {
		return nil, fmt.Errorf("Erreur de la deserialisation du json de la reponse (json.Unmarshal): %v", err)
	}
	return resultPeers, nil
}

// =========================================================================================================================
//
//	Partie Pair-a-Pair
//
// =========================================================================================================================
func serialisationStructUDP(Id uint32, Type uint8, Length uint16, Body BodyMessage) []byte {
	//calcul de la taille du datagramme
	sizeDatagram := 4 + 1 + 2 + int(Length)
	//Slice pour le datagramme
	packet := make([]byte, sizeDatagram)

	binary.BigEndian.PutUint32(packet[0:4], Id)
	packet[4] = Type
	binary.BigEndian.PutUint16(packet[5:7], Length)
	binary.BigEndian.PutUint32(packet[7:11], Body.Message_Id)
	copy(packet[11:], Body.Message)
	return packet
}

func buildMessageSendToPeer() (uint32, uint32, []byte) {
	requestId := generateRandomId()
	requestType := uint8(1)
	var requestBody BodyMessage
	requestBody.Message = []byte("Bonjour")
	requestBody.Message_Id = generateRandomId()
	requesteLength := uint16(len(requestBody.Message) + 4)

	datagram := serialisationStructUDP(requestId, requestType, requesteLength, requestBody)
	return requestId, requestBody.Message_Id, datagram
}
func buildPingSendToPeer() (uint32, []byte) {
	requestId := generateRandomId()
	requestType := uint8(0)
	var requestBody BodyMessage
	requestBody.Message = []byte{}
	requesteLength := uint16(len(requestBody.Message))

	datagram := serialisationStructUDP(requestId, requestType, requesteLength, requestBody)
	return requestId, datagram
}
func buildOkSendToPeer(requestId uint32) (uint32, []byte) {
	requestType := uint8(128)
	var requestBody BodyMessage
	requestBody.Message = []byte{}
	requestLength := uint16(len(requestBody.Message))
	datagram := serialisationStructUDP(requestId, requestType, requestLength, requestBody)
	return requestId, datagram
}
func generateRandomId() uint32 {
	return rand.Uint32()
}

func sendRequestToThePeer(conn *net.UDPConn, addrSoc *net.UDPAddr, datagramUDP []byte) (int, error) {
	nbOctet, err := conn.WriteToUDP(datagramUDP, addrSoc)
	if err != nil {
		return 0, fmt.Errorf("Erreur d'envoie du message(UDP:conn.Writ): %v", err)
	}
	return nbOctet, nil
}

func startUDPEarListener(addrSoc *net.UDPAddr) (*net.UDPConn, error) {
	conn, err := net.ListenUDP("udp", addrSoc)
	if err != nil {
		return nil, fmt.Errorf("Erreur d'ecoute du client sur le port %v: %v", addrSoc, err)
	}
	return conn, nil
}

func deserialisationUDP(packet []byte, responseIdServer uint32) (string, error) {
	if len(packet) < 7 {
		return "", fmt.Errorf("Packet trop court (taille %d)", len(packet))
	}
	responseId := binary.BigEndian.Uint32(packet[0:4])
	responseType := packet[4]
	responseLength := binary.BigEndian.Uint16(packet[5:7])
	if len(packet) != (7 + int(responseLength)) {
		return "", fmt.Errorf("Erreur, paquet mal formé: taille reçue %d ne correspond pas à la taille déclarée (%d + %d)",
			len(packet), 7, responseLength)
	}
	if responseId != responseIdServer {
		return "", fmt.Errorf("Erreur, réponse non destinée à la requête #%d (ID reçu: %d)", responseIdServer, responseId)
	}
	// CAS 1: OK / ACK (Type 128)
	if responseType == 128 {
		// Une réponse OK doit avoir un corps de longueur zéro dans ce protocole
		if responseLength == 0 {
			log.Printf("Réponse OK (Type 128) reçue pour la requête #%d", responseId)
			return "", nil // Succès, pas de données à retourner
		} else {
			// C'est une erreur de protocole si ACK contient des données non attendues
			return "", fmt.Errorf("Erreur de protocole: réponse Type 128 (OK) contient des données (%d octets)", responseLength)
		}
	}
	// CAS 2: ERREUR (Type 129)
	if responseType == 129 {
		// Le corps de l'erreur est lu directement à partir de l'index 7 (pas de Message-Id)
		errorMessage := string(packet[7:])
		return "", fmt.Errorf("Erreur du pair distant (Type 129): %s", errorMessage)
	}
	// CAS 3: AUTRES TYPES (ex: Réponse de données si le protocole était plus avancé)
	// Ce bloc est nécessaire si vous attendez une réponse avec Message-Id + Corps
	if responseType == 130 { // Exemple d'un type de réponse pour des données chiffrées/recherches
		// Si le corps est censé contenir le Message-Id (4 octets) + le Message (variable)
		if responseLength < 4 {
			return "", fmt.Errorf("Erreur de protocole: réponse Type %d trop courte pour contenir Message-Id", responseType)
		}

		// Lecture du corps spécifique à ce type
		// responseMessageId := binary.BigEndian.Uint32(packet[7:11])
		// responseMessage := string(packet[11:])
		// log.Printf("Réponse de données reçue (MsgID: %d)", responseMessageId)

		// Pour l'exercice 1.2, ce bloc n'est pas nécessaire, mais il illustre la structure
		return "Paquet de données reçu, mais décodage non implémenté.", nil

	}
	// Type de réponse non reconnu par le client
	return "", fmt.Errorf("Type de réponse inconnu: %d", responseType)
}
func processPacket(conn *net.UDPConn, ReqOrRep []byte, addr *net.UDPAddr) {
	Id := binary.BigEndian.Uint32(ReqOrRep[0:4])
	Type := ReqOrRep[4]
	if Type < 128 {
		log.Printf("C'est une requete entrante")
		_, datagram := buildOkSendToPeer(Id)
		_, err := sendRequestToThePeer(conn, addr, datagram)
		if err != nil {
			log.Printf("Erreur d'envoie du paquet ok a %v", addr)
			return
		}

	} else {
		log.Printf("C'est une reponse d'un autre pair")
		resultReqOrRep.Lock()
		_, ok := resultReqOrRep.responseChannels[Id]
		defer resultReqOrRep.Unlock()
		if !ok {
			log.Printf("L'Id %v non trouvé sur le map", Id)
			return
		}
		resultReqOrRep.responseChannels[Id] <- ReqOrRep
		delete(resultReqOrRep.responseChannels, Id)
	}
}
func startListenerAndDispatcher(conn *net.UDPConn, addrSoc *net.UDPAddr) {
	for {
		bufferRcv := make([]byte, SIZEMAXDATAGRAM)
		nbOctet, addr, err := conn.ReadFromUDP(bufferRcv)
		if err != nil {
			log.Printf("Erreur de lecture du message du pair %v (conn.ReadFromUDP): %v", addr, err)
			continue
		}
		log.Printf("J'ai lu %d octets provenant de %v", nbOctet, addr)
		ReqOrRep := bufferRcv[:nbOctet]
		processPacket(conn, ReqOrRep, addr)
	}
}

func main() {
	client := createClient()
	nbPeers := 6
	resultPeers, err := obtainListPeersAvailable(client, urlSrvcount, nbPeers)
	if err != nil {
		log.Printf("Erreur de recuperation des peers sur le serveur(obtainListPeersAvailable): %v", err)
		return
	}
	resultReqOrRep.responseChannels = make(map[uint32]chan []byte)
	addrSocClient, err := net.ResolveUDPAddr("udp", AddrIPv4Client)
	conn, err := startUDPEarListener(addrSocClient)
	if err != nil {
		log.Fatalf("Erreur d'ecoute sur l'adresse socket %v", addrSocClient)
	}
	go startListenerAndDispatcher(conn, addrSocClient)

	canalAttente := make(chan []byte)
	requestID, _, datagram := buildMessageSendToPeer()
	resultReqOrRep.Lock()
	defer resultReqOrRep.Unlock()
	resultReqOrRep.responseChannels[requestID] = canalAttente
	resultReqOrRep.Unlock()
	if len(resultPeers) > 0 {
		peer, _ := net.ResolveUDPAddr("udp", resultPeers[0].AddrSoc.IP+":"+strconv.Itoa(int(resultPeers[0].AddrSoc.Port)))
		_, err = sendRequestToThePeer(conn, peer, datagram)
	}
	timeOut := time.After(5 * time.Second)
	select {
	case rcvPacket := <-canalAttente:
		data, err := deserialisationUDP(rcvPacket, requestID)
		if err != nil {
			log.Fatalf("Erreur sur la deserialisation de la reponse du pair")
		}
		log.Printf("Le message recu est %v", data)

	case <-timeOut:
		// Timeout expiré, la réponse n'est pas arrivée
		log.Printf("Timeout expiré pour la requête #%d. Nettoyage...", requestID)

		// 3. Nettoyer la map manuellement (le dispatcher ne l'a pas fait)
		resultReqOrRep.Lock()
		delete(resultReqOrRep.responseChannels, requestID)
		resultReqOrRep.Unlock()
	}
}
