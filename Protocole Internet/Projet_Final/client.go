package main

import (
	"bytes"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/sha256"
	"crypto/tls"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"log"
	"math/big"
	mrand "math/rand"
	"net"
	"net/http"
	"strings"
	"time"
)

// =========================================================================================================================
//
//	Definition des variables globales
//
// =========================================================================================================================
const urlSrv = "https://jch.irif.fr:8443/peers/"
const nickName = "A.S fait un test"

var ErrIgnorePack = errors.New("Packet Ignore: la clé publique est manquante")

//=========================================================================================================================
//											Fin de la definition des variables globales
//=========================================================================================================================

//-------------------------------------------------------------------------------------------------------------------------

// =========================================================================================================================
//
//	Definition des Structures de Données
//
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

//=========================================================================================================================
//											Fin de la Structures de Données
//=========================================================================================================================

//-------------------------------------------------------------------------------------------------------------------------

//=========================================================================================================================
//											Code de la partie Cryptographie du Projet
//=========================================================================================================================

func generateKey() (*ecdsa.PrivateKey, error) {
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return nil, fmt.Errorf("Erreur de Generation de clés(ecdsa.GenerateKey):%v", err)
	}
	return privateKey, nil
}
func formatPubKey(publicKey *ecdsa.PublicKey) []byte {
	formatted := make([]byte, 64)
	publicKey.X.FillBytes(formatted[:32])
	publicKey.Y.FillBytes(formatted[32:])
	return formatted
}

func parsePubKey(data []byte) *ecdsa.PublicKey {
	var x, y big.Int
	x.SetBytes(data[:32])
	y.SetBytes(data[32:])
	publicKey := ecdsa.PublicKey{
		Curve: elliptic.P256(),
		X:     &x,
		Y:     &y,
	}
	return &publicKey
}

func computeSignature(privKey *ecdsa.PrivateKey, data []byte) ([]byte, error) {
	hashed := sha256.Sum256(data)
	r, s, err := ecdsa.Sign(rand.Reader, privKey, hashed[:])
	if err != nil {
		return nil, fmt.Errorf("Erreur de signature(computeSignature): %v", err)
	}
	signature := make([]byte, 64)
	r.FillBytes(signature[:32])
	s.FillBytes(signature[32:])
	return signature, nil
}

func verifySignature(publicKey *ecdsa.PublicKey, signature []byte, data []byte) bool {
	var r, s big.Int
	r.SetBytes(signature[:32])
	s.SetBytes(signature[32:])
	hashed := sha256.Sum256(data)
	ok := ecdsa.Verify(publicKey, hashed[:], &r, &s)
	return ok
}

//=========================================================================================================================
//											Fin: Code de la partie Cryptographie du Projet
//=========================================================================================================================

//-------------------------------------------------------------------------------------------------------------------------

//=========================================================================================================================
//										Code pour la communication avec le serveur REST-Like/HTTP
//=========================================================================================================================

/**
* RegisterKey permet d'enregistrer un pair au-pres du sevrveu via une requete PUT dont le corps est
*la clé publique du pair:
*	-peerName: le nom du pair
*	-pubKey: sa clé publique
 */
func RegisterKey(client *http.Client, peerName string, pubKey []byte) error {
	url := fmt.Sprintf(urlSrv+"%s/key", peerName)
	body := bytes.NewReader(pubKey)
	prepareReq, err := http.NewRequest("PUT", url, body)
	if err != nil {
		return fmt.Errorf("Erreur creation requete: %v", err)
	}
	reponseSrv, err := client.Do(prepareReq)
	if err != nil {
		return fmt.Errorf("Erreur envoie PUT: %v", err)
	}
	defer reponseSrv.Body.Close()

	if reponseSrv.StatusCode != http.StatusOK && reponseSrv.StatusCode != http.StatusCreated && reponseSrv.StatusCode != http.StatusNoContent {
		return fmt.Errorf("Le serveur a repondu avec le status: %d", reponseSrv.StatusCode)
	}
	return nil
}

func GetPeerList(client *http.Client) ([]string, error) {
	response, err := client.Get(urlSrv)
	if err != nil {
		return nil, fmt.Errorf("Erreur reponse serveur: %v", err)
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("Le serveur a repondu avec le status: %d", response.StatusCode)
	}
	bodyResp, err := io.ReadAll(response.Body)
	if err != nil {
		return nil, fmt.Errorf("Erreur lecture corps reponse: %v", err)
	}
	body := strings.TrimSpace(string(bodyResp))
	peers := strings.Split(body, "\n")

	return peers, nil
}

