package main

import (
	"encoding/binary"
	"fmt"
	"log"
	"math"
	"math/rand"
	"net"
	"time"
)

const urlSignComm = "https://jch.irif.fr:8443/udp-addresses.json"
const sizeMaxDatagram = 1024
const defaultPeriod = 5 * time.Second

type ProbeResult struct {
	Addr     string  // L'adresse qui a répondu
	RTT      float64 // Le temps de réponse (tau)
	Response []byte  // Le paquet de réponse reçu
	Error    error   // L'erreur, si l'envoi/lecture a échoué
}

// const addrPort = "127.0.0.1:8081"
func generateRandomId() uint32 {
	return rand.Uint32()
}
func serialisationStructUDP(Id uint32, Type uint8, Length uint16, Body []byte) []byte {
	//calcul de la taille du datagramme
	sizeDatagram := 4 + 1 + 2 + len(Body)
	//Slice pour le datagramme
	packet := make([]byte, sizeDatagram)

	binary.BigEndian.PutUint32(packet[0:4], Id)
	packet[4] = Type
	binary.BigEndian.PutUint16(packet[5:7], Length)
	copy(packet[7:], Body)
	return packet
}

/* func recupAdrresServeur() ([]string, error) {
	response, err := http.Get(urlSignComm)
	if err != nil {
		log.Printf("Erreur d'envoie de la requete get au serveur: %v\n", err)
		return nil, err
	}
	defer response.Body.Close()
	bodyReponse, err := io.ReadAll(response.Body)
	if err != nil {
		log.Printf("Erreur de lecture du corps de la reponse du serveur: %v\n", err)
		return nil, err
	}

	var udpAdresse []string
	err = json.Unmarshal(bodyReponse, &udpAdresse)
	if err != nil {
		log.Printf("Erreur sur la deserealisation du json: %v\n", err)
		return nil, err
	}
	log.Printf("Le couple (address, port) est: %v", udpAdresse)
	return udpAdresse, nil
} */

func recupAdrresServeur() ([]string, error) {
	log.Printf("Utilisation des adresses statiques pour le test local.")
	return []string{
		"127.0.0.1:8081",
		"127.0.0.1:8082", // Simule une autre adresse (peut être non joignable)
		"127.0.0.1:8083", // Simule une autre adresse (peut être non joignable)
	}, nil
}
func buildGetQuotationPacket() (uint32, []byte) {
	requestId := generateRandomId()
	requestType := uint8(0)                   //pour le type get-quotation
	requestBody := []byte{}                   //Un corps vide
	requestLength := uint16(len(requestBody)) //normalement egal a 0

	//serialiser
	datagramme := serialisationStructUDP(
		requestId,
		requestType,
		requestLength,
		requestBody,
	)

	return requestId, datagramme
}

func deserialisationUDP(packet []byte, responseIdServer uint32) (string, error) {
	if len(packet) < 7 {
		return "", fmt.Errorf("Packet trop court (taille %d)", len(packet))
	}
	responseId := binary.BigEndian.Uint32(packet[0:4])
	responseType := packet[4]
	responseLength := binary.BigEndian.Uint16(packet[5:7])
	responseBody := packet[7:]

	if responseId != responseIdServer {
		return "", fmt.Errorf("Erreur reponse non destiné a la requete #%d", responseId)
	}
	if responseType == 128 { //Type 128 quotation
		bodyStr := string(responseBody)
		if len(bodyStr) < int(responseLength) {
			return "", fmt.Errorf("Paquet incomplet pour la citation")
		}
		return bodyStr, nil
	} else if responseType == 129 { //Erreur du serveur
		return "", fmt.Errorf("Erreur du serveur: %s\n", string(responseBody))
	} else {
		return "", fmt.Errorf("type de réponse inconnu: %d", responseType)
	}
}
func adaptative_timeout(tau float64, RTT_init float64, RTT_var_init float64) (float64, float64, float64) {
	const alpha = 7.0 / 8.0
	const beta = 3.0 / 4.0
	delta := math.Abs(tau - RTT_init)
	RTT := alpha*RTT_init + (1-alpha)*tau
	RTT_var := beta*RTT_var_init + (1-beta)*delta
	return delta, RTT, RTT_var
}

