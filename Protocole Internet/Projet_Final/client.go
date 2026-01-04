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
	"sync"
	"time"
)

// =========================================================================================================================
//											Definition des variables globales
// =========================================================================================================================

const urlSrv = "https://jch.irif.fr:8443/peers/"
const nickName = "SARR"
const sizeMaxDatagram = 2048

var ErrIgnorePack = errors.New("Packet Ignore: la clé publique est manquante")

var peerMap = make(map[string]*PeerAssociation)
var mapLock sync.RWMutex

var resultReqOrRep Dispatcher

//=========================================================================================================================
//											Fin de la definition des variables globales
//=========================================================================================================================

//-------------------------------------------------------------------------------------------------------------------------

// =========================================================================================================================
//											Definition des Structures de Données
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
	if len(packet) > 7+int(responseDatagram.Length) {
		signature := packet[7+int(responseDatagram.Length):]
		if len(signature) == 64 {
			responseDatagram.Signature = signature
			if publicKey != nil {
				data := packet[0 : 7+int(responseDatagram.Length)]
				ok := verifySignature(publicKey, signature, data)
				if !ok {
					return nil, ErrIgnorePack
				}
			}
		}
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

func StartRead(client *http.Client, conn *net.UDPConn, privKey *ecdsa.PrivateKey) {
	for {
		buffer := make([]byte, sizeMaxDatagram)
		nbOctet, addr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Erreur de lecture du message du pair %v (conn.ReadFromUDP): %v", addr, err)
			continue
		}
		go ProcessPacket(client, conn, buffer[:nbOctet], addr, privKey)
	}
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
func BuildHelloReplyPacket(IdResponse uint32, peerName string, extensions uint32, privKey *ecdsa.PrivateKey) ([]byte, error) {
	responseType := uint8(129)
	sizeBody := 4 + len(peerName)
	responseBody := make([]byte, sizeBody)
	binary.BigEndian.PutUint32(responseBody[0:4], extensions)
	copy(responseBody[4:], []byte(peerName))
	datagram, err := SerialisationDatagram(IdResponse, responseType, responseBody, privKey)
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

func BuildPongReplyPacket(responseId uint32) ([]byte, error) {
	responseType := uint8(128)
	responseBody := []byte{}
	datagram, err := SerialisationDatagram(responseId, responseType, responseBody, nil)
	if err != nil {
		return nil, fmt.Errorf("Erreur consteruction message Pong:ok(BuildPongReplyPacket)")
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

func BuildDatumRequestPacket(hash []byte) ([]byte, error) {
	requestId := generateRandomId()
	requestType := uint8(3)
	requestBody := make([]byte, 32)
	copy(requestBody, hash)
	datagram, err := SerialisationDatagram(requestId, requestType, requestBody, nil)
	if err != nil {
		return nil, fmt.Errorf("Erreur construction message DatumRequest(BuildDatumRequestPacket): %v", err)
	}
	return datagram, nil
}

// -------------------------------------------------------------------------------------------------------------------------
//  										Fin: 3. Les differents messages du protocole
// -------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
//  										4. Processe de traitement des packet et getter
// -------------------------------------------------------------------------------------------------------------------------

func HandleRequest(client *http.Client, conn *net.UDPConn, addr *net.UDPAddr, datagram Datagram, privKey *ecdsa.PrivateKey) error {
	var pubKey *ecdsa.PublicKey
	if datagram.Type == 1 {
		mapLock.RLock()
		name := string(datagram.Body[4:])
		assos, exist := peerMap[name]
		mapLock.RUnlock()
		if exist && assos.PublicKey != nil {
			pubKey = assos.PublicKey
		} else {
			pubKeyWithoutParse, err := GetPublicKey(client, name)
			if err != nil {
				return err
			}
			pubKey = parsePubKey(pubKeyWithoutParse)
		}
		dataToVerify := make([]byte, 7+len(datagram.Body))
		binary.BigEndian.PutUint32(dataToVerify[0:4], datagram.Id)
		dataToVerify[4] = datagram.Type
		binary.BigEndian.PutUint16(dataToVerify[5:7], datagram.Length)
		copy(dataToVerify[7:], datagram.Body)
		ok := verifySignature(pubKey, datagram.Signature, dataToVerify)
		if !ok {
			return fmt.Errorf("Erreur verfication signature(HandleRequest)")
		}
		mapLock.Lock()
		newAssos := &PeerAssociation{
			NickName:  name,
			PublicKey: pubKey,
			Status:    ASSOCIATED,
			LastSeen:  time.Now(),
		}
		peerMap[name] = newAssos
		mapLock.Unlock()
		reply, err := BuildHelloReplyPacket(datagram.Id, nickName, 0, privKey)
		if err != nil {
			return fmt.Errorf("Erreur constrcution HelloReply(HandleRequest): %v", err)
		}
		_, err = SendRequestToThePeer(conn, addr, reply)
		if err != nil {
			return fmt.Errorf("Erreur envoie HelloReply(HandleRequest): %v", err)
		}
		return nil
	} else if datagram.Type == 0 {
		reply, err := BuildPongReplyPacket(datagram.Id)
		if err != nil {
			return fmt.Errorf("Erreur construction PongReply(HandleRequest): %v", err)
		}
		_, err = SendRequestToThePeer(conn, addr, reply)
		if err != nil {
			return fmt.Errorf("Erreur envoie PongReply(HandleRequest): %v", err)
		}
		return nil
	} else {
		return nil
	}
}

func HandleResponse(datagram Datagram) error {

	resultReqOrRep.RLock()
	chanel, exist := resultReqOrRep.responseChannels[datagram.Id]
	resultReqOrRep.RUnlock()
	if !exist {
		return fmt.Errorf("Erreur il y'a aucun canal qui attende cette reponse")
	}
	var reponse ResponseMessage
	reponse.Type = datagram.Type
	reponse.Body = datagram.Body
	select {
	case chanel <- reponse:
	default:
		log.Println("Personne pour recevoir la réponse")
	}
	return nil
}

func ProcessPacket(client *http.Client, conn *net.UDPConn, ReqOrRep []byte, addr *net.UDPAddr, privKey *ecdsa.PrivateKey) error {

	datagram, err := DeserialisationUDP(ReqOrRep, nil)
	if err != nil {
		return fmt.Errorf("Erreur deserialisation(ProcessPacket): %v", err)
	}
	if datagram.Type < 128 {
		err := HandleRequest(client, conn, addr, *datagram, privKey)
		if err != nil {
			return fmt.Errorf("Erreur traitement de la requete(ProcessPacket): %v", err)
		}
		return nil
	} else {
		err := HandleResponse(*datagram)
		if err != nil {
			return fmt.Errorf("Erreur traitement reponse: %v", err)
		}
		return nil
	}
}

func DiscoveryRoutine(conn *net.UDPConn, serverAddr *net.UDPAddr, privKey *ecdsa.PrivateKey) error {
	packetHello, err := BuildHelloPacket(nickName, 0, privKey)
	if err != nil {
		return err
	}
	idHello := binary.BigEndian.Uint32(packetHello[0:4])
	chHello := make(chan ResponseMessage)
	resultReqOrRep.Lock()
	resultReqOrRep.responseChannels[idHello] = chHello
	resultReqOrRep.Unlock()

	SendRequestToThePeer(conn, serverAddr, packetHello)

	select {
	case resp := <-chHello:
		resultReqOrRep.Lock()
		delete(resultReqOrRep.responseChannels, idHello)
		resultReqOrRep.Unlock()
		if resp.Type == 129 {
			return fmt.Errorf("Le serveur a refusé le Hello: %s", string(resp.Body))
		}
		log.Printf("Association réussie (Type reçu: %d)", resp.Type)
	case <-time.After(3 * time.Second):
		return fmt.Errorf("Timeout sur le Hello")
	}

	packetRoot, err := BuildRootRequestPacket(privKey)
	if err != nil {
		return err
	}
	id := binary.BigEndian.Uint32(packetRoot[0:4])
	chRoot := make(chan ResponseMessage)
	resultReqOrRep.Lock()
	resultReqOrRep.responseChannels[id] = chRoot
	resultReqOrRep.Unlock()

	SendRequestToThePeer(conn, serverAddr, packetRoot)
	select {
	case resp := <-chRoot:
		resultReqOrRep.Lock()
		delete(resultReqOrRep.responseChannels, id)
		resultReqOrRep.Unlock()
		if resp.Type == 131 && len(resp.Body) == 32 {
			log.Printf("SUCCÈS ! Root Hash (32 octets) reçu: %x", resp.Body)
		} else {
			log.Printf("Réponse Type %d inattendue: %s", resp.Type, string(resp.Body))
		}
	case <-time.After(5 * time.Second):
		log.Printf("Timeout sur le RootRequest")
	}
	return nil
}

// -------------------------------------------------------------------------------------------------------------------------
//  										Fin: 4. Processe de traitement des packet et getter
// -------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
//  										5. La gestion de la base de données de contenu(Arbre de Merkle)
// -------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
//  										5. La gestion de la base de données de contenu(Arbre de Merkle)
// -------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------
//  									Fin: 4. Processe de traitement des packet
// -------------------------------------------------------------------------------------------------------------------------

//=========================================================================================================================
//											Fin: Code du protocole Pair-a-Pair: UDP
//=========================================================================================================================

// =========================================================================================================================
//												La fonction main
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
		log.Fatalf("Erreur clé: %v", err)
	}
	publicKey := privateKey.Public().(*ecdsa.PublicKey)
	client := createClient()

	resultReqOrRep.responseChannels = make(map[uint32]chan ResponseMessage)

	pubKeyBytes := formatPubKey(publicKey)
	if err := RegisterKey(client, nickName, pubKeyBytes); err != nil {
		log.Fatalf("Erreur Register: %v", err)
	}
	log.Println("Pair enregistré avec succès sur le serveur REST")

	addrLocal, _ := net.ResolveUDPAddr("udp", ":0")
	conn, err := StartUDPListener(addrLocal)
	if err != nil {
		log.Fatalf("Erreur UDP: %v", err)
	}
	defer conn.Close()
	log.Printf("Ecoute UDP sur le port : %v", conn.LocalAddr())

	go StartRead(client, conn, privateKey)

	serverAddr, _ := net.ResolveUDPAddr("udp", "jch.irif.fr:8443")

	log.Println("Envoi d'une RootRequest pour découvrir le réseau...")
	go DiscoveryRoutine(conn, serverAddr, privateKey)
	peerAddrs, err := GetPeerAddresses(client, "jch.irif.fr")
	log.Printf("Les adresses: %v", peerAddrs)
	select {}
}
