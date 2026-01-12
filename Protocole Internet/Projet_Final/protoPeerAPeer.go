package main

import (
	"crypto/ecdsa"
	"crypto/sha256"
	"encoding/binary"
	"fmt"
	"log"
	"net"
	"net/http"
	"time"
)

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

func HandleRequest(client *http.Client, conn *net.UDPConn, addr *net.UDPAddr, datagram Datagram, privKey *ecdsa.PrivateKey) error {
	var pubKey *ecdsa.PublicKey
	switch datagram.Type {
	case 1:
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
			pubKey = ParsePubKey(pubKeyWithoutParse)
		}
		dataToVerify := make([]byte, 7+len(datagram.Body))
		binary.BigEndian.PutUint32(dataToVerify[0:4], datagram.Id)
		dataToVerify[4] = datagram.Type
		binary.BigEndian.PutUint16(dataToVerify[5:7], datagram.Length)
		copy(dataToVerify[7:], datagram.Body)
		ok := VerifySignature(pubKey, datagram.Signature, dataToVerify)
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
	case 0:
		reply, err := BuildPongReplyPacket(datagram.Id)
		if err != nil {
			return fmt.Errorf("Erreur construction PongReply(HandleRequest): %v", err)
		}
		_, err = SendRequestToThePeer(conn, addr, reply)
		if err != nil {
			return fmt.Errorf("Erreur envoie PongReply(HandleRequest): %v", err)
		}
		return nil
	case 2:
		reply, err := BuildRootReplyPacket(datagram.Id, myRootHash, privKey)
		if err != nil {
			return fmt.Errorf("Erreur construction RootReply(HandleRequest): %v", err)
		}
		_, err = SendRequestToThePeer(conn, addr, reply)
		if err != nil {
			return fmt.Errorf("Erreur envoie RootReply(HandleRequest): %v", err)
		}
		return nil
	case 3:
		mapLock.RLock()
		var peerName string
		for name, assos := range peerMap {
			if assos.PublicKey != nil {
				peerName = name
				break
			}
		}
		mapLock.RUnlock()
		if peerName == "" {
			return fmt.Errorf("Erreur: le pair demandeur est inconnu(HandleRequest)")
		}
		contentStorage.RLock()
		hashKey := fmt.Sprintf("%x", datagram.Body)
		data, exist := contentStorage.storage[hashKey]
		contentStorage.RUnlock()
		if !exist {
			NoDatum, err := BuildNoDatumReplyPacket(datagram.Id, privKey)
			if err != nil {
				return fmt.Errorf("Erreur construction NoDatumReply(HandleRequest): %v", err)
			}
			_, err = SendRequestToThePeer(conn, addr, NoDatum)
			if err != nil {
				return fmt.Errorf("Erreur envoie NoDatumReply(HandleRequest): %v", err)
			}
			return nil
		}
		responseBody := make([]byte, 32+len(data))
		hashData := sha256.Sum256(data)
		copy(responseBody[0:32], hashData[:])
		nodeType := data[0]
		if nodeType != 0 && nodeType != 1 && nodeType != 2 && nodeType != 3 {
			return fmt.Errorf("Erreur: Type de noeud inconnu dans la donnée stockée")
		}
		copy(responseBody[32:], data)
		responsePacket, err := SerialisationDatagram(datagram.Id, 132, responseBody, nil)
		if err != nil {
			return fmt.Errorf("Erreur construction DatumReply(HandleRequest): %v", err)
		}
		_, err = SendRequestToThePeer(conn, addr, responsePacket)
		if err != nil {
			return fmt.Errorf("Erreur envoie DatumReply(HandleRequest): %v", err)
		}
		return nil
	default:
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
		mapLock.RLock()
		assos, exist := peerMap[string(addr.String())]
		mapLock.RUnlock()
		if !exist {
			return fmt.Errorf("Erreur: le pair demandeur est inconnu(HandleResponse)")
		}
		assos.LastSeen = time.Now()
		return nil
	}
}