func requestEachAddr(targetAddr string, requestPacket []byte, requestID uint32, resultChan chan ProbeResult) {
	var finalResult ProbeResult
	defer func() {
		resultChan <- finalResult
	}()
	remoteAddr, err := net.ResolveUDPAddr("udp", targetAddr)
	if err != nil {
		log.Printf("Erreur de resolution d'addresse UDP: %v\n", err)
		finalResult.Error = err
		return
	}
	conn, err := net.DialUDP("udp", nil, remoteAddr) //lier socket du client avec l'adresse du severeur
	if err != nil {
		log.Printf("Erreur de connexion (DialUDP): %v\n", err)
		finalResult.Error = err
		return
	}
	defer conn.Close()
	RTO_max := time.Duration(2) * time.Second
	startTime := time.Now()
	nbOctet, err := conn.Write(requestPacket)
	if err != nil {
		log.Printf("Erreur sur l'envoie (conn.Write)")
		finalResult.Error = err
		return
	}
	log.Printf("Requête #%d envoyée (%d octet)\n", requestID, nbOctet)

	conn.SetReadDeadline(time.Now().Add(RTO_max))
	bufferRecv := make([]byte, sizeMaxDatagram)
	nbOctet, _, err = conn.ReadFromUDP(bufferRecv)
	if err != nil {
		log.Printf("Erreur de lecture de la reponse (conn.ReadFromUDP)")
		finalResult.Error = err
		return
	}
	/* if err != nil {
		netErr, ok := err.(net.Error)
		if ok && netErr.Timeout() {
			log.Fatalf("Timeout pour l'utilisation de cette adresse car le (RTO Max =%v)\n", RTO_max)
		}
		log.Fatalf("Erreur de lecture (ReadFromUDP): %v", err)
	} */

	endTime := time.Now()
	tempsEcoule := endTime.Sub(startTime)
	tau := tempsEcoule.Seconds()

	finalResult.Addr = targetAddr
	finalResult.RTT = tau
	finalResult.Response = bufferRecv[:nbOctet]
	finalResult.Error = nil
	return
}

