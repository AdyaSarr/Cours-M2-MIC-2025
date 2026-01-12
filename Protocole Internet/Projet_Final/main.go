package main

import (
	"crypto/ecdsa"
	"crypto/tls"
	"errors"
	"log"
	"net"
	"net/http"
	"os"
	"sync"
	"time"
)

// =========================================================================================================================
//											Definition des variables globales
// =========================================================================================================================

const urlSrv = "https://jch.irif.fr:8443/peers/"
const nickName = "et"
const sizeMaxDatagram = 2048

var ErrIgnorePack = errors.New("Packet Ignore: la clé publique est manquante")

var peerMap = make(map[string]*PeerAssociation)
var mapLock sync.RWMutex

var resultReqOrRep Dispatcher

var contentStorage = ContentBD{
	storage: make(map[string][]byte),
}

var myRootHash []byte

var downloadLimit = make(chan struct{}, 20)

// =========================================================================================================================
//											Definition des Structures de Données Globales
// =========================================================================================================================

type StatusPeer uint8

/**
* Definition de chaque constante:
*	-DISCOVERED: le nom du pair et sa cle ont été obtenus via le serveur REST
*	-OFFLINE: Le pair n'ets plus actif ou l'association a depassé les 5minutes
*	-ASSOCIATED: Les deux pairs ont echangé avec succés Hello et HelloReply
*	-NAT_TRAVERSAL: Le pair accept de servir d'intermediaire pour la traversee de NAT
 */
const (
	DISCOVERED StatusPeer = iota
	OFFLINE
	HANDSHAKING
	ASSOCIATED
	NAT_TRAVERSAL
)

/**
* Cette structure permet de stocker tous les parametres necessaires pour le pair comme:
*	-LastSeen: Pour gerer l'expiration apres 5 minutes d'inactivité
*	-PublicKey: Stocke la clé publique du pair
*	-Addresses: Stocke les adresses de tous les pairs que le ce pair peut contacter
*	-Status: reflete le cycle de vie du pair au sein du protocole Hybride(Client-Sreveur et Pair-a-Pair)
 */
type PeerAssociation struct {
	NickName   string
	LastSeen   time.Time
	PublicKey  *ecdsa.PublicKey
	Addresses  []net.UDPAddr
	Status     StatusPeer
	Extensions uint32
}

type Datagram struct {
	Id        uint32
	Type      uint8
	Length    uint16
	Body      []byte
	Signature []byte
}

type ResponseMessage struct {
	Type uint8
	Body []byte
}

type Dispatcher struct {
	sync.RWMutex     // Protéger la map contre les accès concurrents
	responseChannels map[uint32]chan ResponseMessage
}

type ContentBD struct {
	sync.RWMutex
	storage map[string][]byte
}

// ============================================================================
// 						Fonctions d'Initialisation et Main
// ============================================================================

func createClient() *http.Client {
	tr := http.DefaultTransport.(*http.Transport).Clone()
	tr.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	return &http.Client{
		Transport: tr,
		Timeout:   50 * time.Second,
	}
}

func main() {
	// 1. Initialisation de la cryptographie
	privateKey, _ := GenerateKey()
	publicKey := privateKey.Public().(*ecdsa.PublicKey)
	pubKeyBytes := FormatPubKey(publicKey)

	// 2. Initialisation du dispatcher
	resultReqOrRep.responseChannels = make(map[uint32]chan ResponseMessage)

	// 3. Inscription sur le serveur REST
	client := createClient()
	err := RegisterKey(client, nickName, pubKeyBytes)
	if err != nil {
		log.Fatalf("Erreur Register: %v", err)
	}

	go func() {
		for {
			time.Sleep(20 * time.Minute) // Attendre 20 minutes
			log.Println("Rafraîchissement du NickName sur le serveur REST...")
			err := RegisterKey(client, nickName, pubKeyBytes)
			if err != nil {
				log.Printf("Erreur lors du rafraîchissement : %v", err)
			} else {
				log.Println("NickName maintenu avec succès.")
			}
		}
	}()

	// 4. Préparation des données locales (Merkle Tree)
	myRootHash, _ = ExportCatsPhotos()
	log.Printf("Mon Root Hash: %x", myRootHash)

	// 5. Configuration Réseau UDP
	os.MkdirAll("downloads", 0755)
	addrLocal, _ := net.ResolveUDPAddr("udp", ":0") // Port aléatoire
	conn, err := StartUDPListener(addrLocal)
	if err != nil {
		log.Fatalf("Erreur UDP: %v", err)
	}
	defer conn.Close()

	// 6. Lancement des services en arrière-plan
	go maintainConnPairs(conn)             // Maintenance des pairs
	go StartRead(client, conn, privateKey) // Écoute réseau UDP

	// 7. Démarrage du protocole de découverte (P2P)
	serverAddr, _ := net.ResolveUDPAddr("udp", "jch.irif.fr:8443")
	log.Println("Démarrage de la Discovery Routine...")
	go DiscoveryRoutine(conn, serverAddr, privateKey)

	// Bloquer le main pour laisser les goroutines travailler
	select {}
}
