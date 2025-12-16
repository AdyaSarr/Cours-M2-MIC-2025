package main

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/binary"
	"encoding/json"
	"encoding/pem" // Ajout de l'import pour le format PEM
	"fmt"
	"io"
	"log"
	"math/big"
	"net"
	"net/http"
	"os" // Ajout de l'import pour la gestion des fichiers
	"strconv"
	"sync"
	"time"
)

// --- Constantes et Structures du Protocole ---

const httpPort = 8080
const udpPort = 8081
const maxPacketSize = 1024
const certFile = "cert.pem" // Nom du fichier de certificat
const keyFile = "key.pem"   // Nom du fichier de clé privée

const (
	// Types de messages P2P
	pingType    uint8 = 0
	messageType uint8 = 1
	okReply     uint8 = 128
	errorReply  uint8 = 129
)

// Définitions du client (doivent correspondre au client)
type AddrSocket struct {
	IP   string `json:"IP"`
	Port uint16 `json:"Port"`
}

type InfoPerrs struct {
	Nickname  string     `json:"Nickname"`
	Token     string     `json:"Token,omitempty"`
	AddrSoc   AddrSocket `json:"AddrSoc"`
	PublicKey string     `json:"PublicKey,omitempty"`
	Status    uint8      `json:"Status"`
}

// État des pairs (simulé pour le contrôle HTTP)
var peersState = map[string]InfoPerrs{
	"jch": {
		Nickname: "jch",
		AddrSoc: AddrSocket{
			IP:   "127.0.0.1",
			Port: udpPort, // L'adresse de ce serveur lui-même
		},
		Status: 0, // ONLINE
	},
	"alice": {
		Nickname: "alice",
		AddrSoc: AddrSocket{
			IP:   "192.168.1.10", // Adresse d'un autre pair (simulée)
			Port: 8081,
		},
		Status: 0, // ONLINE
	},
	"adya": {
		Nickname: "adya",
		AddrSoc: AddrSocket{
			IP:   "192.173.1.1", // Adresse d'un autre pair (simulée)
			Port: 443,
		},
		Status: 0, // ONLINE
	},
}

// --- Fonctions de Contrôle HTTP (Partie Client-Serveur) ---

// Gère GET /peers/?count=n
func handleGetPeers(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Méthode non supportée", http.StatusMethodNotAllowed)
		return
	}

	countStr := r.URL.Query().Get("count")
	count, err := strconv.Atoi(countStr)
	if err != nil || count <= 0 {
		count = len(peersState)
	}

	var peerList []InfoPerrs
	i := 0
	for _, peer := range peersState {
		if i >= count {
			break
		}
		peerList = append(peerList, peer)
		i++
	}

	w.Header().Set("Content-Type", "application/json")
	if err := json.NewEncoder(w).Encode(peerList); err != nil {
		log.Printf("Erreur d'encodage JSON: %v", err)
		http.Error(w, "Erreur interne", http.StatusInternalServerError)
	}
}

// Implémente le handler pour PUT /peers/nickname?token=token
func handlePutPeer(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPut {
		http.Error(w, "Méthode non supportée", http.StatusMethodNotAllowed)
		return
	}

	nickname := r.URL.Path[len("/peers/"):]

	if nickname == "" {
		http.Error(w, "Nickname manquant", http.StatusBadRequest)
		return
	}

	token := r.URL.Query().Get("token")
	if existingPeer, ok := peersState[nickname]; ok && token != "" && existingPeer.Token != "" && existingPeer.Token != token {
		http.Error(w, "Jeton invalide pour ce nickname", http.StatusForbidden)
		return
	}

	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Erreur de lecture du corps", http.StatusBadRequest)
		return
	}
	var registerData map[string]uint16
	if err := json.Unmarshal(body, &registerData); err != nil {
		http.Error(w, "Corps JSON invalide", http.StatusBadRequest)
		return
	}

	port, ok := registerData["port"]
	if !ok {
		http.Error(w, "Le champ 'port' est manquant dans le corps JSON", http.StatusBadRequest)
		return
	}

	// L'IP réelle est dans r.RemoteAddr, mais on utilise 127.0.0.1 pour la simu locale
	ip := "127.0.0.1"

	newPeerInfo := InfoPerrs{
		Nickname: nickname,
		Token:    token,
		AddrSoc: AddrSocket{
			IP:   ip,
			Port: port,
		},
		Status: 0,
	}
	peersState[nickname] = newPeerInfo
	log.Printf("PUT OK: Nickname %s enregistré à %s:%d", nickname, ip, port)

	w.WriteHeader(http.StatusOK)
}

// --- Fonctions de Génération de Certificats TLS ---

// Helper pour encoder en PEM
func pemEncode(data []byte, typ string) []byte {
	return pem.EncodeToMemory(&pem.Block{Type: typ, Bytes: data})
}