func GetPeerAddresses(client *http.Client, peerName string) ([]net.UDPAddr, error) {
	url := fmt.Sprintf(urlSrv+"%s/addresses", peerName)
	response, err := client.Get(url)
	if err != nil {
		return nil, fmt.Errorf("Erreur reponse serveur: %v", err)
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("Le serveur a repondu avec le status: %d", response.StatusCode)
	}
	bodyResp, err := io.ReadAll(response.Body)
	if err != nil {
		return nil, fmt.Errorf("Erreur lecture corps reponse: %v", err)
	}
	body := strings.TrimSpace(string(bodyResp))
	if body == "" {
		return nil, fmt.Errorf("Corps auncun pair trouvé")
	}
	addrSocsStr := strings.Split(body, "\n")
	var addrSoc []net.UDPAddr
	for i := 0; i < len(addrSocsStr); i++ {
		if addrSocsStr[i] == "" {
			continue
		}
		addr, err := net.ResolveUDPAddr("udp", addrSocsStr[i])
		if err != nil {
			log.Printf("Erreur sur la resolution de l'adresse %s", addrSocsStr[i])
			continue
		}
		addrSoc = append(addrSoc, *addr)
	}
	if len(addrSoc) <= 0 {
		return nil, fmt.Errorf("Erreur la conversion des adresse ne s'ait pas bien deroulé")
	}
	return addrSoc, nil
}

func GetPublicKey(client *http.Client, peerName string) ([]byte, error) {
	url := fmt.Sprintf(urlSrv+"%s/key", peerName)
	response, err := client.Get(url)
	if err != nil {
		return nil, fmt.Errorf("Erreur envoie GET(GetPublicKey): %v", err)
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("Le serveur a repondu avec le status(GetPublicKey): %v", response.StatusCode)
	}
	bodyResponse, err := io.ReadAll(response.Body)
	if err != nil {
		return nil, fmt.Errorf("Erreur lecture de la reponse(GetPublicKey): %v", err)
	}
	if len(bodyResponse) != 64 {
		return nil, fmt.Errorf("Erreur sur la taille de la reponse(%d) car elle devrait etre 64", len(bodyResponse))
	}
	return bodyResponse, nil
}

//=========================================================================================================================
//										Fin de la partie communication avec le REST-Like/HTTP
//=========================================================================================================================

//-------------------------------------------------------------------------------------------------------------------------

//=========================================================================================================================
//											Code du protocole Pair-a-Pair: UDP
//=========================================================================================================================

//-------------------------------------------------------------------------------------------------------------------------
//											1. Representation des données sur le reseau
//-------------------------------------------------------------------------------------------------------------------------

func SerialisationDatagram(Id uint32, Type uint8, body []byte, privKey *ecdsa.PrivateKey) ([]byte, error) {
	Length := uint16(len(body))
	sizeDatagram := 4 + 1 + 2 + int(Length)
	if privKey != nil {
		sizeDatagram += 64
	}
	datagram := make([]byte, sizeDatagram)

	binary.BigEndian.PutUint32(datagram[0:4], Id)
	datagram[4] = Type
	binary.BigEndian.PutUint16(datagram[5:7], Length)
	copy(datagram[7:7+int(Length)], body)
	data := datagram[0 : 7+int(Length)]
	if privKey != nil {
		signature, err := computeSignature(privKey, data)
		if err != nil {
			return nil, fmt.Errorf("Erreur signature(SerialisationDatagram): %v", err)
		}
		copy(datagram[7+int(Length):], signature)
	}
	return datagram, nil
}

func DeserialisationUDP(packet []byte, publicKey *ecdsa.PublicKey) (*Datagram, error) {
	if len(packet) < 7 {
		return nil, fmt.Errorf("Erreur packet trop court(Taille%d)(deserialisationUDP)", len(packet))
	}
	var responseDatagram Datagram
	responseDatagram.Id = binary.BigEndian.Uint32(packet[0:4])
	responseDatagram.Type = uint8(packet[4])
	responseDatagram.Length = binary.BigEndian.Uint16(packet[5:7])

	if len(packet) < (7 + int(responseDatagram.Length)) {
		return nil, fmt.Errorf("Packet mal formé taille recu %d correspond pas a la taille declarer %d + %d", len(packet), 7, responseDatagram.Length)
	}
	responseDatagram.Body = packet[7 : 7+int(responseDatagram.Length)]
	if len(packet[7+int(responseDatagram.Length):]) == 0 {
		return &responseDatagram, nil
	}
	signature := packet[7+int(responseDatagram.Length):]
	if len(signature) != 64 {
		return nil, fmt.Errorf("La taille de la signature du packet(%d) est incorrecte", len(signature))
	}
	data := packet[0 : 7+int(responseDatagram.Length)]
	if publicKey == nil {
		return nil, fmt.Errorf("Erreur sur l'argument(publicKey) de la fonction deserialisationUDP qui est nul")
	}
	ok := verifySignature(publicKey, signature, data)
	if !ok {
		return nil, ErrIgnorePack
	}
	return &responseDatagram, nil
}