func getBestQuotation(addressSrvs []string) (string, []byte, float64, error) {

	requestIDInit, datagrammeInit := buildGetQuotationPacket()
	resultChan := make(chan ProbeResult)

	for _, addr := range addressSrvs {
		// La goroutine utilise la taille max du datagramme (constante globale)
		go requestEachAddr(addr, datagrammeInit, requestIDInit, resultChan)
	}

	var bestAddr string
	var bestResponse []byte
	var initialRTT float64

	// Boucle d'attente
	for i := 0; i < len(addressSrvs); i++ {
		select {
		case result := <-resultChan:
			if result.Error != nil {
				log.Printf("Erreur: Addresse %s a échoué: %v\n", result.Addr, result.Error)
				continue
			}

			// Désérialisation et vérification pour s'assurer que c'est une citation valide
			_, err := deserialisationUDP(result.Response, requestIDInit)

			if err == nil {
				// Succès : Première réponse valide
				bestAddr = result.Addr
				bestResponse = result.Response
				initialRTT = result.RTT
				log.Printf("Adresse choisie (RTT le plus rapide): %s (%.3f s)", bestAddr, initialRTT)
				return bestAddr, bestResponse, initialRTT, nil // Sortie réussie
			}
			log.Printf("Réponse reçue de %s, mais invalide/corrompue: %v", result.Addr, err)

		case <-time.After(5 * time.Second):
			// Timeout global si aucune adresse ne répond dans 5s
			return "", nil, 0, fmt.Errorf("aucune adresse n'a répondu dans le délai imparti")
		}
	}

	// Si la boucle se termine sans succès (toutes les réponses ont échoué/étaient invalides)
	return "", nil, 0, fmt.Errorf("toutes les adresses ont échoué")
}
func main() {
	// Initialisation des outils aléatoires
	rand.Seed(time.Now().UnixNano())

	// 1. Récupération des adresses
	addressSrvs, err := recupAdrresServeur()
	if err != nil {
		// En cas d'échec de la récupération HTTP, on utilise l'adresse locale simulée (si besoin)
		// Dans un vrai TP, nous devrions quitter ici.
		log.Fatalf("Erreur critique: Échec de la récupération des adresses du serveur: %v\n", err)
	}

	// 2. Sélection de la meilleure adresse (Exercice 3)
	bestAddr, bestResponse, initialRTT, err := getBestQuotation(addressSrvs)
	if err != nil {
		log.Fatalf("Échec de la connexion initiale: %v", err)
	}

	// 3. Initialisation de l'état RTT/RTO (Exercice 2.2)
	RTT := initialRTT
	RTT_var := float64(0)
	RTO := RTT + 4*RTT_var // RTO initial = RTT mesuré + 0
	timeRTO := time.Duration(RTO * float64(time.Second))

	// Décoder la première citation (succès de la sonde)
	// Note: L'ID pour la désérialisation de bestResponse est implicite via getBestQuotation
	firstQuotation, err := deserialisationUDP(bestResponse, binary.BigEndian.Uint32(bestResponse[0:4]))
	if err != nil {
		log.Fatalf("Erreur de décodage de la première citation: %v", err)
	}
	fmt.Printf("\n--- Première Citation Reçue (via %s) ---\n%s\n----------------------\n", bestAddr, firstQuotation)

	// 4. Connexion permanente à la meilleure adresse
	remoteAddr, err := net.ResolveUDPAddr("udp", bestAddr) // Utilisation de bestAddr
	if err != nil {
		log.Fatalf("Erreur de resolution d'addresse UDP pour %s: %v\n", bestAddr, err)
	}
	conn, err := net.DialUDP("udp", nil, remoteAddr)
	if err != nil {
		log.Fatalf("Erreur de connexion (DialUDP) sur %s: %v\n", bestAddr, err)
	}
	defer conn.Close()

	// 5. Boucle de travail permanente (Exercices 2.1 - 2.3)
	for {
		// Logique d'envoi et RTO/Backoff

		// Envoi de la requête
		requestID, datagramme := buildGetQuotationPacket()
		startTime := time.Now()
		_, err = conn.Write(datagramme)
		if err != nil {
			log.Fatalf("Erreur d'envoie (Write): %v\n", err)
		}

		// Attente de la réponse avec RTO dynamique/doublé
		conn.SetReadDeadline(time.Now().Add(timeRTO))
		bufferRecv := make([]byte, sizeMaxDatagram)
		nbOctet, _, err := conn.ReadFromUDP(bufferRecv)

		if err != nil {
			netErr, ok := err.(net.Error)
			if ok && netErr.Timeout() {
				// Exponential Backoff (Perte détectée)
				log.Printf("Timeout RTO dépassé (RTO=%v). Tentative de réémission.\n", timeRTO)
				if timeRTO > 64*time.Second {
					log.Fatalf("Abandon car le RTO dépasse 64s.")
				}
				timeRTO *= 2 // Doubler le RTO
				continue
			}
			log.Fatalf("Erreur de lecture non-Timeout: %v", err)
		}

		// Traitement de la réponse réussie
		endTime := time.Now()
		tempsEcoule := endTime.Sub(startTime)
		tau := tempsEcoule.Seconds()

		// Mise à jour RTT et RTO (Exercice 2.2)
		_, RTT, RTT_var = adaptative_timeout(tau, RTT, RTT_var)
		RTO = RTT + 4*RTT_var
		timeRTO = time.Duration(RTO * float64(time.Second)) // Réinitialisation au RTO dynamique
		log.Printf("RTT=%.3f s, RTT_var=%.3f s. Prochain RTO=%.3f s.", RTT, RTT_var, RTO)

		// Désérialisation et affichage
		responsePacket := bufferRecv[:nbOctet]
		quotation, err := deserialisationUDP(responsePacket, requestID)
		if err != nil {
			log.Fatalf("Erreur de décodage ou de vérification: %v", err)
		}

		fmt.Printf("\n--- Citation Reçue ---\n%s\n----------------------\n", quotation)

		// Délai de périodicité (Exercice 2.1)
		time.Sleep(defaultPeriod)
	}
}
