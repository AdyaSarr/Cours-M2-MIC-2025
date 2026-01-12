package main

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"fmt"
	"log"
	"net"
	"os"
	"path/filepath"
	"time"
)

func ParseNode(conn *net.UDPConn, serverAddr *net.UDPAddr, data []byte, currentPath string) error {
	if len(data) == 0 {
		return fmt.Errorf("Erreur taille(%d) de la donnée(ParseNode)", 0)
	}
	nodeType := data[0]
	payload := data[1:]
	switch nodeType {
	case 0:
		if len(payload) > 1024 {
			return fmt.Errorf("Chunk trop grand: %d octets", len(payload))
		}
		filePath := "downloads/" + currentPath
		dir := filepath.Dir(filePath)
		err := os.MkdirAll(dir, 0755)
		if err != nil {
			return fmt.Errorf("Erreur création dossier %s: %v", dir, err)
		}

		f, err := os.OpenFile(filePath, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err != nil {
			return fmt.Errorf("Erreur ouverture fichier %s: %v", filePath, err)
		}
		defer f.Close()

		_, err = f.Write(payload)
		if err != nil {
			return fmt.Errorf("Erreur écriture dans %s: %v", filePath, err)
		}

		log.Printf("Morceau ajouté au fichier : %s", filePath)
		return nil
	case 1:
		if len(payload)%64 != 0 {
			return fmt.Errorf("Erreur: la taille directory n'est pas multiple de 64: %d", len(payload))
		}
		numEntries := len(payload) / 64
		for i := 0; i < numEntries; i++ {
			entryHash := payload[i*64 : (i+1)*64]
			name := string(bytes.Trim(entryHash[0:32], "\x00"))
			hashValue := entryHash[32:64]
			go DownloadDatum(conn, serverAddr, hashValue, filepath.Join(currentPath, name))
		}
	case 2, 3:
		if len(payload)%32 != 0 {
			return fmt.Errorf("Taille BigNode ou BigDirectory invalide: %d", len(payload))
		}
		numChildren := len(payload) / 32
		for i := 0; i < numChildren; i++ {
			childHash := payload[i*32 : (i+1)*32]
			_, err := DownloadDatum(conn, serverAddr, childHash, currentPath)
			if err != nil {
				log.Printf("Erreur morceau %d du fichier %s : %v", i, currentPath, err)
			}
		}
		log.Printf("Nœud structurel (%d) reçu avec %d enfants", nodeType, len(payload)/32)
	default:
		return fmt.Errorf("Erreur: Type de noeud inconnu")
	}
	return nil
}

func DownloadDatum(conn *net.UDPConn, serverAddr *net.UDPAddr, hash []byte, fileName string) ([]byte, error) {
	const maxRetries = 3
	var lastErr error
	for i := 0; i < maxRetries; i++ {
		downloadLimit <- struct{}{}

		packetDatum, _ := BuildDatumRequestPacket(hash)
		idDatum := binary.BigEndian.Uint32(packetDatum[0:4])
		chDatum := make(chan ResponseMessage, 1)

		resultReqOrRep.Lock()
		resultReqOrRep.responseChannels[idDatum] = chDatum
		resultReqOrRep.Unlock()

		SendRequestToThePeer(conn, serverAddr, packetDatum)
		select {
		case datumResp := <-chDatum:
			<-downloadLimit // Libère la place
			resultReqOrRep.Lock()
			delete(resultReqOrRep.responseChannels, idDatum)
			resultReqOrRep.Unlock()

			if datumResp.Type == 132 {
				log.Printf("Donnée reçue (%d octets)", len(datumResp.Body))
				hashDataRcv := sha256.Sum256(datumResp.Body[32:])
				if !bytes.Equal(hashDataRcv[:], datumResp.Body[:32]) {
					return nil, fmt.Errorf("Erreur: le hash de la donnée reçue ne correspond pas au hash inclus dans la donnée")
				}
				if !bytes.Equal(hashDataRcv[:], hash) {
					return nil, fmt.Errorf("Erreur: le hash de la donnée reçue ne correspond pas au Root Hash")
				}
				err := ParseNode(conn, serverAddr, datumResp.Body[32:], fileName)
				if err != nil {
					log.Printf("Erreur ParseNode: %v", err)
				}
				return datumResp.Body, nil
			}
			lastErr = fmt.Errorf("Type inattendu %d", datumResp.Type)

		case <-time.After(3 * time.Second):
			<-downloadLimit
			resultReqOrRep.Lock()
			delete(resultReqOrRep.responseChannels, idDatum)
			resultReqOrRep.Unlock()
			lastErr = fmt.Errorf("Timeout essai %d", i+1)
		}
	}
	return nil, lastErr
}
func Store(data []byte) ([]byte, error) {
	if len(data) <= 0 {
		return nil, fmt.Errorf("Erreur taille(%d) de la donnée(Store)", 0)
	}
	if len(data) > 1024 {
		return nil, fmt.Errorf("Erreur taille(%d) de la donnée(Store) dépasse 1024", len(data))
	}
	contentStorage.Lock()
	hash := sha256.Sum256(data)
	hashKey := fmt.Sprintf("%x", hash[:])
	contentStorage.storage[hashKey] = data
	contentStorage.Unlock()
	return hash[:], nil
}
