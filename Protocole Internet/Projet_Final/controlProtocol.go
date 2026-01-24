package main

import (
	"bytes"
	"crypto/ecdsa"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"net"
	"net/http"
	"strings"
	"time"
)

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

func DiscoveryRoutine(conn *net.UDPConn, serverAddr *net.UDPAddr, privKey *ecdsa.PrivateKey) error {
	packetHello, err := BuildHelloPacket(nickName, 1, privKey)
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
			_, err := DownloadDatum(conn, serverAddr, resp.Body, "")
			if err != nil {
				log.Printf("Erreur DownloadDatum: %v", err)
			}
		} else {
			log.Printf("Réponse Type %d inattendue: %s", resp.Type, string(resp.Body))
		}
	case <-time.After(5 * time.Second):
		log.Printf("Timeout sur le RootRequest")
	}
	return nil
}

func maintainConnPairs(conn *net.UDPConn) {
	for {
		time.Sleep(1 * time.Minute)
		mapLock.Lock()
		for name, assos := range peerMap {
			if assos.Status != ASSOCIATED {
				continue
			}
			if time.Since(assos.LastSeen) > 5*time.Minute {
				log.Printf("Le pair %s est marqué comme OFFLINE", name)
				assos.Status = OFFLINE
			} else if time.Since(assos.LastSeen) > 3*time.Minute {
				ping, err := BuildPingPacket()
				if err != nil {
					log.Printf("Erreur construction Ping(maintainConnPairs): %v", err)
					continue
				}
				if len(assos.Addresses) > 0 {
					for _, addr := range assos.Addresses {
						_, err := SendRequestToThePeer(conn, &addr, ping)
						if err != nil {
							log.Printf("Erreur envoie Ping au pair %s(maintainConnPairs): %v", name, err)
						} else {
							log.Printf("Ping envoyé au pair %s pour maintenir la connexion", name)
						}
					}
					assos.LastSeen = time.Now()
				}
			}
		}
		mapLock.Unlock()
	}
}
