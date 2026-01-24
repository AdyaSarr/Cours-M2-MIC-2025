package main

import (
	"crypto/ecdsa"
	"encoding/binary"
	"fmt"
	"math/rand/v2"
	"net"
)

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
		signature, err := ComputeSignature(privKey, data)
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
				ok := VerifySignature(publicKey, signature, data)
				if !ok {
					return nil, ErrIgnorePack
				}
			}
		}
	}
	return &responseDatagram, nil
}

func generateRandomId() uint32 {
	return rand.Uint32()
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
	responseType := uint8(130)
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

func BuildRootReplyPacket(responseId uint32, rootHash []byte, privKey *ecdsa.PrivateKey) ([]byte, error) {
	responseType := uint8(131)
	responseBody := make([]byte, 32)
	copy(responseBody, rootHash)
	datagram, err := SerialisationDatagram(responseId, responseType, responseBody, privKey)
	if err != nil {
		return nil, fmt.Errorf("Erreur construction message RootReply(BuildRootReplyPacket): %v", err)
	}
	return datagram, nil
}

func BuildNoDatumReplyPacket(responseId uint32, privKey *ecdsa.PrivateKey) ([]byte, error) {
	responseType := uint8(133)
	responseBody := []byte{}
	datagram, err := SerialisationDatagram(responseId, responseType, responseBody, privKey)
	if err != nil {
		return nil, fmt.Errorf("Erreur construction message NoDatumReply(BuildNoDatumReplyPacket): %v", err)
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

func BuildForwardPacket(targetAddr *net.UDPAddr, originalDatagram []byte) ([]byte, error) {
	requestId := generateRandomId()
	requestType := uint8(4)
	ip := targetAddr.IP.To4()
	if ip == nil {
		ip = targetAddr.IP.To16()
	}
	body := make([]byte, len(ip)+2+len(originalDatagram))
	copy(body[0:len(ip)], ip)
	binary.BigEndian.PutUint16(body[len(ip):len(ip)+2], uint16(targetAddr.Port))
	copy(body[len(ip)+2:], originalDatagram)
	return SerialisationDatagram(requestId, requestType, body, nil)
}
