package main

import (
	"fmt"
	"log"
	"os"
)

func CutFileIntoChunks(filePath string) ([]byte, error) {
	fileData, err := os.ReadFile(filePath)
	if err != nil {
		return nil, err
	}

	var currentHashes [][]byte
	for i := 0; i < len(fileData); i += 1024 {
		end := i + 1024
		if end > len(fileData) {
			end = len(fileData)
		}
		h, _ := Store(append([]byte{0}, fileData[i:end]...))
		currentHashes = append(currentHashes, h)
	}

	for len(currentHashes) > 1 {
		var nextLevel [][]byte
		for i := 0; i < len(currentHashes); i += 31 {
			end := i + 31
			if end > len(currentHashes) {
				end = len(currentHashes)
			}

			node := []byte{2}
			for _, h := range currentHashes[i:end] {
				node = append(node, h...)
			}
			parentHash, _ := Store(node)
			nextLevel = append(nextLevel, parentHash)
		}
		currentHashes = nextLevel
	}
	return currentHashes[0], nil
}

func ExportCatsPhotos() ([]byte, error) {
	entries, err := os.ReadDir("Photos_Chats")
	if err != nil {
		return nil, fmt.Errorf("Erreur lecture dossier Photos_Chats: %v", err)
	}
	var dirBody []byte
	dirBody = append(dirBody, byte(1))
	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}
		filePath := "Photos_Chats/" + entry.Name()

		hash, err := CutFileIntoChunks(filePath)
		if err != nil {
			log.Printf("Erreur export %s: %v", entry.Name(), err)
			continue
		}
		entryBytes := make([]byte, 64)
		copy(entryBytes[0:32], []byte(entry.Name()))
		copy(entryBytes[32:64], hash)
		dirBody = append(dirBody, entryBytes...)
	}
	bigDir := append([]byte{3}, dirBody...)
	return Store(bigDir)
}