//-------------------------------------------------------------------------------------------------------------------------
//										Fin: 1. Representation des données sur le reseau
//-------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
//  										2. Ouverture d'un socket d'ecoute et envoie de requete
// -------------------------------------------------------------------------------------------------------------------------

func StartUDPListener(addrSoc *net.UDPAddr) (*net.UDPConn, error) {
	conn, err := net.ListenUDP("udp", addrSoc)
	if err != nil {
		return nil, fmt.Errorf("Erreur d'ecoute du client sur le port %v: %v", addrSoc, err)
	}
	return conn, nil
}

func SendRequestToThePeer(conn *net.UDPConn, addrSoc *net.UDPAddr, datagramUDP []byte) (int, error) {
	nbOctet, err := conn.WriteToUDP(datagramUDP, addrSoc)
	if err != nil {
		return 0, fmt.Errorf("Erreur d'envoie du message(UDP:conn.Writ): %v", err)
	}
	return nbOctet, nil
}

//-------------------------------------------------------------------------------------------------------------------------
//										Fin: 2. Ouverture d'un socket d'ecoute et envoie de requete
//-------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
//  										3. Les differents messages du protocole
// -------------------------------------------------------------------------------------------------------------------------

func generateRandomId() uint32 {
	return mrand.Uint32()
}

func BuildHelloPacket(peerName string, extensions uint32, privKey *ecdsa.PrivateKey) ([]byte, error) {
	requestId := generateRandomId()
	requestType := uint8(1)
	sizeBody := 4 + len(peerName)
	requestBody := make([]byte, sizeBody)
	binary.BigEndian.PutUint32(requestBody[0:4], extensions)
	copy(requestBody[4:], []byte(peerName))
	datagram, err := SerialisationDatagram(requestId, requestType, requestBody, privKey)
	if err != nil {
		return nil, fmt.Errorf("Erreur sur la construction du message Hello(BuildMsgBody): %v", err)
	}
	return datagram, nil
}

func BuildPingPacket() ([]byte, error) {
	requestId := generateRandomId()
	resquestType := uint8(0)
	requestBody := []byte{}
	datagram, err := SerialisationDatagram(requestId, resquestType, requestBody, nil)
	if err != nil {
		return nil, fmt.Errorf("Erreur sur la construction du message Ping(BuildPingPacket): %v", err)
	}
	return datagram, nil
}

func BuildRootRequestPacket(privKey *ecdsa.PrivateKey) ([]byte, error) {
	requestId := generateRandomId()
	requestType := uint8(2)
	requestBody := []byte{}
	datagram, err := SerialisationDatagram(requestId, requestType, requestBody, privKey)
	if err != nil {
		return nil, fmt.Errorf("Erreur construction message RootRequest(BuildRootRequestPacket): %v", err)
	}
	return datagram, nil
}

// -------------------------------------------------------------------------------------------------------------------------
//  										Fin: 3. Les differents messages du protocole
// -------------------------------------------------------------------------------------------------------------------------

//=========================================================================================================================
//											Fin: Code du protocole Pair-a-Pair: UDP
//=========================================================================================================================

// =========================================================================================================================
//
//	La fonction main
//
// =========================================================================================================================
// Configuration du client pour ignorer la verification TLS
func createClient() *http.Client {
	// Crée une *copie* du transport par défaut pour la modification
	tr := http.DefaultTransport.(*http.Transport).Clone()

	// Modifie la configuration TLS de la copie
	tr.TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	return &http.Client{
		Transport: tr,
		Timeout:   50 * time.Second,
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
	}
}
func main() {
	privateKey, err := generateKey()
	if err != nil {
		log.Fatalf("Erreur de la generation de la clé privée: %v", err)
	}
	publicKey, ok := privateKey.Public().(*ecdsa.PublicKey)
	if !ok {
		log.Fatalf("L'exctration de la clé publique a essoué")
	}
	pubKey := formatPubKey(publicKey)
	client := createClient()
	err = RegisterKey(client, nickName, pubKey)
	if err != nil {
		log.Fatalf("Erreur enregistrement du peer(%s): %v", nickName, err)
	}
	log.Printf("Enregistrer avec succes")

	peers, err := GetPeerList(client)
	if err != nil {
		log.Fatalf("Erreur le serveur n'a pas transmis la liste des pairs: %v", err)
	}
	log.Printf("Les peers sont:\n%v\n", peers)
	addrSoc, err := GetPeerAddresses(client, nickName)
	if err != nil {
		log.Fatalf("Erreur sur la recuperation des adresse de socket des pairs: %v", err)
	}
	log.Printf("Les adresses des pairs sont: \n%v\n", addrSoc)
	pubk, err := GetPublicKey(client, nickName)
	if err != nil {
		log.Fatalf("Erreur sur la recuperation de la cle publique")
	}
	log.Printf("La cle publique est: %v", pubk)
}