// Fonction combinée pour vérifier, générer et écrire les certificats sur le disque
func ensureCertificatesExist(certPath, keyPath string) error {
	// 1. Vérifier si les fichiers existent
	if _, err := os.Stat(certPath); err == nil {
		if _, err := os.Stat(keyPath); err == nil {
			log.Println("Clés TLS existantes trouvées. Utilisation des clés existantes.")
			return nil
		}
	}

	log.Println("Génération de nouvelles clés TLS auto-signées...")

	// 2. Générer la clé privée RSA
	privateKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return fmt.Errorf("échec de la génération de la clé privée: %w", err)
	}

	// 3. Créer le modèle de certificat
	template := x509.Certificate{
		SerialNumber: big.NewInt(1),
		Subject: pkix.Name{
			Organization: []string{"Advanced Networks TP"},
			CommonName:   "localhost",
		},
		NotBefore:   time.Now(),
		NotAfter:    time.Now().Add(time.Hour * 24 * 365),
		KeyUsage:    x509.KeyUsageKeyEncipherment | x509.KeyUsageDigitalSignature,
		ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
	}

	// 4. Créer le certificat DER (encodage binaire)
	certDER, err := x509.CreateCertificate(rand.Reader, &template, &template, &privateKey.PublicKey, privateKey)
	if err != nil {
		return fmt.Errorf("échec de la création du certificat: %w", err)
	}

	// 5. Encodage PEM
	certPEM := pemEncode(certDER, "CERTIFICATE")
	keyPEM := pemEncode(x509.MarshalPKCS1PrivateKey(privateKey), "RSA PRIVATE KEY")

	// 6. Écriture des fichiers
	if err := os.WriteFile(certPath, certPEM, 0600); err != nil {
		return fmt.Errorf("échec de l'écriture du certificat: %w", err)
	}
	if err := os.WriteFile(keyPath, keyPEM, 0600); err != nil {
		return fmt.Errorf("échec de l'écriture de la clé privée: %w", err)
	}

	log.Println("Génération réussie.")
	return nil
}

func startHTTPServer() {
	// 0. Assurer la présence des certificats
	if err := ensureCertificatesExist(certFile, keyFile); err != nil {
		log.Fatalf("Erreur critique lors de la préparation des certificats: %v", err)
	}

	mux := http.NewServeMux()
	mux.HandleFunc("/peers/", func(w http.ResponseWriter, r *http.Request) {
		if r.Method == http.MethodGet {
			handleGetPeers(w, r)
		} else if r.Method == http.MethodPut {
			handlePutPeer(w, r)
		} else {
			http.Error(w, "Méthode non gérée", http.StatusMethodNotAllowed)
		}
	})

	log.Printf("Serveur de contrôle HTTPS démarré sur :%d (GET/PUT/DELETE /peers/)", httpPort)

	// Démarrage du serveur HTTPS
	if err := http.ListenAndServeTLS(fmt.Sprintf(":%d", httpPort), certFile, keyFile, mux); err != nil {
		// En cas d'erreur de l'écoute (ex: port déjà utilisé), on stoppe
		log.Fatalf("Échec du démarrage du serveur HTTPS: %v", err)
	}
}

// --- Serveur P2P UDP (Répond aux Messages Type 0 et 1) ---

// Fonction utilitaire pour envoyer une réponse UDP formatée
func sendUDPReply(conn *net.UDPConn, addr *net.UDPAddr, id uint32, msgType byte, body []byte) {
	responseLength := len(body)
	responsePacket := make([]byte, 7+responseLength)

	binary.BigEndian.PutUint32(responsePacket[0:4], id)
	responsePacket[4] = msgType
	binary.BigEndian.PutUint16(responsePacket[5:7], uint16(responseLength))
	copy(responsePacket[7:], body)

	conn.WriteToUDP(responsePacket, addr)
	log.Printf("[UDP] Réponse #%d envoyée à %s (Type: %d)", id, addr.String(), msgType)
}

func handleUDPRequest(conn *net.UDPConn, addr *net.UDPAddr, data []byte) {
	if len(data) < 7 {
		log.Printf("[UDP] Paquet trop court reçu de %s", addr.String())
		return
	}

	requestID := binary.BigEndian.Uint32(data[0:4])
	requestType := data[4]
	requestLength := binary.BigEndian.Uint16(data[5:7])
	requestBody := data[7:]

	if len(data) != 7+int(requestLength) {
		log.Printf("[UDP] Paquet #%d corrompu (longueur %d != %d)", requestID, len(data), 7+int(requestLength))
		return
	}

	if requestType == pingType { // Type 0: Ping
		sendUDPReply(conn, addr, requestID, okReply, nil)
	} else if requestType == messageType { // Type 1: Message

		if requestLength < 4 {
			sendUDPReply(conn, addr, requestID, errorReply, []byte("Message body trop court pour contenir Message-Id"))
			return
		}

		messageID := binary.BigEndian.Uint32(requestBody[0:4])
		messageContent := string(requestBody[4:])

		log.Printf("[UDP] MESSAGE #%d reçu de %s (MsgID: %d): %s", requestID, addr.String(), messageID, messageContent)

		sendUDPReply(conn, addr, requestID, okReply, nil)
	} else {
		log.Printf("[UDP] Requête #%d ignorée (Type: %d)", requestID, requestType)
	}
}

func startUDPServer() {
	udpAddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf(":%d", udpPort))
	if err != nil {
		log.Fatalf("Erreur de résolution d'adresse UDP: %v", err)
	}

	conn, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		log.Fatalf("Erreur de l'écoute UDP: %v", err)
	}
	defer conn.Close()

	log.Printf("Serveur P2P UDP démarré sur :%d", udpPort)

	buffer := make([]byte, maxPacketSize)
	for {
		n, addr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("[UDP] Erreur de lecture: %v", err)
			continue
		}

		dataCopy := make([]byte, n)
		copy(dataCopy, buffer[:n])

		go handleUDPRequest(conn, addr, dataCopy)
	}
}

// --- Fonction Main (Lancement des deux serveurs) ---

func main() {
	log.SetFlags(log.Lshortfile | log.Ltime)

	var wg sync.WaitGroup
	wg.Add(2)

	// 1. Lancer le serveur de contrôle HTTP (HTTPS maintenant)
	go func() {
		defer wg.Done()
		startHTTPServer()
	}()

	// 2. Lancer le serveur P2P UDP
	go func() {
		defer wg.Done()
		startUDPServer()
	}()

	wg.Wait()
}
